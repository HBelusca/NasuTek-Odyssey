/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/msvcrt/conio/putch.c
 * PURPOSE:     Writes a character to stdout
 * PROGRAMER:   Ariadne
 * UPDATE HISTORY:
 *              28/12/98: Created
 */

#include <precomp.h>

/*
 * @implemented
 */
int _putch(int c)
{
  DWORD NumberOfCharsWritten;

  if (WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),&c,1,&NumberOfCharsWritten,NULL)) {
    return -1;
  }
  return NumberOfCharsWritten;
}
