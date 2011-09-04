/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/vf/driver.c
 * PURPOSE:         Driver Verifier Device Driver Interface
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

/*
 * @unimplemented
 */
BOOLEAN
NTAPI
VfIsVerificationEnabled(IN VF_OBJECT_TYPE VfObjectType,
                        IN PVOID Object OPTIONAL)
{
    UNIMPLEMENTED;
    return FALSE;
}

/*
 * @unimplemented
 */
VOID
NTAPI
VfFailDeviceNode(IN PDEVICE_OBJECT PhysicalDeviceObject,
                 IN ULONG BugCheckMajorCode,
                 IN ULONG BugCheckMinorCode,
                 IN VF_FAILURE_CLASS FailureClass,
                 IN OUT PULONG AssertionControl,
                 IN PSTR DebuggerMessageText,
                 IN PSTR ParameterFormatString,
                 ...)
{
    UNIMPLEMENTED;
}

/*
 * @unimplemented
 */
VOID
NTAPI
VfFailSystemBIOS(IN ULONG BugCheckMajorCode,
                 IN ULONG BugCheckMinorCode,
                 IN VF_FAILURE_CLASS FailureClass,
                 IN OUT PULONG AssertionControl,
                 IN PSTR DebuggerMessageText,
                 IN PSTR ParameterFormatString,
                 ...)
{
    UNIMPLEMENTED;
}

/* EOF */
