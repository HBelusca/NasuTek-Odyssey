/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/msvcrt/mbstring/mbsncoll.c
 * PURPOSE:
 * PROGRAMER:   Ariadne
 * UPDATE HISTORY:
 *              12/04/99: Created
 */
#include <mbstring.h>

int colldif(unsigned short c1, unsigned short c2);

/*
 * @implemented
 */
int _mbsncoll(const unsigned char *str1, const unsigned char *str2, size_t n)
{
	unsigned char *s1 = (unsigned char *)str1;
	unsigned char *s2 = (unsigned char *)str2;

	unsigned short *short_s1, *short_s2;

	int l1, l2;

	if (n == 0)
		return 0;
	do {

		if (*s1 == 0)
			break;

		l1 = _ismbblead(*s1);
		l2 = _ismbblead(*s2);
		if ( !l1 &&  !l2  ) {

			if (*s1 != *s2)
				return colldif(*s1, *s2);
			else {
				s1 += 1;
				s2 += 1;
				n--;
			}
		}
		else if ( l1 && l2 ){
			short_s1 = (unsigned short *)s1;
			short_s2 = (unsigned short *)s2;
			if ( *short_s1 != *short_s2 )
				return colldif(*short_s1, *short_s2);
			else {
				s1 += 2;
				s2 += 2;
				n--;

			}
		}
		else
			return colldif(*s1, *s2);
	} while (n > 0);
	return 0;
}

/*
 * @implemented
 */
int _mbsnbcoll(const unsigned char *str1, const unsigned char *str2, size_t n)
{
	unsigned char *s1 = (unsigned char *)str1;
	unsigned char *s2 = (unsigned char *)str2;

	unsigned short *short_s1, *short_s2;

	int l1, l2;

	if (n == 0)
		return 0;
	do {

		if (*s1 == 0)
			break;

		l1 = _ismbblead(*s1);
		l2 = _ismbblead(*s2);
		if ( !l1 &&  !l2  ) {

			if (*s1 != *s2)
				return colldif(*s1, *s2);
			else {
				s1 += 1;
				s2 += 1;
				n--;
			}
		}
		else if ( l1 && l2 ){
			short_s1 = (unsigned short *)s1;
			short_s2 = (unsigned short *)s2;
			if ( *short_s1 != *short_s2 )
				return colldif(*short_s1, *short_s2);
			else {
				s1 += 2;
				s2 += 2;
				n-=2;

			}
		}
		else
			return colldif(*s1, *s2);
	} while (n > 0);
	return 0;
}

int colldif(unsigned short c1, unsigned short c2)
{
  return c1 - c2;
}
