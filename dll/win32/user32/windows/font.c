/*
 *  Odyssey kernel
 *  Copyright (C) 1998, 1999, 2000, 2001 ReactOS Team; (C) 2011 NasuTek Enterprises
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* $Id: font.c 48387 2010-07-31 22:36:22Z fireball $
 *
 * PROJECT:         Odyssey user32.dll
 * FILE:            lib/user32/windows/input.c
 * PURPOSE:         Input
 * PROGRAMMER:      Casper S. Hornstrup (chorns@users.sourceforge.net)
 * UPDATE HISTORY:
 *      09-05-2001  CSH  Created
 */

/* INCLUDES ******************************************************************/

#include <user32.h>

#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(text);

/* FUNCTIONS *****************************************************************/

#ifndef NDEBUG

#ifdef assert
#undef assert
#endif

#define assert(e) ((e) ? (void)0 : _font_assert(#e, __FILE__, __LINE__))

#endif

void _font_assert(const char *msg, const char *file, int line)
{
  /* Assertion failed at foo.c line 45: x<y */
  DbgPrint("Assertion failed at %s line %d: %s\n", file, line, msg);
  ExitProcess(3);
  for(;;); /* eliminate warning by mingw */
}

/*
 * @implemented
 */
LONG
WINAPI
TabbedTextOutA(
  HDC hDC,
  int X,
  int Y,
  LPCSTR lpString,
  int nCount,
  int nTabPositions,
  CONST INT *lpnTabStopPositions,
  int nTabOrigin)
{
  LONG ret;
  DWORD len;
  LPWSTR strW;

  len = MultiByteToWideChar(CP_ACP, 0, lpString, nCount, NULL, 0);
  strW = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
  if (!strW)
    {
	  return 0;
    }
  MultiByteToWideChar(CP_ACP, 0, lpString, nCount, strW, len);
  ret = TabbedTextOutW(hDC, X, Y, strW, len, nTabPositions, lpnTabStopPositions, nTabOrigin);
  HeapFree(GetProcessHeap(), 0, strW);
  return ret;
}


/***********************************************************************
 *           TEXT_TabbedTextOut
 *
 * Helper function for TabbedTextOut() and GetTabbedTextExtent().
 * Note: this doesn't work too well for text-alignment modes other
 *       than TA_LEFT|TA_TOP. But we want bug-for-bug compatibility :-)
 */
/* WINE synced 22-May-2006 */
static LONG TEXT_TabbedTextOut( HDC hdc, INT x, INT y, LPCWSTR lpstr,
                                INT count, INT cTabStops, const INT *lpTabPos, INT nTabOrg,
                                BOOL fDisplayText )
{
    INT defWidth;
    SIZE extent;
    int i, j;
    int start = x;

    if (!lpTabPos)
        cTabStops=0;

    if (cTabStops == 1)
    {
        defWidth = *lpTabPos;
        cTabStops = 0;
    }
    else
    {
        TEXTMETRICA tm;
        GetTextMetricsA( hdc, &tm );
        defWidth = 8 * tm.tmAveCharWidth;
    }

    while (count > 0)
    {
        RECT r;
        INT x0;
        x0 = x;
        r.left = x0;
        /* chop the string into substrings of 0 or more <tabs>
         * possibly followed by 1 or more normal characters */
        for (i = 0; i < count; i++)
            if (lpstr[i] != '\t') break;
        for (j = i; j < count; j++)
            if (lpstr[j] == '\t') break;
        /* get the extent of the normal character part */
        GetTextExtentPointW( hdc, lpstr + i, j - i , &extent );
        /* and if there is a <tab>, calculate its position */
        if( i) {
            /* get x coordinate for the drawing of this string */
            for (; cTabStops > i; lpTabPos++, cTabStops--)
            {
                if( nTabOrg + abs( *lpTabPos) > x) {
                    if( lpTabPos[ i - 1] >= 0) {
                        /* a left aligned tab */
                        x = nTabOrg + lpTabPos[ i-1] + extent.cx;
                        break;
                    }
                    else
                    {
                        /* if tab pos is negative then text is right-aligned
                         * to tab stop meaning that the string extends to the
                         * left, so we must subtract the width of the string */
                        if (nTabOrg - lpTabPos[ i - 1] - extent.cx > x)
                        {
                            x = nTabOrg - lpTabPos[ i - 1];
                            x0 = x - extent.cx;
                            break;
                        }
                    }
                }
            }
            /* if we have run out of tab stops and we have a valid default tab
             * stop width then round x up to that width */
            if ((cTabStops <= i) && (defWidth > 0)) {
                x0 = nTabOrg + ((x - nTabOrg) / defWidth + i) * defWidth;
                x = x0 + extent.cx;
            } else if ((cTabStops <= i) && (defWidth < 0)) {
                x = nTabOrg + ((x - nTabOrg + extent.cx) / -defWidth + i)
                    * -defWidth;
                x0 = x - extent.cx;
            }
        } else
            x += extent.cx;

        if (fDisplayText)
        {
            r.top    = y;
            r.right  = x;
            r.bottom = y + extent.cy;
            ExtTextOutW( hdc, x0, y, GetBkMode(hdc) == OPAQUE ? ETO_OPAQUE : 0,
                         &r, lpstr + i, j - i, NULL );
        }
        count -= j;
        lpstr += j;
    }
    return MAKELONG(x - start, extent.cy);
}

/*
 * @implemented
 */
LONG
WINAPI
TabbedTextOutW(
  HDC hDC,
  int X,
  int Y,
  LPCWSTR lpString,
  int nCount,
  int nTabPositions,
  CONST INT *lpnTabStopPositions,
  int nTabOrigin)
{
  return TEXT_TabbedTextOut(hDC, X, Y, lpString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin, TRUE);
}

/* WINE synced 22-May-2006 */
/*
 * @implemented
 */
