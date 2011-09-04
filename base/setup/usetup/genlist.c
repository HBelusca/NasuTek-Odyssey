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
/* COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey text-mode setup
 * FILE:            subsys/system/usetup/genlist.c
 * PURPOSE:         Generic list functions
 * PROGRAMMER:      Eric Kohl
 *                  Christoph von Wittich <christoph at odyssey.org>
 */

/* INCLUDES *****************************************************************/

#include "usetup.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

typedef struct _GENERIC_LIST_ENTRY
{
    LIST_ENTRY Entry;
    PGENERIC_LIST List;
    PVOID UserData;
    CHAR Text[1];
} GENERIC_LIST_ENTRY;


typedef struct _GENERIC_LIST
{
    LIST_ENTRY ListHead;

    PLIST_ENTRY FirstShown;
    PLIST_ENTRY LastShown;
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
    BOOL Redraw;

    PGENERIC_LIST_ENTRY CurrentEntry;
    PGENERIC_LIST_ENTRY BackupEntry;
} GENERIC_LIST;

PGENERIC_LIST
CreateGenericList(VOID)
{
    PGENERIC_LIST List;

    List = (PGENERIC_LIST)RtlAllocateHeap(ProcessHeap,
                                          0,
                                          sizeof(GENERIC_LIST));
    if (List == NULL)
        return NULL;

    InitializeListHead(&List->ListHead);

    List->Left = 0;
    List->Top = 0;
    List->Right = 0;
    List->Bottom = 0;
    List->Redraw = TRUE;

    List->CurrentEntry = NULL;

    return List;
}


VOID
DestroyGenericList(PGENERIC_LIST List,
                   BOOLEAN FreeUserData)
{
    PGENERIC_LIST_ENTRY ListEntry;
    PLIST_ENTRY Entry;

    /* Release list entries */
    while (!IsListEmpty (&List->ListHead))
    {
        Entry = RemoveHeadList (&List->ListHead);
        ListEntry = CONTAINING_RECORD (Entry, GENERIC_LIST_ENTRY, Entry);

        /* Release user data */
        if (FreeUserData && ListEntry->UserData != NULL)
            RtlFreeHeap (ProcessHeap, 0, ListEntry->UserData);

        /* Release list entry */
        RtlFreeHeap (ProcessHeap, 0, ListEntry);
    }

    /* Release list head */
    RtlFreeHeap (ProcessHeap, 0, List);
}


BOOLEAN
AppendGenericListEntry(PGENERIC_LIST List,
                     PCHAR Text,
                     PVOID UserData,
                     BOOLEAN Current)
{
    PGENERIC_LIST_ENTRY Entry;

    Entry = (PGENERIC_LIST_ENTRY)RtlAllocateHeap(ProcessHeap,
                                                 0,
                                                 sizeof(GENERIC_LIST_ENTRY) + strlen(Text));
    if (Entry == NULL)
        return FALSE;

    strcpy (Entry->Text, Text);
    Entry->List = List;
    Entry->UserData = UserData;

    InsertTailList(&List->ListHead,
                   &Entry->Entry);

    if (Current || List->CurrentEntry == NULL)
    {
        List->CurrentEntry = Entry;
    }

    return TRUE;
}


