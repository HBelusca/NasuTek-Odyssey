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

/*
 * @implemented
 */
char * _strrev(char *s)
{
	char a, *b, *e;
	b=e=s;
	while (*e)
		e++;
	e--; /* start at last char, not NULL char */
	while ( b < e )
	{
		a=*b;
		*b=*e;
		*e=a;
		b++;
		e--;
	}
	return s; /* return ptr to beginning of string */
}
