/* $Id: tuiconsole.h 21947 2006-05-20 10:49:56Z fireball $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            subsys/csrss/win32csr/tuiconsole.h
 * PURPOSE:         Interface to text-mode consoles
 */

#include "api.h"

extern NTSTATUS FASTCALL TuiInitConsole(PCSRSS_CONSOLE Console);
extern PCSRSS_CONSOLE FASTCALL TuiGetFocusConsole(VOID);
extern BOOL FASTCALL TuiSwapConsole(int Next);

/* EOF */
