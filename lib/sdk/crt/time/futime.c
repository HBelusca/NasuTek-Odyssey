/*
 * COPYRIGHT:   LGPL, See LGPL.txt in the top level directory
 * PROJECT:     Odyssey CRT library
 * FILE:        lib/sdk/crt/time/futime.c
 * PURPOSE:     Implementation of _futime
 * PROGRAMERS:  Wine team
 */

/*
 * msvcrt.dll file functions
 *
 * Copyright 1996,1998 Marcus Meissner
 * Copyright 1996 Jukka Iivonen
 * Copyright 1997,2000 Uwe Bonnes
 * Copyright 2000 Jon Griffiths
 * Copyright 2004 Eric Pouech
 * Copyright 2004 Juan Lang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * TODO
 * Use the file flag hints O_SEQUENTIAL, O_RANDOM, O_SHORT_LIVED
 */

#include <precomp.h>
#define RC_INVOKED 1 // to prevent inline functions
#include <time.h>
#include <sys/utime.h>
#include "bitsfixup.h"

HANDLE fdtoh(int fd);

/******************************************************************************
 * \name _futime
 * \brief Set a files modification time.
 * \param [out] ptimeb Pointer to a structure of type struct _timeb that
 *        recieves the current time.
 * \sa http://msdn.microsoft.com/en-us/library/95e68951.aspx
 */
int
_futime(int fd, struct _utimbuf *filetime)
{
    HANDLE handle;
    FILETIME at, wt;

    handle = fdtoh(fd);
    if (handle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    if (!filetime)
    {
        time_t currTime;
        _time(&currTime);
        RtlSecondsSince1970ToTime(currTime, (LARGE_INTEGER *)&at);
        wt = at;
    }
    else
    {
        RtlSecondsSince1970ToTime(filetime->actime, (LARGE_INTEGER *)&at);
        if (filetime->actime == filetime->modtime)
        {
            wt = at;
        }
        else
        {
            RtlSecondsSince1970ToTime(filetime->modtime, (LARGE_INTEGER *)&wt);
        }
    }

    if (!SetFileTime(handle, NULL, &at, &wt))
    {
        __set_errno(GetLastError());
        return -1 ;
    }

    return 0;
}
