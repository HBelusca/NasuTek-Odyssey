/* $Id: tcsncpy.h 30283 2007-11-08 21:06:20Z fireball $
 */

#include <stddef.h>
#include <tchar.h>

_TCHAR * _tcsncpy(_TCHAR * dst, const _TCHAR * src, size_t n)
{
 if(n != 0)
 {
  _TCHAR * d = dst;
  const _TCHAR * s = src;

  do
  {
   if((*d ++ = *s ++) == 0)
   {
    while (-- n != 0) *d ++ = 0;
    break;
   }
  }
  while(-- n != 0);
 }

 return dst;
}

/* EOF */
