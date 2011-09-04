/*
 *  Odyssey kernel
 *  Copyright (C) 2003, 2006 ReactOS Team; (C) 2011 NasuTek Enterprises
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey hive maker
 * FILE:            tools/mkhive/mkhive.c
 * PURPOSE:         Hive maker
 * PROGRAMMER:      Eric Kohl
 *                  Herv� Poussineau
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "mkhive.h"

#ifdef _MSC_VER
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif//_MSC_VER

#ifndef _WIN32
#ifndef PATH_MAX
#define PATH_MAX 260
#endif
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_STRING "/"
#else
#define DIR_SEPARATOR_CHAR '\\'
#define DIR_SEPARATOR_STRING "\\"
#endif


void usage (void)
{
	printf ("Usage: mkhive <srcdir> <dstdir> <arch> [addinf]\n\n");
	printf ("  srcdir  - inf files are read from this directory\n");
	printf ("  dstdir  - binary hive files are created in this directory\n");
	printf ("  arch    - architecture\n");
	printf ("  addinf  - additional inf files with full path\n");
}

void convert_path(char *dst, char *src)
{
	int i;

	i = 0;
	while (src[i] != 0)
	{
#ifdef _WIN32
		if (src[i] == '/')
		{
			dst[i] = '\\';
		}
#else
		if (src[i] == '\\')
		{
			dst[i] = '/';
		}
#endif
		else
		{
			dst[i] = src[i];
		}

		i++;
	}
	dst[i] = 0;
}

int main (int argc, char *argv[])
{
	char FileName[PATH_MAX];
	int Param;

	if (argc < 4)
	{
		usage ();
		return 1;
	}

	printf ("Binary hive maker: %s\n", argv[3]);

	RegInitializeRegistry ();

	convert_path (FileName, argv[1]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "hivesys_");
    strcat (FileName, argv[3]);
    strcat (FileName, ".inf");
	ImportRegistryFile (FileName);

	convert_path (FileName, argv[1]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "hivecls_");
    strcat (FileName, argv[3]);
    strcat (FileName, ".inf");
	ImportRegistryFile (FileName);

	convert_path (FileName, argv[1]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "hivesft_");
    strcat (FileName, argv[3]);
    strcat (FileName, ".inf");
	ImportRegistryFile (FileName);

	convert_path (FileName, argv[1]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "hivedef_");
    strcat (FileName, argv[3]);
    strcat (FileName, ".inf");
	ImportRegistryFile (FileName);

	for (Param = 4; Param < argc; Param++)
	{
		convert_path (FileName, argv[Param]);
		ImportRegistryFile (FileName);
	}

	convert_path (FileName, argv[2]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "default");
	if (!ExportBinaryHive (FileName, &DefaultHive))
	{
		return 1;
	}

	convert_path (FileName, argv[2]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "sam");
	if (!ExportBinaryHive (FileName, &SamHive))
	{
		return 1;
	}

	convert_path (FileName, argv[2]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "security");
	if (!ExportBinaryHive (FileName, &SecurityHive))
	{
		return 1;
	}

	convert_path (FileName, argv[2]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "software");
	if (!ExportBinaryHive (FileName, &SoftwareHive))
	{
		return 1;
	}

	convert_path (FileName, argv[2]);
	strcat (FileName, DIR_SEPARATOR_STRING);
	strcat (FileName, "system");
	if (!ExportBinaryHive (FileName, &SystemHive))
	{
		return 1;
	}

	RegShutdownRegistry ();

	printf ("  Done.\n");

	return 0;
}

/* EOF */
