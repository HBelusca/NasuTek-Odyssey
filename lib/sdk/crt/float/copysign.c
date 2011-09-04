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
#include <internal/ieee.h>

/*
 * @implemented
 */
double _copysign (double __d, double __s)
{
  union
  {
      double*	__d;
      double_s*	  d;
  } d;
  union
  {
      double*	__s;
      double_s*   s;
  } s;
  d.__d = &__d;
  s.__s = &__s;

  d.d->sign = s.s->sign;

  return __d;
}

