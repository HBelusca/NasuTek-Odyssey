/*
 * COPYRIGHT:       GNU GPL, see COPYING in the top level directory
 * PROJECT:         Odyssey crt library
 * FILE:            lib/sdk/crt/printf/fprintf.c
 * PURPOSE:         Implementation of fprintf
 * PROGRAMMER:      Timo Kreuzer
 */

#include <stdio.h>
#include <stdarg.h>

int
_cdecl
fprintf(FILE *stream, const char *format, ...)
{
    va_list argptr;
    int result;

    va_start(argptr, format);
    result = vfprintf(stream, format, argptr);
    va_end(argptr);
    return result;
}