DWORD
WINAPI
GetTabbedTextExtentA(
  HDC hDC,
  LPCSTR lpString,
  int nCount,
  int nTabPositions,
  CONST INT *lpnTabStopPositions)
{
    LONG ret;
    DWORD len = MultiByteToWideChar(CP_ACP, 0, lpString, nCount, NULL, 0);
    LPWSTR strW = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));

    if (!strW)
        return 0;
    MultiByteToWideChar(CP_ACP, 0, lpString, nCount, strW, len);

    ret = GetTabbedTextExtentW(hDC, strW, len, nTabPositions,
        lpnTabStopPositions);

    HeapFree(GetProcessHeap(), 0, strW);
    return ret;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetTabbedTextExtentW(
  HDC hDC,
  LPCWSTR lpString,
  int nCount,
  int nTabPositions,
  CONST INT *lpnTabStopPositions)
{
   return TEXT_TabbedTextOut(hDC, 0, 0, lpString, nCount, nTabPositions, lpnTabStopPositions, 0, FALSE);
}

/*********************************************************************
 *
 *            DrawText functions
 *
 * Copied from Wine.
 *   Copyright 1993, 1994 Alexandre Julliard
 *   Copyright 2002 Bill Medland
 *
 * Design issues
 *   How many buffers to use
 *     While processing in DrawText there are potentially three different forms
 *     of the text that need to be held.  How are they best held?
 *     1. The original text is needed, of course, to see what to display.
 *     2. The text that will be returned to the user if the DT_MODIFYSTRING is
 *        in effect.
 *     3. The buffered text that is about to be displayed e.g. the current line.
 *        Typically this will exclude the ampersands used for prefixing etc.
 *
 *     Complications.
 *     a. If the buffered text to be displayed includes the ampersands then
 *        we will need special measurement and draw functions that will ignore
 *        the ampersands (e.g. by copying to a buffer without the prefix and
 *        then using the normal forms).  This may involve less space but may
 *        require more processing.  e.g. since a line containing tabs may
 *        contain several underlined characters either we need to carry around
 *        a list of prefix locations or we may need to locate them several
 *        times.
 *     b. If we actually directly modify the "original text" as we go then we
 *        will need some special "caching" to handle the fact that when we
 *        ellipsify the text the ellipsis may modify the next line of text,
 *        which we have not yet processed.  (e.g. ellipsification of a W at the
 *        end of a line will overwrite the W, the \n and the first character of
 *        the next line, and a \0 will overwrite the second.  Try it!!)
 *
 *     Option 1.  Three separate storages. (To be implemented)
 *       If DT_MODIFYSTRING is in effect then allocate an extra buffer to hold
 *       the edited string in some form, either as the string itself or as some
 *       sort of "edit list" to be applied just before returning.
 *       Use a buffer that holds the ellipsified current line sans ampersands
 *       and accept the need occasionally to recalculate the prefixes (if
 *       DT_EXPANDTABS and not DT_NOPREFIX and not DT_HIDEPREFIX)
 */

#define TAB     9
#define LF     10
#define CR     13
#define SPACE  32
#define PREFIX 38

#define FORWARD_SLASH '/'
#define BACK_SLASH '\\'

static const WCHAR ELLIPSISW[] = {'.','.','.', 0};

typedef struct tag_ellipsis_data
{
    int before;
    int len;
    int under;
    int after;
} ellipsis_data;

/*********************************************************************
 *                      TEXT_Ellipsify (static)
 *
 *  Add an ellipsis to the end of the given string whilst ensuring it fits.
 *
 * If the ellipsis alone doesn't fit then it will be returned anyway.
 *
 * See Also TEXT_PathEllipsify
 *
 * Arguments
 *   hdc        [in] The handle to the DC that defines the font.
 *   str        [in/out] The string that needs to be modified.
 *   max_str    [in] The dimension of str (number of WCHAR).
 *   len_str    [in/out] The number of characters in str
 *   width      [in] The maximum width permitted (in logical coordinates)
 *   size       [out] The dimensions of the text
 *   modstr     [out] The modified form of the string, to be returned to the
 *                    calling program.  It is assumed that the caller has
 *                    made sufficient space available so we don't need to
 *                    know the size of the space.  This pointer may be NULL if
 *                    the modified string is not required.
 *   len_before [out] The number of characters before the ellipsis.
 *   len_ellip  [out] The number of characters in the ellipsis.
 *
 * See for example Microsoft article Q249678.
 *
 * For now we will simply use three dots rather than worrying about whether
 * the font contains an explicit ellipsis character.
 */
static void TEXT_Ellipsify (HDC hdc, WCHAR *str, unsigned int max_len,
                            unsigned int *len_str, int width, SIZE *size,
                            WCHAR *modstr,
                            int *len_before, int *len_ellip)
{
    unsigned int len_ellipsis;
    unsigned int lo, mid, hi;

    len_ellipsis = strlenW (ELLIPSISW);
    if (len_ellipsis > max_len) len_ellipsis = max_len;
    if (*len_str > max_len - len_ellipsis)
        *len_str = max_len - len_ellipsis;

    /* First do a quick binary search to get an upper bound for *len_str. */
    if (*len_str > 0 &&
        GetTextExtentExPointW(hdc, str, *len_str, width, NULL, NULL, size) &&
        size->cx > width)
    {
        for (lo = 0, hi = *len_str; lo < hi; )
        {
            mid = (lo + hi) / 2;
            if (!GetTextExtentExPointW(hdc, str, mid, width, NULL, NULL, size))
                break;
            if (size->cx > width)
                hi = mid;
            else
                lo = mid + 1;
        }
        *len_str = hi;
    }
    /* Now this should take only a couple iterations at most. */
    for ( ; ; )
    {
        memcpy(str + *len_str, ELLIPSISW, len_ellipsis*sizeof(WCHAR));

        if (!GetTextExtentExPointW (hdc, str, *len_str + len_ellipsis, width,
                                    NULL, NULL, size)) break;

        if (!*len_str || size->cx <= width) break;

        (*len_str)--;
    }
    *len_ellip = len_ellipsis;
    *len_before = *len_str;
    *len_str += len_ellipsis;

    if (modstr)
    {
        memcpy (modstr, str, *len_str * sizeof(WCHAR));
        modstr[*len_str] = '\0';
    }
}

