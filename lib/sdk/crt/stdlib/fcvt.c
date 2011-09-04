/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
#include <precomp.h>

char *fcvtbuf (double, int, int *, int *, char *);

/*
 * @implemented
 */
char *
_fcvt (double value, int ndigits, int *decpt, int *sign)
{
  static char fcvt_buf[2 * DBL_MAX_10_EXP + 10];
  return fcvtbuf (value, ndigits, decpt, sign, fcvt_buf);
}