static VOID
DrawListFrame(PGENERIC_LIST GenericList)
{
    COORD coPos;
    DWORD Written;
    SHORT i;

    /* Draw upper left corner */
    coPos.X = GenericList->Left;
    coPos.Y = GenericList->Top;
    FillConsoleOutputCharacterA (StdOutput,
                                 0xDA, // '+',
                                 1,
                                 coPos,
                                 &Written);

    /* Draw upper edge */
    coPos.X = GenericList->Left + 1;
    coPos.Y = GenericList->Top;
    FillConsoleOutputCharacterA (StdOutput,
                                 0xC4, // '-',
                                 GenericList->Right - GenericList->Left - 1,
                                 coPos,
                                 &Written);

    /* Draw upper right corner */
    coPos.X = GenericList->Right;
    coPos.Y = GenericList->Top;
    FillConsoleOutputCharacterA (StdOutput,
                                 0xBF, // '+',
                                 1,
                                 coPos,
                                 &Written);

    /* Draw left and right edge */
    for (i = GenericList->Top + 1; i < GenericList->Bottom; i++)
    {
        coPos.X = GenericList->Left;
        coPos.Y = i;
        FillConsoleOutputCharacterA (StdOutput,
                                     0xB3, // '|',
                                     1,
                                     coPos,
                                     &Written);

        coPos.X = GenericList->Right;
        FillConsoleOutputCharacterA (StdOutput,
                                     0xB3, //'|',
                                     1,
                                     coPos,
                                     &Written);
    }

    /* Draw lower left corner */
    coPos.X = GenericList->Left;
    coPos.Y = GenericList->Bottom;
    FillConsoleOutputCharacterA (StdOutput,
                                 0xC0, // '+',
                                 1,
                                 coPos,
                                 &Written);

    /* Draw lower edge */
    coPos.X = GenericList->Left + 1;
    coPos.Y = GenericList->Bottom;
    FillConsoleOutputCharacterA (StdOutput,
                                 0xC4, // '-',
                                 GenericList->Right - GenericList->Left - 1,
                                 coPos,
                                 &Written);

    /* Draw lower right corner */
    coPos.X = GenericList->Right;
    coPos.Y = GenericList->Bottom;
    FillConsoleOutputCharacterA (StdOutput,
                                 0xD9, // '+',
                                 1,
                                 coPos,
                                 &Written);
}


static VOID
DrawListEntries(PGENERIC_LIST GenericList)
{
    PGENERIC_LIST_ENTRY ListEntry;
    PLIST_ENTRY Entry;
    COORD coPos;
    DWORD Written;
    USHORT Width;

    coPos.X = GenericList->Left + 1;
    coPos.Y = GenericList->Top + 1;
    Width = GenericList->Right - GenericList->Left - 1;

    Entry = GenericList->FirstShown;
    while (Entry != &GenericList->ListHead)
    {
        ListEntry = CONTAINING_RECORD (Entry, GENERIC_LIST_ENTRY, Entry);

        if (coPos.Y == GenericList->Bottom)
            break;
        GenericList->LastShown = Entry;

        FillConsoleOutputAttribute (StdOutput,
                                    (GenericList->CurrentEntry == ListEntry) ?
                                    FOREGROUND_BLUE | BACKGROUND_WHITE :
                                    FOREGROUND_WHITE | BACKGROUND_BLUE,
                                    Width,
                                    coPos,
                                    &Written);

        FillConsoleOutputCharacterA (StdOutput,
                                     ' ',
                                     Width,
                                     coPos,
                                     &Written);

        coPos.X++;
        WriteConsoleOutputCharacterA (StdOutput,
                                      ListEntry->Text,
                                      min (strlen(ListEntry->Text), (SIZE_T)Width - 2),
                                      coPos,
                                      &Written);
        coPos.X--;

        coPos.Y++;
        Entry = Entry->Flink;
    }

    while (coPos.Y < GenericList->Bottom)
    {
        FillConsoleOutputAttribute (StdOutput,
                                    FOREGROUND_WHITE | BACKGROUND_BLUE,
                                    Width,
                                    coPos,
                                    &Written);

        FillConsoleOutputCharacterA (StdOutput,
                                     ' ',
                                     Width,
                                     coPos,
                                     &Written);
        coPos.Y++;
    }
}

