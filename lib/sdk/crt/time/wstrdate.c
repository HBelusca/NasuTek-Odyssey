/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey CRT library
 * FILE:        lib/sdk/crt/time/strtime.c
 * PURPOSE:     Fills a buffer with a formatted date representation
 * PROGRAMER:   Ariadne
 * UPDATE HISTORY:
 *              28/12/98: Created
 */
#include <precomp.h>

/*
 * @implemented
 */
wchar_t* _wstrdate(wchar_t* date)
{
   static const WCHAR format[] = { 'M','M','\'','/','\'','d','d','\'','/','\'','y','y',0 };

   GetDateFormatW(LOCALE_NEUTRAL, 0, NULL, format, (LPWSTR)date, 9);

   return date;

}
