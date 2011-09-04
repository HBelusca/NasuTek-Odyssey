/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey System Libraries
 * FILE:            lib/secur32/precomp.h
 * PURPOSE:         Security Library Header
 * PROGRAMMER:      Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/

/* SDK/DDK/NDK Headers. */
#define WIN32_NO_STATUS
#include <windows.h>
#define NTOS_MODE_USER
#include <ndk/rtlfuncs.h>
#include <lsass/lsass.h>

#include <ntsecapi.h>
#include <secext.h>
#include <security.h>
#include <sspi.h>