/*********************************************************************
 *                      TEXT_PathEllipsify (static)
 *
 * Add an ellipsis to the provided string in order to make it fit within
 * the width.  The ellipsis is added as specified for the DT_PATH_ELLIPSIS
 * flag.
 *
 * See Also TEXT_Ellipsify
 *
 * Arguments
 *   hdc        [in] The handle to the DC that defines the font.
 *   str        [in/out] The string that needs to be modified
 *   max_str    [in] The dimension of str (number of WCHAR).
 *   len_str    [in/out] The number of characters in str
 *   width      [in] The maximum width permitted (in logical coordinates)
 *   size       [out] The dimensions of the text
 *   modstr     [out] The modified form of the string, to be returned to the
 *                    calling program.  It is assumed that the caller has
 *                    made sufficient space available so we don't need to
 *                    know the size of the space.  This pointer may be NULL if
 *                    the modified string is not required.
 *   pellip     [out] The ellipsification results
 *
 * For now we will simply use three dots rather than worrying about whether
 * the font contains an explicit ellipsis character.
 *
 * The following applies, I think to Win95.  We will need to extend it for
 * Win98 which can have both path and end ellipsis at the same time (e.g.
 *  C:\MyLongFileName.Txt becomes ...\MyLongFileN...)
 *
 * The resulting string consists of as much as possible of the following:
 * 1. The ellipsis itself
 * 2. The last \ or / of the string (if any)
 * 3. Everything after the last \ or / of the string (if any) or the whole
 *    string if there is no / or \.  I believe that under Win95 this would
 *    include everything even though some might be clipped off the end whereas
 *    under Win98 that might be ellipsified too.
 *    Yet to be investigated is whether this would include wordbreaking if the
 *    filename is more than 1 word and splitting if DT_EDITCONTROL was in
 *    effect.  (If DT_EDITCONTROL is in effect then on occasions text will be
 *    broken within words).
 * 4. All the stuff before the / or \, which is placed before the ellipsis.
 */
static void TEXT_PathEllipsify (HDC hdc, WCHAR *str, unsigned int max_len,
                                unsigned int *len_str, int width, SIZE *size,
                                WCHAR *modstr, ellipsis_data *pellip)
{
    int len_ellipsis;
    int len_trailing;
    int len_under;
    WCHAR *lastBkSlash, *lastFwdSlash, *lastSlash;

    len_ellipsis = strlenW (ELLIPSISW);
    if (!max_len) return;
    if (len_ellipsis >= max_len) len_ellipsis = max_len - 1;
    if (*len_str + len_ellipsis >= max_len)
        *len_str = max_len - len_ellipsis-1;
        /* Hopefully this will never happen, otherwise it would probably lose
         * the wrong character
         */
    str[*len_str] = '\0'; /* to simplify things */

    lastBkSlash  = strrchrW (str, BACK_SLASH);
    lastFwdSlash = strrchrW (str, FORWARD_SLASH);
    lastSlash = lastBkSlash > lastFwdSlash ? lastBkSlash : lastFwdSlash;
    if (!lastSlash) lastSlash = str;
    len_trailing = *len_str - (lastSlash - str);

    /* overlap-safe movement to the right */
    memmove (lastSlash+len_ellipsis, lastSlash, len_trailing * sizeof(WCHAR));
    memcpy (lastSlash, ELLIPSISW, len_ellipsis*sizeof(WCHAR));
    len_trailing += len_ellipsis;
    /* From this point on lastSlash actually points to the ellipsis in front
     * of the last slash and len_trailing includes the ellipsis
     */

    len_under = 0;
    for ( ; ; )
    {
        if (!GetTextExtentExPointW (hdc, str, *len_str + len_ellipsis, width,
                                    NULL, NULL, size)) break;

        if (lastSlash == str || size->cx <= width) break;

        /* overlap-safe movement to the left */
        memmove (lastSlash-1, lastSlash, len_trailing * sizeof(WCHAR));
        lastSlash--;
        len_under++;

        assert (*len_str);
        (*len_str)--;
    }
    pellip->before = lastSlash-str;
    pellip->len = len_ellipsis;
    pellip->under = len_under;
    pellip->after = len_trailing - len_ellipsis;
    *len_str += len_ellipsis;

    if (modstr)
    {
        memcpy(modstr, str, *len_str * sizeof(WCHAR));
        modstr[*len_str] = '\0';
    }
}

/* Check the character is Chinese, Japanese, Korean and/or Thai */
inline BOOL IsCJKT(WCHAR wch)
{
    if (0x0E00 <= wch && wch <= 0x0E7F)
        return TRUE;    /* Thai */

    if (0x3000 <= wch && wch <= 0x9FFF)
        return TRUE;    /* CJK */

    if (0xAC00 <= wch && wch <= 0xD7FF)
        return TRUE;    /* Korean */

    if (0xFF00 <= wch && wch <= 0xFFEF)
        return TRUE;    /* CJK */

    return FALSE;
}

/* See http://en.wikipedia.org/wiki/Kinsoku_shori */
static const WCHAR KinsokuClassA[] =
{
    0x2010, 0x2013, 0x2019, 0x201D, 0x203C, 0x2047, 0x2048, 0x2049, 0x3001, 
    0x3002, 0x3005, 0x3009, 0x300B, 0x300D, 0x300F, 0x3011, 0x3015, 0x3017, 
    0x3019, 0x301C, 0x301F, 0x303B, 0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 
    0x3063, 0x3083, 0x3085, 0x3087, 0x308E, 0x3095, 0x3096, 0x30A0, 0x30A1, 
    0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5, 0x30E7, 0x30EE, 
    0x30F5, 0x30F6, 0x30FB, 0x30FC, 0x30FD, 0x30FE, 0x31F0, 0x31F1, 0x31F2, 
    0x31F3, 0x31F4, 0x31F5, 0x31F6, 0x31F7, 0x31F8, 0x31F9, 0x31FA, 0x31FB, 
    0x31FC, 0x31FD, 0x31FE, 0x31FF, 0xFF01, 0xFF09, 0xFF0C, 0xFF0E, 0xFF1A, 
    0xFF1B, 0xFF1F, 0xFF3D, 0xFF5D, 0xFF60, 0
};

