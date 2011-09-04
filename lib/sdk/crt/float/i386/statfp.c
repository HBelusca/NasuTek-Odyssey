/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/crt/??????
 * PURPOSE:     Unknown
 * PROGRAMER:   Unknown
 * UPDATE HISTORY:
 *              25/11/05: Added license header
 */

#include <precomp.h>
#include "float.h"

//WTF IS HAPPENING WITH float.h !??!?!
#define _SW_INVALID    0x00000010 /* invalid */
#define _SW_ZERODIVIDE 0x00000008 /* zero divide */
#define _SW_UNDERFLOW  0x00000002 /* underflow */
#define _SW_OVERFLOW   0x00000004 /* overflow */
#define _SW_INEXACT    0x00000001 /* inexact (precision) */
#define _SW_DENORMAL    0x00080000 /* denormal status bit */

/**********************************************************************
 *		_statusfp (MSVCRT.@)
 */
unsigned int CDECL _statusfp(void)
{
  unsigned int retVal = 0;
  unsigned int fpword;
#if defined(__GNUC__)
  __asm__ __volatile__( "fstsw %0" : "=m" (fpword) : );
#elif defined(_M_IX86)
  __asm fstsw [fpword];
#else
  #pragma message("FIXME: _statusfp is halfplemented")
#endif
  if (fpword & 0x1)  retVal |= _SW_INVALID;
  if (fpword & 0x2)  retVal |= _SW_DENORMAL;
  if (fpword & 0x4)  retVal |= _SW_ZERODIVIDE;
  if (fpword & 0x8)  retVal |= _SW_OVERFLOW;
  if (fpword & 0x10) retVal |= _SW_UNDERFLOW;
  if (fpword & 0x20) retVal |= _SW_INEXACT;
  return retVal;
}