static VOID
DrawScrollBarGenericList(PGENERIC_LIST GenericList)
{
    COORD coPos;
    DWORD Written;

    coPos.X = GenericList->Right + 1;
    coPos.Y = GenericList->Top;

    if (GenericList->FirstShown != GenericList->ListHead.Flink)
    {
        FillConsoleOutputCharacterA (StdOutput,
                                     '\x18',
                                     1,
                                     coPos,
                                     &Written);
    }
    else
    {
        FillConsoleOutputCharacterA (StdOutput,
                                     ' ',
                                     1,
                                     coPos,
                                     &Written);
    }

    coPos.Y = GenericList->Bottom;
    if (GenericList->LastShown != GenericList->ListHead.Blink)
    {
        FillConsoleOutputCharacterA (StdOutput,
                                     '\x19',
                                     1,
                                     coPos,
                                     &Written);
    }
    else
    {
        FillConsoleOutputCharacterA (StdOutput,
                                     ' ',
                                     1,
                                     coPos,
                                     &Written);
    }
}

VOID
DrawGenericList(PGENERIC_LIST List,
                SHORT Left,
                SHORT Top,
                SHORT Right,
                SHORT Bottom)
{
    List->FirstShown = List->ListHead.Flink;
    List->Left = Left;
    List->Top = Top;
    List->Right = Right;
    List->Bottom = Bottom;

    DrawListFrame(List);

    if (IsListEmpty(&List->ListHead))
        return;

    DrawListEntries(List);
    DrawScrollBarGenericList(List);
}

VOID
ScrollPageDownGenericList (PGENERIC_LIST List)
{
    SHORT i;

    /* Suspend auto-redraw */
    List->Redraw = FALSE;

    for (i = List->Top + 1; i < List->Bottom - 1; i++)
    {
        ScrollDownGenericList (List);
    }

    /* Update user interface */
    DrawListEntries(List);
    DrawScrollBarGenericList(List);

    /* Re enable auto-redraw */
    List->Redraw = TRUE;
}

VOID
ScrollPageUpGenericList (PGENERIC_LIST List)
{
    SHORT i;

    /* Suspend auto-redraw */
    List->Redraw = FALSE;

    for (i = List->Bottom - 1; i > List->Top + 1; i--)
    {
         ScrollUpGenericList (List);
    }

    /* Update user interface */
    DrawListEntries(List);
    DrawScrollBarGenericList(List);

    /* Re enable auto-redraw */
    List->Redraw = TRUE;
}

VOID
ScrollDownGenericList (PGENERIC_LIST List)
{
    PLIST_ENTRY Entry;

    if (List->CurrentEntry == NULL)
        return;

    if (List->CurrentEntry->Entry.Flink != &List->ListHead)
    {
        Entry = List->CurrentEntry->Entry.Flink;
        if (List->LastShown == &List->CurrentEntry->Entry)
        {
            List->FirstShown = List->FirstShown->Flink;
            List->LastShown = List->LastShown->Flink;
        }
        List->CurrentEntry = CONTAINING_RECORD (Entry, GENERIC_LIST_ENTRY, Entry);

        if (List->Redraw)
        {
            DrawListEntries(List);
            DrawScrollBarGenericList(List);
        }
    }
}


VOID
ScrollToPositionGenericList (PGENERIC_LIST List, ULONG uIndex)
{
    PLIST_ENTRY Entry;
    ULONG uCount = 0;

    if (List->CurrentEntry == NULL)
        return;

    do
    {
        if (List->CurrentEntry->Entry.Flink != &List->ListHead)
        {
            Entry = List->CurrentEntry->Entry.Flink;
            if (List->LastShown == &List->CurrentEntry->Entry)
            {
                List->FirstShown = List->FirstShown->Flink;
                List->LastShown = List->LastShown->Flink;
            }
            List->CurrentEntry = CONTAINING_RECORD (Entry, GENERIC_LIST_ENTRY, Entry);
        }
        uCount++;
    }
    while (uIndex != uCount);

    if (List->Redraw)
    {
        DrawListEntries(List);
        DrawScrollBarGenericList(List);
    }
}