/*********************************************************************
 *                      TEXT_WordBreak (static)
 *
 *  Perform wordbreak processing on the given string
 *
 * Assumes that DT_WORDBREAK has been specified and not all the characters
 * fit.  Note that this function should even be called when the first character
 * that doesn't fit is known to be a space or tab, so that it can swallow them.
 *
 * Note that the Windows processing has some strange properties.
 * 1. If the text is left-justified and there is room for some of the spaces
 *    that follow the last word on the line then those that fit are included on
 *    the line.
 * 2. If the text is centred or right-justified and there is room for some of
 *    the spaces that follow the last word on the line then all but one of those
 *    that fit are included on the line.
 * 3. (Reasonable behaviour) If the word breaking causes a space to be the first
 *    character of a new line it will be skipped.
 *
 * Arguments
 *   hdc        [in] The handle to the DC that defines the font.
 *   str        [in/out] The string that needs to be broken.
 *   max_str    [in] The dimension of str (number of WCHAR).
 *   len_str    [in/out] The number of characters in str
 *   width      [in] The maximum width permitted
 *   format     [in] The format flags in effect
 *   chars_fit  [in] The maximum number of characters of str that are already
 *              known to fit; chars_fit+1 is known not to fit.
 *   chars_used [out] The number of characters of str that have been "used" and
 *              do not need to be included in later text.  For example this will
 *              include any spaces that have been discarded from the start of
 *              the next line.
 *   size       [out] The size of the returned text in logical coordinates
 *
 * Pedantic assumption - Assumes that the text length is monotonically
 * increasing with number of characters (i.e. no weird kernings)
 *
 * Algorithm
 *
 * Work back from the last character that did fit to either a space or the last
 * character of a word, whichever is met first.
 * If there was one or the first character didn't fit then
 *     If the text is centred or right justified and that one character was a
 *     space then break the line before that character
 *     Otherwise break the line after that character
 *     and if the next character is a space then discard it.
 * Suppose there was none (and the first character did fit).
 *     If Break Within Word is permitted
 *         break the word after the last character that fits (there must be
 *         at least one; none is caught earlier).
 *     Otherwise
 *         discard any trailing space.
 *         include the whole word; it may be ellipsified later
 *
 * Break Within Word is permitted under a set of circumstances that are not
 * totally clear yet.  Currently our best guess is:
 *     If DT_EDITCONTROL is in effect and neither DT_WORD_ELLIPSIS nor
 *     DT_PATH_ELLIPSIS is
 */

static void TEXT_WordBreak (HDC hdc, WCHAR *str, unsigned int max_str,
                            unsigned int *len_str,
                            int width, int format, unsigned int chars_fit,
                            unsigned int *chars_used, SIZE *size)
{
    WCHAR *p;
    int word_fits;
    assert (format & DT_WORDBREAK);
    assert (chars_fit < *len_str);

    /* Work back from the last character that did fit to either a space or the
     * last character of a word, whichever is met first.
     */
    p = str + chars_fit; /* The character that doesn't fit */
    word_fits = TRUE;
    if (!chars_fit)
        ; /* we pretend that it fits anyway */
    else if (*p == SPACE) /* chars_fit < *len_str so this is valid */
        p--; /* the word just fitted */
    else
    {
        while (p > str && *(--p) != SPACE && (!IsCJKT(p[1]) || 
                p[1] == L'\0' || wcschr(KinsokuClassA, p[1]) != NULL))
            ;
        word_fits = (p != str || *p == SPACE || IsCJKT(p[1]));
    }
    /* If there was one or the first character didn't fit then */
    if (word_fits)
    {
        int next_is_space;
        /* break the line before/after that character */
        if (!(format & (DT_RIGHT | DT_CENTER)) || *p != SPACE)
            p++;
        next_is_space = (unsigned int) (p - str) < *len_str && *p == SPACE;
        *len_str = p - str;
        /* and if the next character is a space then discard it. */
        *chars_used = *len_str;
        if (next_is_space)
            (*chars_used)++;
    }
    /* Suppose there was none. */
    else
    {
        if ((format & (DT_EDITCONTROL | DT_WORD_ELLIPSIS | DT_PATH_ELLIPSIS)) ==
            DT_EDITCONTROL)
        {
            /* break the word after the last character that fits (there must be
             * at least one; none is caught earlier).
             */
            *len_str = chars_fit;
            *chars_used = chars_fit;

            /* FIXME - possible error.  Since the next character is now removed
             * this could make the text longer so that it no longer fits, and
             * so we need a loop to test and shrink.
             */
        }
        /* Otherwise */
        else
        {
            /* discard any trailing space. */
            const WCHAR *e = str + *len_str;
            p = str + chars_fit;
            while (p < e && *p != SPACE)
                p++;
            *chars_used = p - str;
            if (p < e) /* i.e. loop failed because *p == SPACE */
                (*chars_used)++;

            /* include the whole word; it may be ellipsified later */
            *len_str = p - str;
            /* Possible optimisation; if DT_WORD_ELLIPSIS only use chars_fit+1
             * so that it will be too long
             */
        }
    }
    /* Remeasure the string */
    GetTextExtentExPointW (hdc, str, *len_str, 0, NULL, NULL, size);
}

/*********************************************************************
 *                      TEXT_SkipChars
 *
 *  Skip over the given number of characters, bearing in mind prefix
 *  substitution and the fact that a character may take more than one
 *  WCHAR (Unicode surrogates are two words long) (and there may have been
 *  a trailing &)
 *
 * Parameters
 *   new_count  [out] The updated count
 *   new_str    [out] The updated pointer
 *   start_count [in] The count of remaining characters corresponding to the
 *                    start of the string
 *   start_str  [in] The starting point of the string
 *   max        [in] The number of characters actually in this segment of the
 *                   string (the & counts)
 *   n          [in] The number of characters to skip (if prefix then
 *                   &c counts as one)
 *   prefix     [in] Apply prefix substitution
 *
 * Return Values
 *   none
 *
 * Remarks
 *   There must be at least n characters in the string
 *   We need max because the "line" may have ended with a & followed by a tab
 *   or newline etc. which we don't want to swallow
 */

