/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey Kernel Streaming
 * FILE:            drivers/wdm/audio/backpln/portcls/dll.cpp
 * PURPOSE:         portcls generic dispatcher
 * PROGRAMMER:      Andrew Greenwood
 */


#include "private.hpp"

ULONG
NTAPI
DllInitialize(ULONG Unknown)
{
    return 0;
}


extern
"C"
{
ULONG
NTAPI
DllUnload()
{
    return 0;
}
}
