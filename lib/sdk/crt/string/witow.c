/* $Id: witow.c 30266 2007-11-08 10:54:42Z fireball $
 *
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/msvcrt/stdlib/itow.c
 * PURPOSE:     converts a integer to wchar_t
 * PROGRAMER:
 * UPDATE HISTORY:
 *              1995: Created
 *              1998: Added ltoa by Ariadne
 *              2000: derived from ./itoa.c by ea
 */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

#include <precomp.h>

/*
 * @implemented
 */
wchar_t* _i64tow(__int64 value, wchar_t* string, int radix)
{
  wchar_t tmp[65];
  wchar_t* tp = tmp;
  int i;
  unsigned v;
  int sign;
  wchar_t* sp;

  if (radix > 36 || radix <= 1) {
    __set_errno(EDOM);
    return 0;
  }

  sign = (radix == 10 && value < 0);
  if (sign)
    v = -value;
  else
    v = (unsigned)value;
  while (v || tp == tmp) {
    i = v % radix;
    v = v / radix;
    if (i < 10)
      *tp++ = i+L'0';
    else
      *tp++ = i + L'a' - 10;
  }

  if (string == 0)
    string = (wchar_t*)malloc(((tp-tmp)+sign+1)*sizeof(wchar_t));
  sp = string;

  if (sign)
    *sp++ = L'-';
  while (tp > tmp)
    *sp++ = *--tp;
  *sp = 0;
  return string;
}

/*
 * @implemented
 */
wchar_t* _ui64tow(unsigned __int64 value, wchar_t* string, int radix)
{
  wchar_t tmp[65];
  wchar_t* tp = tmp;
  long i;
  unsigned long v = value;
  wchar_t* sp;

  if (radix > 36 || radix <= 1) {
    __set_errno(EDOM);
    return 0;
  }

  while (v || tp == tmp) {
    i = v % radix;
    v = v / radix;
    if (i < 10)
      *tp++ = i+L'0';
    else
      *tp++ = i + L'a' - 10;
  }

  if (string == 0)
    string = (wchar_t*)malloc(((tp-tmp)+1)*sizeof(wchar_t));
  sp = string;

  while (tp > tmp)
    *sp++ = *--tp;
  *sp = 0;
  return string;
}
