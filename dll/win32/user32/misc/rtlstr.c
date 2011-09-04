/*
 * PROJECT:         Odyssey user32.dll
 * FILE:            lib/user32/misc/rtlstr.c
 * PURPOSE:         Large Strings
 * PROGRAMMER:
 * UPDATE HISTORY:
 *
 */

/* INCLUDES ******************************************************************/

#include <user32.h>

#include <wine/debug.h>

WINE_DEFAULT_DEBUG_CHANNEL(user32);

/* FUNCTIONS *****************************************************************/
VOID
NTAPI
RtlInitLargeAnsiString(IN OUT PLARGE_ANSI_STRING DestinationString,
                       IN PCSZ SourceString,
                       IN INT Unknown)
{
    ULONG DestSize;

    if (SourceString)
    {
        DestSize = strlen(SourceString);
        DestinationString->Length = DestSize;
        DestinationString->MaximumLength = DestSize + sizeof(CHAR);
    }
    else
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }

    DestinationString->Buffer = (PCHAR)SourceString;
    DestinationString->bAnsi  = TRUE;
}

VOID
NTAPI
RtlInitLargeUnicodeString(IN OUT PLARGE_UNICODE_STRING DestinationString,
                          IN PCWSTR SourceString,
                          IN INT Unknown)
{
    ULONG DestSize;

    if (SourceString)
    {
        DestSize = wcslen(SourceString) * sizeof(WCHAR);
        DestinationString->Length = DestSize;
        DestinationString->MaximumLength = DestSize + sizeof(WCHAR);
    }
    else
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }

    DestinationString->Buffer = (PWSTR)SourceString;
    DestinationString->bAnsi  = FALSE;
}

BOOL
NTAPI
RtlLargeStringToUnicodeString( PUNICODE_STRING DestinationString,
                               PLARGE_STRING SourceString)
{
  ANSI_STRING AnsiString;

  RtlInitUnicodeString(DestinationString, NULL);
  if (DestinationString && SourceString && SourceString->bAnsi)
  {
     RtlInitAnsiString(&AnsiString, (LPSTR)SourceString->Buffer);
     return NT_SUCCESS(RtlAnsiStringToUnicodeString(DestinationString, &AnsiString, TRUE));
  }
  else if (DestinationString && SourceString)
  {
     return RtlCreateUnicodeString(DestinationString, SourceString->Buffer);
  }
  else
     return FALSE;
}
