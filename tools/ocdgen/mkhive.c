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
 *                  Hervé Poussineau
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
	printf ("Usage: ocdgen <srcfile> <dstfile>\n\n");
	printf ("  srcfile - inf file containing Odyssey boot configuration\n");
	printf ("  dstfile - generated binary hive file containing Odyssey boot configuration\n");
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

	if (argc < 2)
	{
		usage ();
		return 1;
	}

	printf ("Binary hive maker: %s\n", argv[3]);

	RegInitializeRegistry ();

	convert_path (FileName, argv[1]);
	ImportRegistryFile (FileName);

	convert_path (FileName, argv[2]);
	if (!ExportBinaryHive (FileName, &BootConfigurationHive))
	{
		return 1;
	}

	RegShutdownRegistry ();

	printf ("  Done.\n");

	return 0;
}

/* EOF */
