/* $Id: stubs.c 39444 2009-02-06 19:07:22Z cwittich $
 *
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey WinSock DLL
 * FILE:        stubs.c
 * PURPOSE:     Stub functions
 * PROGRAMMERS: Ge van Geldorp (ge@gse.nl)
 * REVISIONS:
 */
#include <winsock2.h>
#include <windows.h>
//#include <stdlib.h>


/*
 * @unimplemented
 */
BOOL
WINAPI
DllMain(HINSTANCE InstDLL,
        DWORD Reason,
        LPVOID Reserved)
{
  return TRUE;
}
