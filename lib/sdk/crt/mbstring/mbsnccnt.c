#include <mbstring.h>

/*
 * @implemented
 */
size_t _mbsnccnt(const unsigned char *str, size_t n)
{
	unsigned char *s = (unsigned char *)str;
	size_t cnt = 0;
	while(*s != 0 && n > 0) {
		if (_ismbblead(*s) )
			s++;
		else
			n--;
		s++;
		cnt++;
	}

	return cnt;
}

/*
 * @implemented
 */
size_t _mbsnbcnt(const unsigned char *str, size_t n)
{
	unsigned char *s = (unsigned char *)str;
	while(*s != 0 && n > 0) {
		if (!_ismbblead(*s) )
			n--;
		s++;
	}

	return (size_t)(s - str);
}
