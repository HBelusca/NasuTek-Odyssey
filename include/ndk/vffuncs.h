/*++ NDK Version: 0098

Copyright (c) Odyssey Portable Systems Group.  All rights reserved.

Header Name:

    vffuncs.h

Abstract:

    Function definitions for the Driver Verifier.

Author:

    Odyssey Portable Systems Group (ros.arm@odyssey.org) - Created - 27-Jun-2010

--*/

#ifndef _VFFUNCS_H
#define _VFFUNCS_H

//
// Dependencies
//
#include <umtypes.h>
#include <vftypes.h>

#ifndef NTOS_MODE_USER

//
// Verifier Device Driver Interface
//
BOOLEAN
NTAPI
VfIsVerificationEnabled(
    IN VF_OBJECT_TYPE VfObjectType,
    IN PVOID Object OPTIONAL
);
    
VOID
NTAPI
VfFailDeviceNode(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN ULONG BugCheckMajorCode,
    IN ULONG BugCheckMinorCode,
    IN VF_FAILURE_CLASS FailureClass,
    IN OUT PULONG AssertionControl,
    IN PSTR DebuggerMessageText,
    IN PSTR ParameterFormatString,
    ...
);
#endif

#endif
