/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            lib/rossym/delete.c
 * PURPOSE:         Free rossym info
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@odyssey.com)
 */

#include <precomp.h>

VOID
RosSymDelete(PROSSYM_INFO RosSymInfo)
{
	dwarfclose(RosSymInfo);
}

/* EOF */