static void TEXT_SkipChars (int *new_count, const WCHAR **new_str,
                            int start_count, const WCHAR *start_str,
                            int max, int n, int prefix)
{
    /* This is specific to wide characters, MSDN doesn't say anything much
     * about Unicode surrogates yet and it isn't clear if _wcsinc will
     * correctly handle them so we'll just do this the easy way for now
     */

    if (prefix)
    {
        const WCHAR *str_on_entry = start_str;
        assert (max >= n);
        max -= n;
        while (n--)
        {
            if (*start_str++ == PREFIX && max--)
                start_str++;
        }
        start_count -= (start_str - str_on_entry);
    }
    else
    {
        start_str += n;
        start_count -= n;
    }
    *new_str = start_str;
    *new_count = start_count;
}

/*********************************************************************
 *                      TEXT_Reprefix
 *
 *  Reanalyse the text to find the prefixed character.  This is called when
 *  wordbreaking or ellipsification has shortened the string such that the
 *  previously noted prefixed character is no longer visible.
 *
 * Parameters
 *   str        [in] The original string segment (including all characters)
 *   ns         [in] The number of characters in str (including prefixes)
 *   pe         [in] The ellipsification data
 *
 * Return Values
 *   The prefix offset within the new string segment (the one that contains the
 *   ellipses and does not contain the prefix characters) (-1 if none)
 */

static int TEXT_Reprefix (const WCHAR *str, unsigned int ns,
                          const ellipsis_data *pe)
{
    int result = -1;
    unsigned int i = 0;
    unsigned int n = pe->before + pe->under + pe->after;
    assert (n <= ns);
    while (i < n)
    {
        if (i == (unsigned int) pe->before)
        {
            /* Reached the path ellipsis; jump over it */
            if (ns < (unsigned int) pe->under) break;
            str += pe->under;
            ns -= pe->under;
            i += pe->under;
            if (!pe->after) break; /* Nothing after the path ellipsis */
        }
        if (!ns) break;
        ns--;
        if (*str++ == PREFIX)
        {
            if (!ns) break;
            if (*str != PREFIX)
                result = (i < (unsigned int) pe->before || pe->under == 0) ? i : i - pe->under + pe->len;
                /* pe->len may be non-zero while pe_under is zero */
            str++;
            ns--;
        }
        i++;
    }
    return result;
}

/*********************************************************************
 *  Returns true if and only if the remainder of the line is a single
 *  newline representation or nothing
 */

static int remainder_is_none_or_newline (int num_chars, const WCHAR *str)
{
    if (!num_chars) return TRUE;
    if (*str != LF && *str != CR) return FALSE;
    if (!--num_chars) return TRUE;
    if (*str == *(str+1)) return FALSE;
    str++;
    if (*str != CR && *str != LF) return FALSE;
    if (--num_chars) return FALSE;
    return TRUE;
}

/*********************************************************************
 *  Return next line of text from a string.
 *
 * hdc - handle to DC.
 * str - string to parse into lines.
 * count - length of str.
 * dest - destination in which to return line.
 * len - dest buffer size in chars on input, copied length into dest on output.
 * width - maximum width of line in pixels.
 * format - format type passed to DrawText.
 * retsize - returned size of the line in pixels.
 * last_line - TRUE if is the last line that will be processed
 * p_retstr - If DT_MODIFYSTRING this points to a cursor in the buffer in which
 *            the return string is built.
 * tabwidth - The width of a tab in logical coordinates
 * pprefix_offset - Here is where we return the offset within dest of the first
 *                  prefixed (underlined) character.  -1 is returned if there
 *                  are none.  Note that there may be more; the calling code
 *                  will need to use TEXT_Reprefix to find any later ones.
 * pellip - Here is where we return the information about any ellipsification
 *          that was carried out.  Note that if tabs are being expanded then
 *          this data will correspond to the last text segment actually
 *          returned in dest; by definition there would not have been any
 *          ellipsification in earlier text segments of the line.
 *
 * Returns pointer to next char in str after end of the line
 * or NULL if end of str reached.
 */
