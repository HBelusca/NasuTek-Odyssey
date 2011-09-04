/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/mm/ARM3/miavl.h
 * PURPOSE:         ARM Memory Manager VAD Node Algorithms
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES ******************************************************************/

/*
 * This is the glue code for the Memory Manager version of AVL Trees that is used
 * to store the MM_AVL_TABLE for Virtual Address Descriptors (VAD) in the VadRoot
 * field in EPROCESS.
 *
 * In this version of the package, the balance and parent pointers are stored in
 * the same field as a union (since we know the parent will be at least 8-byte
 * aligned), saving some space, but requiring special logic to handle setting and
 * querying the parent and balance.
 *
 * The other difference is that the AVL package for Rtl has custom callbacks for
 * comparison purposes (which would access some internal, opaque, user data) while
 * the Mm package stores the user-data inline as StartingVpn and EndingVpn. So
 * when a compare is being made, RtlpAvlCompareRoutine is called, which will either
 * perform the Mm work, or call the user-specified callback in the Rtl case.
 */
#define PRTL_AVL_TABLE              PMM_AVL_TABLE
#define PRTL_BALANCED_LINKS         PMMADDRESS_NODE
#define MI_ASSERT(x)                ASSERT(x)

VOID
FORCEINLINE
RtlpCopyAvlNodeData(IN PRTL_BALANCED_LINKS Node1,
                    IN PRTL_BALANCED_LINKS Node2)
{
    Node1->u1.Parent = Node2->u1.Parent;
    Node1->LeftChild = Node2->LeftChild;
    Node1->RightChild = Node2->RightChild;
}

RTL_GENERIC_COMPARE_RESULTS
FORCEINLINE
RtlpAvlCompareRoutine(IN PRTL_AVL_TABLE Table,
                      IN PVOID Buffer,
                      IN PVOID UserData)
{
    PRTL_BALANCED_LINKS CurrentNode = (PVOID)((ULONG_PTR)UserData - sizeof(RTL_BALANCED_LINKS));
    ULONG_PTR StartingVpn = (ULONG_PTR)Buffer;
    if (StartingVpn < CurrentNode->StartingVpn)
    {
        return GenericLessThan;
    }
    else if (StartingVpn <= CurrentNode->EndingVpn)
    {
        return GenericEqual;
    }
    else
    {
        return GenericGreaterThan;
    }
}

VOID
FORCEINLINE
RtlSetParent(IN PRTL_BALANCED_LINKS Node,
             IN PRTL_BALANCED_LINKS Parent)
{
    Node->u1.Parent = (PRTL_BALANCED_LINKS)((ULONG_PTR)Parent | (Node->u1.Balance & 0x3));
}

VOID
FORCEINLINE
RtlSetBalance(IN PRTL_BALANCED_LINKS Node,
              IN SCHAR Balance)
{
    Node->u1.Balance = Balance;
}

SCHAR
FORCEINLINE
RtlBalance(IN PRTL_BALANCED_LINKS Node)
{
    return (SCHAR)Node->u1.Balance;
}

PRTL_BALANCED_LINKS
FORCEINLINE
RtlParentAvl(IN PRTL_BALANCED_LINKS Node)
{
    return (PRTL_BALANCED_LINKS)((ULONG_PTR)Node->u1.Parent & ~3);
}

BOOLEAN
FORCEINLINE
RtlIsRootAvl(IN PRTL_BALANCED_LINKS Node)
{
   return (RtlParentAvl(Node) == Node);
}

PRTL_BALANCED_LINKS
FORCEINLINE
RtlRightChildAvl(IN PRTL_BALANCED_LINKS Node)
{
    return Node->RightChild;
}

PRTL_BALANCED_LINKS
FORCEINLINE
RtlLeftChildAvl(IN PRTL_BALANCED_LINKS Node)
{
    return Node->LeftChild;
}

BOOLEAN
FORCEINLINE
RtlIsLeftChildAvl(IN PRTL_BALANCED_LINKS Node)
{
   return (RtlLeftChildAvl(RtlParentAvl(Node)) == Node);
}

BOOLEAN
FORCEINLINE
RtlIsRightChildAvl(IN PRTL_BALANCED_LINKS Node)
{
   return (RtlRightChildAvl(RtlParentAvl(Node)) == Node);
}

VOID
FORCEINLINE
RtlInsertAsLeftChildAvl(IN PRTL_BALANCED_LINKS Parent,
                        IN PRTL_BALANCED_LINKS Node)
{
    Parent->LeftChild = Node;
    RtlSetParent(Node, Parent);
}

VOID
FORCEINLINE
RtlInsertAsRightChildAvl(IN PRTL_BALANCED_LINKS Parent,
                         IN PRTL_BALANCED_LINKS Node)
{
    Parent->RightChild = Node;
    RtlSetParent(Node, Parent);
}

/* EOF */
