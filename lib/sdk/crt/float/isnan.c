/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <precomp.h>


/*
 * @implemented
 */
int _isnan(double __x)
{
	union
	{
		double*   __x;
		double_s*   x;
	} x;
    	x.__x = &__x;
	return ( x.x->exponent == 0x7ff  && ( x.x->mantissah != 0 || x.x->mantissal != 0 ));
}

int _isnanl(long double __x)
{
	/* Intel's extended format has the normally implicit 1 explicit
	   present.  Sigh!  */
	union
	{
		long double*   __x;
		long_double_s*   x;
	} x;
	x.__x = &__x;


	 /* IEEE 854 NaN's have the maximum possible
     exponent and a nonzero mantissa.  */

	return (( x.x->exponent == 0x7fff)
	  && ( (x.x->mantissah & 0x80000000) != 0)
	  && ( (x.x->mantissah & (unsigned int)0x7fffffff) != 0  || x.x->mantissal != 0 ));
}

int _isinf(double __x)
{
	union
	{
		double*   __x;
		double_s*   x;
	} x;

	x.__x = &__x;
	return ( x.x->exponent == 0x7ff  && ( x.x->mantissah == 0 && x.x->mantissal == 0 ));
}

/*
 * @implemented
 */
int _finite( double x )
{
	return !_isinf(x);
}

int _isinfl(long double __x)
{
	/* Intel's extended format has the normally implicit 1 explicit
	   present.  Sigh!  */
	union
	{
		long double*   __x;
                long_double_s*   x;
	} x;

	x.__x = &__x;


	 /* An IEEE 854 infinity has an exponent with the
     maximum possible value and a zero mantissa.  */


	if ( x.x->exponent == 0x7fff  && ( (x.x->mantissah == 0x80000000 )   && x.x->mantissal == 0 ))
		return x.x->sign ? -1 : 1;
	return 0;
}