static const WCHAR *TEXT_NextLineW( HDC hdc, const WCHAR *str, int *count,
                                 WCHAR *dest, int *len, int width, DWORD format,
                                 SIZE *retsize, int last_line, WCHAR **p_retstr,
                                 int tabwidth, int *pprefix_offset,
                                 ellipsis_data *pellip)
{
    int i = 0, j = 0;
    int plen = 0;
    SIZE size = {0, 0};
    int maxl = *len;
    int seg_i, seg_count, seg_j;
    int max_seg_width;
    int num_fit;
    int word_broken;
    int line_fits;
    unsigned int j_in_seg;
    int ellipsified;
    *pprefix_offset = -1;

    /* For each text segment in the line */

    retsize->cy = 0;
    while (*count)
    {

        /* Skip any leading tabs */

        if (str[i] == TAB && (format & DT_EXPANDTABS))
        {
            plen = ((plen/tabwidth)+1)*tabwidth;
            (*count)--; if (j < maxl) dest[j++] = str[i++]; else i++;
            while (*count && str[i] == TAB)
            {
                plen += tabwidth;
                (*count)--; if (j < maxl) dest[j++] = str[i++]; else i++;
            }
        }


        /* Now copy as far as the next tab or cr/lf or eos */

        seg_i = i;
        seg_count = *count;
        seg_j = j;

        while (*count &&
               (str[i] != TAB || !(format & DT_EXPANDTABS)) &&
               ((str[i] != CR && str[i] != LF) || (format & DT_SINGLELINE)))
        {
	    if (str[i] == PREFIX && !(format & DT_NOPREFIX) && *count > 1)
            {
                (*count)--, i++; /* Throw away the prefix itself */
                if (str[i] == PREFIX)
                {
                    /* Swallow it before we see it again */
		    (*count)--; if (j < maxl) dest[j++] = str[i++]; else i++;
                }
		else if (*pprefix_offset == -1 || *pprefix_offset >= seg_j)
                {
                    *pprefix_offset = j;
                }
                /* else the previous prefix was in an earlier segment of the
                 * line; we will leave it to the drawing code to catch this
                 * one.
                 */
	    }
            else
            {
                (*count)--; if (j < maxl) dest[j++] = str[i++]; else i++;
            }
        }


        /* Measure the whole text segment and possibly WordBreak and
         * ellipsify it
         */

        j_in_seg = j - seg_j;
        max_seg_width = width - plen;
        GetTextExtentExPointW (hdc, dest + seg_j, j_in_seg, max_seg_width, &num_fit, NULL, &size);

        /* The Microsoft handling of various combinations of formats is weird.
         * The following may very easily be incorrect if several formats are
         * combined, and may differ between versions (to say nothing of the
         * several bugs in the Microsoft versions).
         */
        word_broken = 0;
        line_fits = (num_fit >= j_in_seg);
        if (!line_fits && (format & DT_WORDBREAK))
        {
            const WCHAR *s;
            unsigned int chars_used;
            TEXT_WordBreak (hdc, dest+seg_j, maxl-seg_j, &j_in_seg,
                            max_seg_width, format, num_fit, &chars_used, &size);
            line_fits = (size.cx <= max_seg_width);
            /* and correct the counts */
            TEXT_SkipChars (count, &s, seg_count, str+seg_i, i-seg_i,
                            chars_used, !(format & DT_NOPREFIX));
            i = s - str;
            word_broken = 1;
        }
        pellip->before = j_in_seg;
        pellip->under = 0;
        pellip->after = 0;
        pellip->len = 0;
        ellipsified = 0;
        if (!line_fits && (format & DT_PATH_ELLIPSIS))
        {
            TEXT_PathEllipsify (hdc, dest + seg_j, maxl-seg_j, &j_in_seg,
                                max_seg_width, &size, *p_retstr, pellip);
            line_fits = (size.cx <= max_seg_width);
            ellipsified = 1;
        }
        /* NB we may end up ellipsifying a word-broken or path_ellipsified
         * string */
        if ((!line_fits && (format & DT_WORD_ELLIPSIS)) ||
            ((format & DT_END_ELLIPSIS) &&
              ((last_line && *count) ||
               (remainder_is_none_or_newline (*count, &str[i]) && !line_fits))))
        {
            int before, len_ellipsis;
            TEXT_Ellipsify (hdc, dest + seg_j, maxl-seg_j, &j_in_seg,
                            max_seg_width, &size, *p_retstr, &before, &len_ellipsis);
            if (before > pellip->before)
            {
                /* We must have done a path ellipsis too */
                pellip->after = before - pellip->before - pellip->len;
                /* Leave the len as the length of the first ellipsis */
            }
            else
            {
                /* If we are here after a path ellipsification it must be
                 * because even the ellipsis itself didn't fit.
                 */
                assert (pellip->under == 0 && pellip->after == 0);
                pellip->before = before;
                pellip->len = len_ellipsis;
                /* pellip->after remains as zero as does
                 * pellip->under
                 */
            }
            line_fits = (size.cx <= max_seg_width);
            ellipsified = 1;
        }
        /* As an optimisation if we have ellipsified and we are expanding
         * tabs and we haven't reached the end of the line we can skip to it
         * now rather than going around the loop again.
         */
        if ((format & DT_EXPANDTABS) && ellipsified)
        {
            if (format & DT_SINGLELINE)
                *count = 0;
            else
            {
                while ((*count) && str[i] != CR && str[i] != LF)
                {
                    (*count)--, i++;
                }
            }
        }

        j = seg_j + j_in_seg;
        if (*pprefix_offset >= seg_j + pellip->before)
        {
            *pprefix_offset = TEXT_Reprefix (str + seg_i, i - seg_i, pellip);
            if (*pprefix_offset != -1)
                *pprefix_offset += seg_j;
        }

        plen += size.cx;
        if (size.cy > retsize->cy)
            retsize->cy = size.cy;

        if (word_broken)
            break;
        else if (!*count)
            break;
        else if (str[i] == CR || str[i] == LF)
        {
            (*count)--, i++;
            if (*count && (str[i] == CR || str[i] == LF) && str[i] != str[i-1])
            {
                (*count)--, i++;
            }
            break;
        }
        /* else it was a Tab and we go around again */
    }

    retsize->cx = plen;
    *len = j;
    if (*count)
        return (&str[i]);
    else
        return NULL;
}


/***********************************************************************
 *                      TEXT_DrawUnderscore
 *
 *  Draw the underline under the prefixed character
 *
 * Parameters
 *   hdc        [in] The handle of the DC for drawing
 *   x          [in] The x location of the line segment (logical coordinates)
 *   y          [in] The y location of where the underscore should appear
 *                   (logical coordinates)
 *   str        [in] The text of the line segment
 *   offset     [in] The offset of the underscored character within str
 *   rect       [in] Clipping rectangle (if not NULL)
 */
/* Synced with wine 1.1.32 */
static void TEXT_DrawUnderscore (HDC hdc, int x, int y, const WCHAR *str, int offset, const RECT *rect)
{
    int prefix_x;
    int prefix_end;
    SIZE size;
    HPEN hpen;
    HPEN oldPen;

    GetTextExtentPointW (hdc, str, offset, &size);
    prefix_x = x + size.cx;
    GetTextExtentPointW (hdc, str, offset+1, &size);
    prefix_end = x + size.cx - 1;
    /* The above method may eventually be slightly wrong due to kerning etc. */

    /* Check for clipping */
    if (rect)
    {
        if (prefix_x > rect->right || prefix_end < rect->left ||
            y < rect->top || y > rect->bottom)
            return; /* Completely outside */
        /* Partially outside */
        if (prefix_x   < rect->left ) prefix_x   = rect->left;
        if (prefix_end > rect->right) prefix_end = rect->right;
    }

    hpen = CreatePen (PS_SOLID, 1, GetTextColor (hdc));
    oldPen = SelectObject (hdc, hpen);
    MoveToEx (hdc, prefix_x, y, NULL);
    LineTo (hdc, prefix_end, y);
    SelectObject (hdc, oldPen);
    DeleteObject (hpen);
}

