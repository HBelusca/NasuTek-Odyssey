/*
 *  Odyssey kernel
 *  Copyright (C) 2004 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* $Id: genlist.h 45685 2010-02-26 11:43:19Z gedmurphy $
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey text-mode setup
 * FILE:            subsys/system/usetup/genlist.h
 * PURPOSE:         Generic list functions
 * PROGRAMMER:      Eric Kohl
 */

#pragma once

struct _GENERIC_LIST_ENTRY;
typedef struct _GENERIC_LIST_ENTRY *PGENERIC_LIST_ENTRY;
struct _GENERIC_LIST;
typedef struct _GENERIC_LIST *PGENERIC_LIST;

PGENERIC_LIST
CreateGenericList(VOID);

VOID
DestroyGenericList(PGENERIC_LIST List,
                   BOOLEAN FreeUserData);

BOOLEAN
AppendGenericListEntry(PGENERIC_LIST List,
                       PCHAR Text,
                       PVOID UserData,
                       BOOLEAN Current);

VOID
DrawGenericList(PGENERIC_LIST List,
                SHORT Left,
                SHORT Top,
                SHORT Right,
                SHORT Bottom);

VOID
DrawScrollBarGenericLis(PGENERIC_LIST List);

VOID
ScrollDownGenericList(PGENERIC_LIST List);

VOID
ScrollUpGenericList(PGENERIC_LIST List);

VOID
ScrollPageDownGenericList(PGENERIC_LIST List);

VOID
ScrollPageUpGenericList(PGENERIC_LIST List);

VOID
ScrollToPositionGenericList (PGENERIC_LIST List, ULONG uIndex);

VOID
SetCurrentListEntry(PGENERIC_LIST List, PGENERIC_LIST_ENTRY Entry);

PGENERIC_LIST_ENTRY
GetCurrentListEntry(PGENERIC_LIST List);

PGENERIC_LIST_ENTRY
GetFirstListEntry(PGENERIC_LIST List);

PGENERIC_LIST_ENTRY
GetNextListEntry(PGENERIC_LIST_ENTRY Entry);

PVOID
GetListEntryUserData(PGENERIC_LIST_ENTRY List);

LPCSTR
GetListEntryText(PGENERIC_LIST_ENTRY List);

VOID
SaveGenericListState(PGENERIC_LIST List);

VOID
RestoreGenericListState(PGENERIC_LIST List);

VOID
GenericListKeyPress (PGENERIC_LIST List, CHAR AsciChar);

/* EOF */