VOID
ScrollUpGenericList (PGENERIC_LIST List)
{
    PLIST_ENTRY Entry;

    if (List->CurrentEntry == NULL)
        return;

    if (List->CurrentEntry->Entry.Blink != &List->ListHead)
    {
        Entry = List->CurrentEntry->Entry.Blink;
        if (List->FirstShown == &List->CurrentEntry->Entry)
        {
            List->FirstShown = List->FirstShown->Blink;
            List->LastShown = List->LastShown->Blink;
        }
        List->CurrentEntry = CONTAINING_RECORD (Entry, GENERIC_LIST_ENTRY, Entry);

        if (List->Redraw)
        {
            DrawListEntries(List);
            DrawScrollBarGenericList(List);
        }
    }
}


VOID
SetCurrentListEntry(PGENERIC_LIST List, PGENERIC_LIST_ENTRY Entry)
{
    if (Entry->List != List)
        return;
    List->CurrentEntry = Entry;
}


PGENERIC_LIST_ENTRY
GetCurrentListEntry(PGENERIC_LIST List)
{
    return List->CurrentEntry;
}


PGENERIC_LIST_ENTRY
GetFirstListEntry(PGENERIC_LIST List)
{
    PLIST_ENTRY Entry = List->ListHead.Flink;

    if (Entry == &List->ListHead)
        return NULL;
    return CONTAINING_RECORD(Entry, GENERIC_LIST_ENTRY, Entry);
}


PGENERIC_LIST_ENTRY
GetNextListEntry(PGENERIC_LIST_ENTRY Entry)
{
    PLIST_ENTRY Next = Entry->Entry.Flink;

    if (Next == &Entry->List->ListHead)
        return NULL;
    return CONTAINING_RECORD(Next, GENERIC_LIST_ENTRY, Entry);
}


PVOID
GetListEntryUserData(PGENERIC_LIST_ENTRY List)
{
    return List->UserData;
}


LPCSTR
GetListEntryText(PGENERIC_LIST_ENTRY List)
{
    return List->Text;
}


VOID
GenericListKeyPress (PGENERIC_LIST GenericList, CHAR AsciChar)
{
    PGENERIC_LIST_ENTRY ListEntry;
    PGENERIC_LIST_ENTRY OldListEntry;
    BOOLEAN Flag = FALSE;

    ListEntry = GenericList->CurrentEntry;
    OldListEntry = GenericList->CurrentEntry;

    GenericList->Redraw = FALSE;

    if ((strlen(ListEntry->Text) > 0) && (tolower(ListEntry->Text[0]) == AsciChar) &&
         (GenericList->CurrentEntry->Entry.Flink != &GenericList->ListHead))
    {
        ScrollDownGenericList(GenericList);
        ListEntry = GenericList->CurrentEntry;

        if ((strlen(ListEntry->Text) > 0) && (tolower(ListEntry->Text[0]) == AsciChar))
            goto End;
    }

    while (GenericList->CurrentEntry->Entry.Blink != &GenericList->ListHead)
        ScrollUpGenericList(GenericList);

    ListEntry = GenericList->CurrentEntry;

    for (;;)
    {
        if ((strlen(ListEntry->Text) > 0) && (tolower(ListEntry->Text[0]) == AsciChar))
        {
            Flag = TRUE;
            break;
        }

        if (GenericList->CurrentEntry->Entry.Flink == &GenericList->ListHead)
            break;

        ScrollDownGenericList(GenericList);
        ListEntry = GenericList->CurrentEntry;
    }

    if (!Flag)
    {
        while (GenericList->CurrentEntry->Entry.Blink != &GenericList->ListHead)
        {
            if (GenericList->CurrentEntry != OldListEntry)
                ScrollUpGenericList(GenericList);
            else
                break;
        }
    }
End:
    DrawListEntries(GenericList);
    DrawScrollBarGenericList(GenericList);

    GenericList->Redraw = TRUE;
}


VOID
SaveGenericListState(PGENERIC_LIST List)
{
    List->BackupEntry = List->CurrentEntry;
}


VOID
RestoreGenericListState(PGENERIC_LIST List)
{
    List->CurrentEntry = List->BackupEntry;
}

/* EOF */