/***********************************************************************
 *           DrawTextExW    (USER32.@)
 *
 * The documentation on the extra space required for DT_MODIFYSTRING at MSDN
 * is not quite complete, especially with regard to \0.  We will assume that
 * the returned string could have a length of up to i_count+3 and also have
 * a trailing \0 (which would be 4 more than a not-null-terminated string but
 * 3 more than a null-terminated string).  If this is not so then increase
 * the allowance in DrawTextExA.
 */
#define MAX_BUFFER 1024
/* 
 * @implemented
 *
 * Synced with wine 1.1.32
 */
INT WINAPI DrawTextExW( HDC hdc, LPWSTR str, INT i_count,
                        LPRECT rect, UINT flags, LPDRAWTEXTPARAMS dtp )
{
    SIZE size;
    const WCHAR *strPtr;
    WCHAR *retstr, *p_retstr;
    size_t size_retstr;
    WCHAR line[MAX_BUFFER];
    int len, lh, count=i_count;
    TEXTMETRICW tm;
    int lmargin = 0, rmargin = 0;
    int x = rect->left, y = rect->top;
    int width = rect->right - rect->left;
    int max_width = 0;
    int last_line;
    int tabwidth /* to keep gcc happy */ = 0;
    int prefix_offset;
    ellipsis_data ellip;
    int invert_y=0;

    TRACE("%s, %d, [%s] %08x\n", debugstr_wn (str, count), count,
        wine_dbgstr_rect(rect), flags);

   if (dtp) TRACE("Params: iTabLength=%d, iLeftMargin=%d, iRightMargin=%d\n",
          dtp->iTabLength, dtp->iLeftMargin, dtp->iRightMargin);

    if (!str) return 0;

    strPtr = str;

    if (flags & DT_SINGLELINE)
        flags &= ~DT_WORDBREAK;

    GetTextMetricsW(hdc, &tm);
    if (flags & DT_EXTERNALLEADING)
	lh = tm.tmHeight + tm.tmExternalLeading;
    else
	lh = tm.tmHeight;

    if (str[0] && count == 0)
        return lh;

    if (dtp && dtp->cbSize != sizeof(DRAWTEXTPARAMS))
        return 0;

    if (count == -1)
    {
        count = strlenW(str);
        if (count == 0)
        {
            if( flags & DT_CALCRECT)
            {
                rect->right = rect->left;
                if( flags & DT_SINGLELINE)
                    rect->bottom = rect->top + lh;
                else
                    rect->bottom = rect->top;
            }
            return lh;
        }
    }

    if (GetGraphicsMode(hdc) == GM_COMPATIBLE)
    {
        SIZE window_ext, viewport_ext;
        GetWindowExtEx(hdc, &window_ext);
        GetViewportExtEx(hdc, &viewport_ext);
        if ((window_ext.cy > 0) != (viewport_ext.cy > 0))
            invert_y = 1;
    }

    if (dtp)
    {
        lmargin = dtp->iLeftMargin;
        rmargin = dtp->iRightMargin;
        if (!(flags & (DT_CENTER | DT_RIGHT)))
            x += lmargin;
        dtp->uiLengthDrawn = 0;     /* This param RECEIVES number of chars processed */
    }

    if (flags & DT_EXPANDTABS)
    {
        int tabstop = ((flags & DT_TABSTOP) && dtp) ? dtp->iTabLength : 8;
	tabwidth = tm.tmAveCharWidth * tabstop;
    }

    if (flags & DT_CALCRECT) flags |= DT_NOCLIP;

    if (flags & DT_MODIFYSTRING)
    {
        size_retstr = (count + 4) * sizeof (WCHAR);
        retstr = HeapAlloc(GetProcessHeap(), 0, size_retstr);
        if (!retstr) return 0;
        memcpy (retstr, str, size_retstr);
    }
    else
    {
        size_retstr = 0;
        retstr = NULL;
    }
    p_retstr = retstr;

    do
    {
	len = sizeof(line)/sizeof(line[0]);
	if (invert_y)
            last_line = !(flags & DT_NOCLIP) && y - ((flags & DT_EDITCONTROL) ? 2*lh-1 : lh) < rect->bottom;
	else
            last_line = !(flags & DT_NOCLIP) && y + ((flags & DT_EDITCONTROL) ? 2*lh-1 : lh) > rect->bottom;
	strPtr = TEXT_NextLineW(hdc, strPtr, &count, line, &len, width, flags, &size, last_line, &p_retstr, tabwidth, &prefix_offset, &ellip);

	if (flags & DT_CENTER) x = (rect->left + rect->right -
				    size.cx) / 2;
	else if (flags & DT_RIGHT) x = rect->right - size.cx;

	if (flags & DT_SINGLELINE)
	{
	    if (flags & DT_VCENTER) y = rect->top +
	    	(rect->bottom - rect->top) / 2 - size.cy / 2;
	    else if (flags & DT_BOTTOM) y = rect->bottom - size.cy;
        }

	if (!(flags & DT_CALCRECT))
	{
            const WCHAR *str = line;
            int xseg = x;
            while (len)
            {
                int len_seg;
                SIZE size;
                if ((flags & DT_EXPANDTABS))
                {
                    const WCHAR *p;
                    p = str; while (p < str+len && *p != TAB) p++;
                    len_seg = p - str;
                    if (len_seg != len && !GetTextExtentPointW(hdc, str, len_seg, &size))
                        return 0;
                }
                else
                    len_seg = len;

                if (!ExtTextOutW( hdc, xseg, y,
                                 ((flags & DT_NOCLIP) ? 0 : ETO_CLIPPED) |
                                 ((flags & DT_RTLREADING) ? ETO_RTLREADING : 0),
                                 rect, str, len_seg, NULL ))  return 0;
                if (prefix_offset != -1 && prefix_offset < len_seg)
                {
                    TEXT_DrawUnderscore (hdc, xseg, y + tm.tmAscent + 1, str, prefix_offset, (flags & DT_NOCLIP) ? NULL : rect);
                }
                len -= len_seg;
                str += len_seg;
                if (len)
                {
                    assert ((flags & DT_EXPANDTABS) && *str == TAB);
                    len--; str++;
                    xseg += ((size.cx/tabwidth)+1)*tabwidth;
                    if (prefix_offset != -1)
                    {
                        if (prefix_offset < len_seg)
                        {
                            /* We have just drawn an underscore; we ought to
                             * figure out where the next one is.  I am going
                             * to leave it for now until I have a better model
                             * for the line, which will make reprefixing easier.
                             * This is where ellip would be used.
                             */
                            prefix_offset = -1;
                        }
                        else
                            prefix_offset -= len_seg;
                    }
                }
            }
    }
    else if (size.cx > max_width)
        max_width = size.cx;

        if (invert_y)
	    y -= lh;
        else
	    y += lh;
        if (dtp)
            dtp->uiLengthDrawn += len;
    }
    while (strPtr && !last_line);

    if (flags & DT_CALCRECT)
    {
	rect->right = rect->left + max_width;
	rect->bottom = y;
        if (dtp)
            rect->right += lmargin + rmargin;
    }
    if (retstr)
    {
        memcpy (str, retstr, size_retstr);
        HeapFree (GetProcessHeap(), 0, retstr);
    }
    return y - rect->top;
}

