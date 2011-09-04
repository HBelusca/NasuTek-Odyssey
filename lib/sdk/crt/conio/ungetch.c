/*
 * COPYRIGHT:   See COPYING in the top level directory
 * 		Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/msvcrt/conio/ungetch.c
 * PURPOSE:     Ungets a character from stdin
 * PROGRAMER:   DJ Delorie
		Ariadne [ Adapted from djgpp libc ]
 * UPDATE HISTORY:
 *              28/12/98: Created
 */

#include <precomp.h>

int char_avail = 0;
int ungot_char = 0;


/*
 * @implemented
 */
int _ungetch(int c)
{
  if (char_avail)
    return(EOF);
  ungot_char = c;
  char_avail = 1;
  return(c);
}
