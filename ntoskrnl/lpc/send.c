/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/lpc/send.c
 * PURPOSE:         Local Procedure Call: Sending (Requests)
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* PUBLIC FUNCTIONS **********************************************************/

/*
 * @implemented
 */
NTSTATUS
NTAPI
LpcRequestPort(IN PVOID PortObject,
               IN PPORT_MESSAGE LpcMessage)
{
    PLPCP_PORT_OBJECT Port = PortObject, QueuePort, ConnectionPort = NULL;
    ULONG MessageType;
    PLPCP_MESSAGE Message;
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    PAGED_CODE();
    LPCTRACE(LPC_SEND_DEBUG, "Port: %p. Message: %p\n", Port, LpcMessage);

    /* Check if this is a non-datagram message */
    if (LpcMessage->u2.s2.Type)
    {
        /* Get the message type */
        MessageType = LpcpGetMessageType(LpcMessage);

        /* Validate it */
        if ((MessageType < LPC_DATAGRAM) || (MessageType > LPC_CLIENT_DIED))
        {
            /* Fail */
            return STATUS_INVALID_PARAMETER;
        }

        /* Mark this as a kernel-mode message only if we really came from it */
        if ((PreviousMode == KernelMode) &&
            (LpcMessage->u2.s2.Type & LPC_KERNELMODE_MESSAGE))
        {
            /* We did, this is a kernel mode message */
            MessageType |= LPC_KERNELMODE_MESSAGE;
        }
    }
    else
    {
        /* This is a datagram */
        MessageType = LPC_DATAGRAM;
    }

    /* Can't have data information on this type of call */
    if (LpcMessage->u2.s2.DataInfoOffset) return STATUS_INVALID_PARAMETER;

    /* Validate message sizes */
    if (((ULONG)LpcMessage->u1.s1.TotalLength > Port->MaxMessageLength) ||
        ((ULONG)LpcMessage->u1.s1.TotalLength <= (ULONG)LpcMessage->u1.s1.DataLength))
    {
        /* Fail */
        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

    /* Allocate a new message */
    Message = LpcpAllocateFromPortZone();
    if (!Message) return STATUS_NO_MEMORY;

    /* Clear the context */
    Message->RepliedToThread = NULL;
    Message->PortContext = NULL;

    /* Copy the message */
    LpcpMoveMessage(&Message->Request,
                    LpcMessage,
                    LpcMessage + 1,
                    MessageType,
                    &PsGetCurrentThread()->Cid);

    /* Acquire the LPC lock */
    KeAcquireGuardedMutex(&LpcpLock);

    /* Check if this is anything but a connection port */
    if ((Port->Flags & LPCP_PORT_TYPE_MASK) != LPCP_CONNECTION_PORT)
    {
        /* The queue port is the connected port */
        QueuePort = Port->ConnectedPort;
        if (QueuePort)
        {
            /* Check if this is a client port */
            if ((Port->Flags & LPCP_PORT_TYPE_MASK) == LPCP_CLIENT_PORT)
            {
                /* Then copy the context */
                Message->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = Port->ConnectionPort;
                if (!ConnectionPort)
                {
                    /* Fail */
                    LpcpFreeToPortZone(Message, 3);
                    return STATUS_PORT_DISCONNECTED;
                }
            }
            else if ((Port->Flags & LPCP_PORT_TYPE_MASK) != LPCP_COMMUNICATION_PORT)
            {
                /* Any other kind of port, use the connection port */
                ConnectionPort = QueuePort = Port->ConnectionPort;
                if (!ConnectionPort)
                {
                    /* Fail */
                    LpcpFreeToPortZone(Message, 3);
                    return STATUS_PORT_DISCONNECTED;
                }
            }

            /* If we have a connection port, reference it */
            if (ConnectionPort) ObReferenceObject(ConnectionPort);
        }
    }
    else
    {
        /* For connection ports, use the port itself */
        QueuePort = PortObject;
    }

    /* Make sure we have a port */
    if (QueuePort)
    {
        /* Generate the Message ID and set it */
        Message->Request.MessageId =  LpcpNextMessageId++;
        if (!LpcpNextMessageId) LpcpNextMessageId = 1;
        Message->Request.CallbackId = 0;

        /* No Message ID for the thread */
        PsGetCurrentThread()->LpcReplyMessageId = 0;

        /* Insert the message in our chain */
        InsertTailList(&QueuePort->MsgQueue.ReceiveHead, &Message->Entry);

        /* Release the lock and release the semaphore */
        KeEnterCriticalRegion();
        KeReleaseGuardedMutex(&LpcpLock);
        LpcpCompleteWait(QueuePort->MsgQueue.Semaphore);

        /* If this is a waitable port, wake it up */
        if (QueuePort->Flags & LPCP_WAITABLE_PORT)
        {
            /* Wake it */
            KeSetEvent(&QueuePort->WaitEvent, IO_NO_INCREMENT, FALSE);
        }

        /* We're done */
        KeLeaveCriticalRegion();
        if (ConnectionPort) ObDereferenceObject(ConnectionPort);
        LPCTRACE(LPC_SEND_DEBUG, "Port: %p. Message: %p\n", QueuePort, Message);
        return STATUS_SUCCESS;
    }

    /* If we got here, then free the message and fail */
    LpcpFreeToPortZone(Message, 3);
    if (ConnectionPort) ObDereferenceObject(ConnectionPort);
    return STATUS_PORT_DISCONNECTED;
}

/*
* @implemented
*/
NTSTATUS
NTAPI
LpcRequestWaitReplyPort(IN PVOID PortObject,
                        IN PPORT_MESSAGE LpcRequest,
                        OUT PPORT_MESSAGE LpcReply)
{
    PLPCP_PORT_OBJECT Port, QueuePort, ReplyPort, ConnectionPort = NULL;
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    NTSTATUS Status = STATUS_SUCCESS;
    PLPCP_MESSAGE Message;
    PETHREAD Thread = PsGetCurrentThread();
    BOOLEAN Callback = FALSE;
    PKSEMAPHORE Semaphore;
    ULONG MessageType;
    PAGED_CODE();

    Port = (PLPCP_PORT_OBJECT)PortObject;

    LPCTRACE(LPC_SEND_DEBUG,
             "Port: %p. Messages: %p/%p. Type: %lx\n",
             Port,
             LpcRequest,
             LpcReply,
             LpcpGetMessageType(LpcRequest));

    /* Check if the thread is dying */
    if (Thread->LpcExitThreadCalled) return STATUS_THREAD_IS_TERMINATING;

    /* Check if this is an LPC Request */
    MessageType = LpcpGetMessageType(LpcRequest);
    switch (MessageType)
    {
        /* No type */
        case 0:
            
            /* Assume LPC request */
            MessageType = LPC_REQUEST;
            break;
        
        /* LPC request callback */
        case LPC_REQUEST:
            
            /* This is a callback */
            Callback = TRUE;
            break;
        
        /* Anything else */
        case LPC_CLIENT_DIED:
        case LPC_PORT_CLOSED:
        case LPC_EXCEPTION:
        case LPC_DEBUG_EVENT:
        case LPC_ERROR_EVENT:
            
            /* Nothing to do */
            break;
            
        default:
            
            /* Invalid message type */
            return STATUS_INVALID_PARAMETER;
    }
    
    /* Set the request type */
    LpcRequest->u2.s2.Type = MessageType;

    /* Validate the message length */
    if (((ULONG)LpcRequest->u1.s1.TotalLength > Port->MaxMessageLength) ||
        ((ULONG)LpcRequest->u1.s1.TotalLength <= (ULONG)LpcRequest->u1.s1.DataLength))
    {
        /* Fail */
        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

    /* Allocate a message from the port zone */
    Message = LpcpAllocateFromPortZone();
    if (!Message)
    {
        /* Fail if we couldn't allocate a message */
        return STATUS_NO_MEMORY;
    }

    /* Check if this is a callback */
    if (Callback)
    {
        /* FIXME: TODO */
        Semaphore = NULL; // we'd use the Thread Semaphore here
        ASSERT(FALSE);
        return STATUS_NOT_IMPLEMENTED;
    }
    else
    {
        /* No callback, just copy the message */
        LpcpMoveMessage(&Message->Request,
                        LpcRequest,
                        LpcRequest + 1,
                        0,
                        &Thread->Cid);

        /* Acquire the LPC lock */
        KeAcquireGuardedMutex(&LpcpLock);

        /* Right now clear the port context */
        Message->PortContext = NULL;

        /* Check if this is a not connection port */
        if ((Port->Flags & LPCP_PORT_TYPE_MASK) != LPCP_CONNECTION_PORT)
        {
            /* We want the connected port */
            QueuePort = Port->ConnectedPort;
            if (!QueuePort)
            {
                /* We have no connected port, fail */
                LpcpFreeToPortZone(Message, 3);
                return STATUS_PORT_DISCONNECTED;
            }

            /* This will be the rundown port */
            ReplyPort = QueuePort;

            /* Check if this is a communication port */
            if ((Port->Flags & LPCP_PORT_TYPE_MASK) == LPCP_CLIENT_PORT)
            {
                /* Copy the port context and use the connection port */
                Message->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = Port->ConnectionPort;
                if (!ConnectionPort)
                {
                    /* Fail */
                    LpcpFreeToPortZone(Message, 3);
                    return STATUS_PORT_DISCONNECTED;
                }
            }
            else if ((Port->Flags & LPCP_PORT_TYPE_MASK) !=
                      LPCP_COMMUNICATION_PORT)
            {
                /* Use the connection port for anything but communication ports */
                ConnectionPort = QueuePort = Port->ConnectionPort;
                if (!ConnectionPort)
                {
                    /* Fail */
                    LpcpFreeToPortZone(Message, 3);
                    return STATUS_PORT_DISCONNECTED;
                }
            }

            /* Reference the connection port if it exists */
            if (ConnectionPort) ObReferenceObject(ConnectionPort);
        }
        else
        {
            /* Otherwise, for a connection port, use the same port object */
            QueuePort = ReplyPort = Port;
        }

        /* No reply thread */
        Message->RepliedToThread = NULL;
        Message->SenderPort = Port;

        /* Generate the Message ID and set it */
        Message->Request.MessageId =  LpcpNextMessageId++;
        if (!LpcpNextMessageId) LpcpNextMessageId = 1;
        Message->Request.CallbackId = 0;

        /* Set the message ID for our thread now */
        Thread->LpcReplyMessageId = Message->Request.MessageId;
        Thread->LpcReplyMessage = NULL;

        /* Insert the message in our chain */
        InsertTailList(&QueuePort->MsgQueue.ReceiveHead, &Message->Entry);
        InsertTailList(&ReplyPort->LpcReplyChainHead, &Thread->LpcReplyChain);
        LpcpSetPortToThread(Thread, Port);

        /* Release the lock and get the semaphore we'll use later */
        KeEnterCriticalRegion();
        KeReleaseGuardedMutex(&LpcpLock);
        Semaphore = QueuePort->MsgQueue.Semaphore;

        /* If this is a waitable port, wake it up */
        if (QueuePort->Flags & LPCP_WAITABLE_PORT)
        {
            /* Wake it */
            KeSetEvent(&QueuePort->WaitEvent, IO_NO_INCREMENT, FALSE);
        }
    }

    /* Now release the semaphore */
    LpcpCompleteWait(Semaphore);
    KeLeaveCriticalRegion();

    /* And let's wait for the reply */
    LpcpReplyWait(&Thread->LpcReplySemaphore, PreviousMode);

    /* Acquire the LPC lock */
    KeAcquireGuardedMutex(&LpcpLock);

    /* Get the LPC Message and clear our thread's reply data */
    Message = LpcpGetMessageFromThread(Thread);
    Thread->LpcReplyMessage = NULL;
    Thread->LpcReplyMessageId = 0;

    /* Check if we have anything on the reply chain*/
    if (!IsListEmpty(&Thread->LpcReplyChain))
    {
        /* Remove this thread and reinitialize the list */
        RemoveEntryList(&Thread->LpcReplyChain);
        InitializeListHead(&Thread->LpcReplyChain);
    }

    /* Release the lock */
    KeReleaseGuardedMutex(&LpcpLock);

    /* Check if we got a reply */
    if (Status == STATUS_SUCCESS)
    {
        /* Check if we have a valid message */
        if (Message)
        {
            LPCTRACE(LPC_SEND_DEBUG,
                     "Reply Messages: %p/%p\n",
                     &Message->Request,
                     (&Message->Request) + 1);

            /* Move the message */
            LpcpMoveMessage(LpcReply,
                            &Message->Request,
                            (&Message->Request) + 1,
                            0,
                            NULL);
            
            /* Acquire the lock */
            KeAcquireGuardedMutex(&LpcpLock);
            
            /* Check if we replied to a thread */
            if (Message->RepliedToThread)
            {
                /* Dereference */
                ObDereferenceObject(Message->RepliedToThread);
                Message->RepliedToThread = NULL;
            }


            /* Free the message */
            LpcpFreeToPortZone(Message, 3);
        }
        else
        {
            /* We don't have a reply */
            Status = STATUS_LPC_REPLY_LOST;
        }
    }
    else
    {
        /* The wait failed, free the message */
        if (Message) LpcpFreeToPortZone(Message, 0);
    }

    /* All done */
    LPCTRACE(LPC_SEND_DEBUG,
             "Port: %p. Status: %p\n",
             Port,
             Status);

    /* Dereference the connection port */
    if (ConnectionPort) ObDereferenceObject(ConnectionPort);
    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
NtRequestPort(IN HANDLE PortHandle,
              IN PPORT_MESSAGE LpcRequest)
{
    PLPCP_PORT_OBJECT Port, QueuePort, ConnectionPort = NULL;
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    NTSTATUS Status;
    PLPCP_MESSAGE Message;
    PETHREAD Thread = PsGetCurrentThread();

    PKSEMAPHORE Semaphore;
    ULONG MessageType;
    PAGED_CODE();
    LPCTRACE(LPC_SEND_DEBUG,
             "Handle: %lx. Message: %p. Type: %lx\n",
             PortHandle,
             LpcRequest,
             LpcpGetMessageType(LpcRequest));

    /* Get the message type */
    MessageType = LpcRequest->u2.s2.Type | LPC_DATAGRAM;

    /* Can't have data information on this type of call */
    if (LpcRequest->u2.s2.DataInfoOffset) return STATUS_INVALID_PARAMETER;

    /* Validate the length */
    if (((ULONG)LpcRequest->u1.s1.DataLength + sizeof(PORT_MESSAGE)) >
         (ULONG)LpcRequest->u1.s1.TotalLength)
    {
        /* Fail */
        return STATUS_INVALID_PARAMETER;
    }

    /* Reference the object */
    Status = ObReferenceObjectByHandle(PortHandle,
                                       0,
                                       LpcPortObjectType,
                                       PreviousMode,
                                       (PVOID*)&Port,
                                       NULL);
    if (!NT_SUCCESS(Status)) return Status;

    /* Validate the message length */
    if (((ULONG)LpcRequest->u1.s1.TotalLength > Port->MaxMessageLength) ||
        ((ULONG)LpcRequest->u1.s1.TotalLength <= (ULONG)LpcRequest->u1.s1.DataLength))
    {
        /* Fail */
        ObDereferenceObject(Port);
        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

    /* Allocate a message from the port zone */
    Message = LpcpAllocateFromPortZone();
    if (!Message)
    {
        /* Fail if we couldn't allocate a message */
        ObDereferenceObject(Port);
        return STATUS_NO_MEMORY;
    }

    /* No callback, just copy the message */
    _SEH2_TRY
    {
        /* Copy it */
        LpcpMoveMessage(&Message->Request,
            LpcRequest,
            LpcRequest + 1,
            MessageType,
            &Thread->Cid);
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Fail */
        LpcpFreeToPortZone(Message, 0);
        ObDereferenceObject(Port);
        _SEH2_YIELD(return _SEH2_GetExceptionCode());
    }
    _SEH2_END;

    /* Acquire the LPC lock */
    KeAcquireGuardedMutex(&LpcpLock);

    /* Right now clear the port context */
    Message->PortContext = NULL;

    /* Check if this is a not connection port */
    if ((Port->Flags & LPCP_PORT_TYPE_MASK) != LPCP_CONNECTION_PORT)
    {
        /* We want the connected port */
        QueuePort = Port->ConnectedPort;
        if (!QueuePort)
        {
            /* We have no connected port, fail */
            LpcpFreeToPortZone(Message, 3);
            ObDereferenceObject(Port);
            return STATUS_PORT_DISCONNECTED;
        }

        /* Check if this is a communication port */
        if ((Port->Flags & LPCP_PORT_TYPE_MASK) == LPCP_CLIENT_PORT)
        {
            /* Copy the port context and use the connection port */
            Message->PortContext = QueuePort->PortContext;
            ConnectionPort = QueuePort = Port->ConnectionPort;
            if (!ConnectionPort)
            {
                /* Fail */
                LpcpFreeToPortZone(Message, 3);
                ObDereferenceObject(Port);
                return STATUS_PORT_DISCONNECTED;
            }
        }
        else if ((Port->Flags & LPCP_PORT_TYPE_MASK) !=
            LPCP_COMMUNICATION_PORT)
        {
            /* Use the connection port for anything but communication ports */
            ConnectionPort = QueuePort = Port->ConnectionPort;
            if (!ConnectionPort)
            {
                /* Fail */
                LpcpFreeToPortZone(Message, 3);
                ObDereferenceObject(Port);
                return STATUS_PORT_DISCONNECTED;
            }
        }

        /* Reference the connection port if it exists */
        if (ConnectionPort) ObReferenceObject(ConnectionPort);
    }
    else
    {
        /* Otherwise, for a connection port, use the same port object */
        QueuePort = Port;
    }

    /* Reference QueuePort if we have it */
    if (QueuePort && ObReferenceObjectSafe(QueuePort))
    {
        /* Set sender's port */
        Message->SenderPort = Port;

        /* Generate the Message ID and set it */
        Message->Request.MessageId =  LpcpNextMessageId++;
        if (!LpcpNextMessageId) LpcpNextMessageId = 1;
        Message->Request.CallbackId = 0;

        /* No Message ID for the thread */
        PsGetCurrentThread()->LpcReplyMessageId = 0;

        /* Insert the message in our chain */
        InsertTailList(&QueuePort->MsgQueue.ReceiveHead, &Message->Entry);

        /* Release the lock and get the semaphore we'll use later */
        KeEnterCriticalRegion();
        KeReleaseGuardedMutex(&LpcpLock);

        /* Now release the semaphore */
        Semaphore = QueuePort->MsgQueue.Semaphore;
        LpcpCompleteWait(Semaphore);

        /* If this is a waitable port, wake it up */
        if (QueuePort->Flags & LPCP_WAITABLE_PORT)
        {
            /* Wake it */
            KeSetEvent(&QueuePort->WaitEvent, IO_NO_INCREMENT, FALSE);
        }

        KeLeaveCriticalRegion();

        /* Dereference objects */
        if (ConnectionPort) ObDereferenceObject(ConnectionPort);
        ObDereferenceObject(QueuePort);
        ObDereferenceObject(Port);
        LPCTRACE(LPC_SEND_DEBUG, "Port: %p. Message: %p\n", QueuePort, Message);
        return STATUS_SUCCESS;
    }

    Status = STATUS_PORT_DISCONNECTED;

    /* All done with a failure*/
    LPCTRACE(LPC_SEND_DEBUG,
             "Port: %p. Status: %p\n",
             Port,
             Status);

    /* The wait failed, free the message */
    if (Message) LpcpFreeToPortZone(Message, 3);

    ObDereferenceObject(Port);
    if (ConnectionPort) ObDereferenceObject(ConnectionPort);
    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
NtRequestWaitReplyPort(IN HANDLE PortHandle,
                       IN PPORT_MESSAGE LpcRequest,
                       IN OUT PPORT_MESSAGE LpcReply)
{
    PLPCP_PORT_OBJECT Port, QueuePort, ReplyPort, ConnectionPort = NULL;
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    NTSTATUS Status;
    PLPCP_MESSAGE Message;
    PETHREAD Thread = PsGetCurrentThread();
    BOOLEAN Callback;
    PKSEMAPHORE Semaphore;
    ULONG MessageType;
    PAGED_CODE();
    LPCTRACE(LPC_SEND_DEBUG,
             "Handle: %lx. Messages: %p/%p. Type: %lx\n",
             PortHandle,
             LpcRequest,
             LpcReply,
             LpcpGetMessageType(LpcRequest));

    /* Check if the thread is dying */
    if (Thread->LpcExitThreadCalled) return STATUS_THREAD_IS_TERMINATING;

    /* Check if this is an LPC Request */
    if (LpcpGetMessageType(LpcRequest) == LPC_REQUEST)
    {
        /* Then it's a callback */
        Callback = TRUE;
    }
    else if (LpcpGetMessageType(LpcRequest))
    {
        /* This is a not kernel-mode message */
        return STATUS_INVALID_PARAMETER;
    }
    else
    {
        /* This is a kernel-mode message without a callback */
        LpcRequest->u2.s2.Type |= LPC_REQUEST;
        Callback = FALSE;
    }

    /* Get the message type */
    MessageType = LpcRequest->u2.s2.Type;

    /* Validate the length */
    if (((ULONG)LpcRequest->u1.s1.DataLength + sizeof(PORT_MESSAGE)) >
         (ULONG)LpcRequest->u1.s1.TotalLength)
    {
        /* Fail */
        return STATUS_INVALID_PARAMETER;
    }

    /* Reference the object */
    Status = ObReferenceObjectByHandle(PortHandle,
                                       0,
                                       LpcPortObjectType,
                                       PreviousMode,
                                       (PVOID*)&Port,
                                       NULL);
    if (!NT_SUCCESS(Status)) return Status;

    /* Validate the message length */
    if (((ULONG)LpcRequest->u1.s1.TotalLength > Port->MaxMessageLength) ||
        ((ULONG)LpcRequest->u1.s1.TotalLength <= (ULONG)LpcRequest->u1.s1.DataLength))
    {
        /* Fail */
        ObDereferenceObject(Port);
        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

    /* Allocate a message from the port zone */
    Message = LpcpAllocateFromPortZone();
    if (!Message)
    {
        /* Fail if we couldn't allocate a message */
        ObDereferenceObject(Port);
        return STATUS_NO_MEMORY;
    }

    /* Check if this is a callback */
    if (Callback)
    {
        /* FIXME: TODO */
        Semaphore = NULL; // we'd use the Thread Semaphore here
        ASSERT(FALSE);
    }
    else
    {
        /* No callback, just copy the message */
        _SEH2_TRY
        {
            /* Copy it */
            LpcpMoveMessage(&Message->Request,
                            LpcRequest,
                            LpcRequest + 1,
                            MessageType,
                            &Thread->Cid);
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            /* Fail */
            LpcpFreeToPortZone(Message, 0);
            ObDereferenceObject(Port);
            _SEH2_YIELD(return _SEH2_GetExceptionCode());
        }
        _SEH2_END;

        /* Acquire the LPC lock */
        KeAcquireGuardedMutex(&LpcpLock);

        /* Right now clear the port context */
        Message->PortContext = NULL;

        /* Check if this is a not connection port */
        if ((Port->Flags & LPCP_PORT_TYPE_MASK) != LPCP_CONNECTION_PORT)
        {
            /* We want the connected port */
            QueuePort = Port->ConnectedPort;
            if (!QueuePort)
            {
                /* We have no connected port, fail */
                LpcpFreeToPortZone(Message, 3);
                ObDereferenceObject(Port);
                return STATUS_PORT_DISCONNECTED;
            }

            /* This will be the rundown port */
            ReplyPort = QueuePort;

            /* Check if this is a communication port */
            if ((Port->Flags & LPCP_PORT_TYPE_MASK) == LPCP_CLIENT_PORT)
            {
                /* Copy the port context and use the connection port */
                Message->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = Port->ConnectionPort;
                if (!ConnectionPort)
                {
                    /* Fail */
                    LpcpFreeToPortZone(Message, 3);
                    ObDereferenceObject(Port);
                    return STATUS_PORT_DISCONNECTED;
                }
            }
            else if ((Port->Flags & LPCP_PORT_TYPE_MASK) !=
                      LPCP_COMMUNICATION_PORT)
            {
                /* Use the connection port for anything but communication ports */
                ConnectionPort = QueuePort = Port->ConnectionPort;
                if (!ConnectionPort)
                {
                    /* Fail */
                    LpcpFreeToPortZone(Message, 3);
                    ObDereferenceObject(Port);
                    return STATUS_PORT_DISCONNECTED;
                }
            }

            /* Reference the connection port if it exists */
            if (ConnectionPort) ObReferenceObject(ConnectionPort);
        }
        else
        {
            /* Otherwise, for a connection port, use the same port object */
            QueuePort = ReplyPort = Port;
        }

        /* No reply thread */
        Message->RepliedToThread = NULL;
        Message->SenderPort = Port;

        /* Generate the Message ID and set it */
        Message->Request.MessageId =  LpcpNextMessageId++;
        if (!LpcpNextMessageId) LpcpNextMessageId = 1;
        Message->Request.CallbackId = 0;

        /* Set the message ID for our thread now */
        Thread->LpcReplyMessageId = Message->Request.MessageId;
        Thread->LpcReplyMessage = NULL;

        /* Insert the message in our chain */
        InsertTailList(&QueuePort->MsgQueue.ReceiveHead, &Message->Entry);
        InsertTailList(&ReplyPort->LpcReplyChainHead, &Thread->LpcReplyChain);
        LpcpSetPortToThread(Thread, Port);

        /* Release the lock and get the semaphore we'll use later */
        KeEnterCriticalRegion();
        KeReleaseGuardedMutex(&LpcpLock);
        Semaphore = QueuePort->MsgQueue.Semaphore;

        /* If this is a waitable port, wake it up */
        if (QueuePort->Flags & LPCP_WAITABLE_PORT)
        {
            /* Wake it */
            KeSetEvent(&QueuePort->WaitEvent, IO_NO_INCREMENT, FALSE);
        }
    }

    /* Now release the semaphore */
    LpcpCompleteWait(Semaphore);
    KeLeaveCriticalRegion();

    /* And let's wait for the reply */
    LpcpReplyWait(&Thread->LpcReplySemaphore, PreviousMode);

    /* Acquire the LPC lock */
    KeAcquireGuardedMutex(&LpcpLock);

    /* Get the LPC Message and clear our thread's reply data */
    Message = LpcpGetMessageFromThread(Thread);
    Thread->LpcReplyMessage = NULL;
    Thread->LpcReplyMessageId = 0;

    /* Check if we have anything on the reply chain*/
    if (!IsListEmpty(&Thread->LpcReplyChain))
    {
        /* Remove this thread and reinitialize the list */
        RemoveEntryList(&Thread->LpcReplyChain);
        InitializeListHead(&Thread->LpcReplyChain);
    }

    /* Release the lock */
    KeReleaseGuardedMutex(&LpcpLock);

    /* Check if we got a reply */
    if (Status == STATUS_SUCCESS)
    {
        /* Check if we have a valid message */
        if (Message)
        {
            LPCTRACE(LPC_SEND_DEBUG,
                     "Reply Messages: %p/%p\n",
                     &Message->Request,
                     (&Message->Request) + 1);

            /* Move the message */
            _SEH2_TRY
            {
                LpcpMoveMessage(LpcReply,
                                &Message->Request,
                                (&Message->Request) + 1,
                                0,
                                NULL);
            }
            _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
            {
                Status = _SEH2_GetExceptionCode();
            }
            _SEH2_END;

            /* Check if this is an LPC request with data information */
            if ((LpcpGetMessageType(&Message->Request) == LPC_REQUEST) &&
                (Message->Request.u2.s2.DataInfoOffset))
            {
                /* Save the data information */
                LpcpSaveDataInfoMessage(Port, Message, 0);
            }
            else
            {
                /* Otherwise, just free it */
                LpcpFreeToPortZone(Message, 0);
            }
        }
        else
        {
            /* We don't have a reply */
            Status = STATUS_LPC_REPLY_LOST;
        }
    }
    else
    {
        /* The wait failed, free the message */
        if (Message) LpcpFreeToPortZone(Message, 0);
    }

    /* All done */
    LPCTRACE(LPC_SEND_DEBUG,
             "Port: %p. Status: %p\n",
             Port,
             Status);
    ObDereferenceObject(Port);
    if (ConnectionPort) ObDereferenceObject(ConnectionPort);
    return Status;
}

/* EOF */