DWORD
WINAPI
GdiGetCodePage(HDC hdc);

/***********************************************************************
 *           DrawTextExA    (USER32.@)
 *
 * If DT_MODIFYSTRING is specified then there must be room for up to
 * 4 extra characters.  We take great care about just how much modified
 * string we return.
 *
 * @implemented
 *
 * Synced with wine 1.1.32
 */
INT WINAPI DrawTextExA( HDC hdc, LPSTR str, INT count,
                        LPRECT rect, UINT flags, LPDRAWTEXTPARAMS dtp )
{
   WCHAR *wstr;
   WCHAR *p;
   INT ret = 0;
   int i;
   DWORD wcount;
   DWORD wmax;
   DWORD amax;
   UINT cp;

   if (!count) return 0;
   if (!str && count > 0) return 0;
   if( !str || ((count == -1) && !(count = strlen(str))))
   {
        int lh;
        TEXTMETRICA tm;

        if (dtp && dtp->cbSize != sizeof(DRAWTEXTPARAMS))
            return 0;

        GetTextMetricsA(hdc, &tm);
        if (flags & DT_EXTERNALLEADING)
            lh = tm.tmHeight + tm.tmExternalLeading;
        else
            lh = tm.tmHeight;

        if( flags & DT_CALCRECT)
        {
            rect->right = rect->left;
            if( flags & DT_SINGLELINE)
                rect->bottom = rect->top + lh;
            else
                rect->bottom = rect->top;
        }
        return lh;
   }
   cp = GdiGetCodePage( hdc );
   wcount = MultiByteToWideChar( cp, 0, str, count, NULL, 0 );
   wmax = wcount;
   amax = count;
   if (flags & DT_MODIFYSTRING)
   {
        wmax += 4;
        amax += 4;
   }
   wstr = HeapAlloc(GetProcessHeap(), 0, wmax * sizeof(WCHAR));
   if (wstr)
   {
       MultiByteToWideChar( cp, 0, str, count, wstr, wcount );
       if (flags & DT_MODIFYSTRING)
           for (i=4, p=wstr+wcount; i--; p++) *p=0xFFFE;
           /* Initialise the extra characters so that we can see which ones
            * change.  U+FFFE is guaranteed to be not a unicode character and
            * so will not be generated by DrawTextEx itself.
            */
       ret = DrawTextExW( hdc, wstr, wcount, rect, flags, dtp );
       if (flags & DT_MODIFYSTRING)
       {
            /* Unfortunately the returned string may contain multiple \0s
             * and so we need to measure it ourselves.
             */
            for (i=4, p=wstr+wcount; i-- && *p != 0xFFFE; p++) wcount++;
            WideCharToMultiByte( cp, 0, wstr, wcount, str, amax, NULL, NULL );
       }
       HeapFree(GetProcessHeap(), 0, wstr);
   }
   return ret;
}

/***********************************************************************
 *           DrawTextW    (USER32.@)
 *
 * @implemented
 */
INT WINAPI DrawTextW( HDC hdc, LPCWSTR str, INT count, LPRECT rect, UINT flags )
{
    DRAWTEXTPARAMS dtp;

    memset (&dtp, 0, sizeof(dtp));
    dtp.cbSize = sizeof(dtp);
    if (flags & DT_TABSTOP)
    {
        dtp.iTabLength = (flags >> 8) & 0xff;
        flags &= 0xffff00ff;
    }
    return DrawTextExW(hdc, (LPWSTR)str, count, rect, flags, &dtp);
}

/***********************************************************************
 *           DrawTextA    (USER32.@)
 *
 * @implemented
 */
INT WINAPI DrawTextA( HDC hdc, LPCSTR str, INT count, LPRECT rect, UINT flags )
{
    DRAWTEXTPARAMS dtp;

    memset (&dtp, 0, sizeof(dtp));
    dtp.cbSize = sizeof(dtp);
    if (flags & DT_TABSTOP)
    {
        dtp.iTabLength = (flags >> 8) & 0xff;
        flags &= 0xffff00ff;
    }
    return DrawTextExA( hdc, (LPSTR)str, count, rect, flags, &dtp );
}
