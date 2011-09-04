/*
 * COPYRIGHT:   LGPL, See LGPL.txt in the top level directory
 * PROJECT:     Odyssey CRT library
 * FILE:        lib/sdk/crt/time/ctime.c
 * PURPOSE:     Implementation of ctime, _ctime_s
 * PROGRAMERS:  Timo Kreuzer
 */
#define MINGW_HAS_SECURE_API 1

#define RC_INVOKED 1 // to prevent inline functions
#include <tchar.h>
#include <time.h>
#include "bitsfixup.h"

#define EINVAL -1

/* Doesn't really exist, but we need it here */
_CRTIMP errno_t __cdecl localtime_s(struct tm *_Tm,const time_t *_Time);

/******************************************************************************
 * \name _tctime_s
 * \brief Converts a time_t value into a string.
 * \param buffer Buffer that receives the string (26 characters).
 * \param numberOfElements Size of the buffer in characters.
 * \param time Pointer to the UTC time.
 */
errno_t
_tctime_s(_TCHAR *buffer, size_t numberOfElements, const time_t *time)
{
    struct tm _tm;

    if (localtime_s(&_tm, time) == EINVAL)
    {
        return EINVAL;
    }
    return _tasctime_s(buffer, numberOfElements, &_tm);
}

/******************************************************************************
 * \name _tctime
 * \brief Converts a time_t value into a string and returns a pointer to it.
 * \param time Pointer to the UTC time.
 * \remarks The string is stored in thread local buffer, shared between
 *          ctime, gmtime and localtime (both 32 and 64 bit versions).
 */
_TCHAR *
_tctime(const time_t *ptime)
{
    struct tm *ptm = localtime(ptime);
    if (!ptm)
    {
        return 0;
    }
    return _tasctime(ptm);
}

