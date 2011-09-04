/* $Id: environ.c 52854 2011-07-24 23:42:09Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/kernel32/misc/env.c
 * PURPOSE:         Environment functions
 * PROGRAMMER:      Ariadne ( ariadne@xs4all.nl)
 *                  Emanuele Aliberti
 *                  Thomas Weidenmueller
 * UPDATE HISTORY:
 *                  Created 01/11/98
 */

#include <k32.h>

#define NDEBUG
#include <debug.h>


/* FUNCTIONS ******************************************************************/

/*
 * @implemented
 */
DWORD
WINAPI
GetEnvironmentVariableA (
	LPCSTR	lpName,
	LPSTR	lpBuffer,
	DWORD	nSize
	)
{
	ANSI_STRING VarName;
	ANSI_STRING VarValue;
	UNICODE_STRING VarNameU;
	UNICODE_STRING VarValueU;
	NTSTATUS Status;

	/* initialize unicode variable name string */
	RtlInitAnsiString (&VarName,
	                   (LPSTR)lpName);
	RtlAnsiStringToUnicodeString (&VarNameU,
	                              &VarName,
	                              TRUE);

	/* initialize ansi variable value string */
	VarValue.Length = 0;
	VarValue.MaximumLength = (USHORT)nSize;
	VarValue.Buffer = lpBuffer;

	/* initialize unicode variable value string and allocate buffer */
	VarValueU.Length = 0;
	if (nSize != 0)
	{
	    VarValueU.MaximumLength = (USHORT)(nSize - 1) * sizeof(WCHAR);
	    VarValueU.Buffer = RtlAllocateHeap (RtlGetProcessHeap (),
	                                        0,
	                                        nSize * sizeof(WCHAR));
            if (VarValueU.Buffer != NULL)
            {
                /* NULL-terminate the buffer in any case! RtlQueryEnvironmentVariable_U
                   only terminates it if MaximumLength < Length! */
                VarValueU.Buffer[nSize - 1] = L'\0';
            }
        }
	else
	{
            VarValueU.MaximumLength = 0;
            VarValueU.Buffer = NULL;
	}

        if (VarValueU.Buffer != NULL || nSize == 0)
        {
            /* get unicode environment variable */
	    Status = RtlQueryEnvironmentVariable_U (NULL,
	                                            &VarNameU,
	                                            &VarValueU);
	    if (!NT_SUCCESS(Status))
	    {
		/* free unicode buffer */
		RtlFreeHeap (RtlGetProcessHeap (),
		             0,
		             VarValueU.Buffer);

		/* free unicode variable name string */
		RtlFreeUnicodeString (&VarNameU);

		BaseSetLastNTError (Status);
		if (Status == STATUS_BUFFER_TOO_SMALL)
		{
			return (VarValueU.Length / sizeof(WCHAR)) + 1;
		}
		else
		{
			return 0;
		}
	    }

	    /* convert unicode value string to ansi */
	    RtlUnicodeStringToAnsiString (&VarValue,
	                                  &VarValueU,
	                                  FALSE);

            if (VarValueU.Buffer != NULL)
            {
                /* free unicode buffer */
	        RtlFreeHeap (RtlGetProcessHeap (),
	                     0,
	                     VarValueU.Buffer);
            }

	    /* free unicode variable name string */
	    RtlFreeUnicodeString (&VarNameU);

	    return (VarValueU.Length / sizeof(WCHAR));
        }
        else
        {
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
}


/*
 * @implemented
 */
DWORD
WINAPI
GetEnvironmentVariableW (
	LPCWSTR	lpName,
	LPWSTR	lpBuffer,
	DWORD	nSize
	)
{
	UNICODE_STRING VarName;
	UNICODE_STRING VarValue;
	NTSTATUS Status;

	RtlInitUnicodeString (&VarName,
	                      lpName);

	VarValue.Length = 0;
	VarValue.MaximumLength = (USHORT) (nSize ? nSize - 1 : 0) * sizeof(WCHAR);
	VarValue.Buffer = lpBuffer;

	Status = RtlQueryEnvironmentVariable_U (NULL,
	                                        &VarName,
	                                        &VarValue);
	if (!NT_SUCCESS(Status))
	{
		if (Status == STATUS_BUFFER_TOO_SMALL)
		{
			return (VarValue.Length / sizeof(WCHAR)) + 1;
		}
		else
		{
			BaseSetLastNTError (Status);
			return 0;
		}
	}
	
        if (nSize != 0)
        {
            /* make sure the string is NULL-terminated! RtlQueryEnvironmentVariable_U
               only terminates it if MaximumLength < Length */
	    lpBuffer[VarValue.Length / sizeof(WCHAR)] = L'\0';
	}

	return (VarValue.Length / sizeof(WCHAR));
}


/*
 * @implemented
 */
BOOL
WINAPI
SetEnvironmentVariableA (
	LPCSTR	lpName,
	LPCSTR	lpValue
	)
{
	ANSI_STRING VarName;
	ANSI_STRING VarValue;
	UNICODE_STRING VarNameU;
	UNICODE_STRING VarValueU;
	NTSTATUS Status;

	DPRINT("SetEnvironmentVariableA(Name '%s', Value '%s')\n", lpName, lpValue);

	RtlInitAnsiString (&VarName,
	                   (LPSTR)lpName);
	RtlAnsiStringToUnicodeString (&VarNameU,
	                              &VarName,
	                              TRUE);

	if (lpValue)
	{
		RtlInitAnsiString (&VarValue,
		                   (LPSTR)lpValue);
		RtlAnsiStringToUnicodeString (&VarValueU,
		                              &VarValue,
		                              TRUE);

		Status = RtlSetEnvironmentVariable (NULL,
		                                    &VarNameU,
		                                    &VarValueU);

		RtlFreeUnicodeString (&VarValueU);
	}
	else
	{
		Status = RtlSetEnvironmentVariable (NULL,
		                                    &VarNameU,
		                                    NULL);
	}
	RtlFreeUnicodeString (&VarNameU);

	if (!NT_SUCCESS(Status))
	{
		BaseSetLastNTError (Status);
		return FALSE;
	}

	return TRUE;
}


/*
 * @implemented
 */
BOOL
WINAPI
SetEnvironmentVariableW (
	LPCWSTR	lpName,
	LPCWSTR	lpValue
	)
{
	UNICODE_STRING VarName;
	UNICODE_STRING VarValue;
	NTSTATUS Status;

	DPRINT("SetEnvironmentVariableW(Name '%S', Value '%S')\n", lpName, lpValue);

	RtlInitUnicodeString (&VarName,
	                      lpName);

	RtlInitUnicodeString (&VarValue,
	                      lpValue);

	Status = RtlSetEnvironmentVariable (NULL,
	                                    &VarName,
	                                    &VarValue);

	if (!NT_SUCCESS(Status))
	{
		BaseSetLastNTError (Status);
		return FALSE;
	}

	return TRUE;
}


/*
 * @implemented
 */
LPSTR
WINAPI
GetEnvironmentStringsA (
	VOID
	)
{
	UNICODE_STRING UnicodeString;
	ANSI_STRING AnsiString;
	PWCHAR EnvU;
	PWCHAR PtrU;
	ULONG  Length;
	PCHAR EnvPtr = NULL;

	EnvU = (PWCHAR)(NtCurrentPeb ()->ProcessParameters->Environment);

	if (EnvU == NULL)
		return NULL;

	if (*EnvU == 0)
		return NULL;

	/* get environment size */
	PtrU = EnvU;
	while (*PtrU)
	{
		while (*PtrU)
			PtrU++;
		PtrU++;
	}
	Length = (ULONG)(PtrU - EnvU);
	DPRINT("Length %lu\n", Length);

	/* allocate environment buffer */
	EnvPtr = RtlAllocateHeap (RtlGetProcessHeap (),
	                          0,
	                          Length + 1);
        if (EnvPtr == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
	DPRINT("EnvPtr %p\n", EnvPtr);

	/* convert unicode environment to ansi */
	UnicodeString.MaximumLength = (USHORT)Length * sizeof(WCHAR) + sizeof(WCHAR);
	UnicodeString.Buffer = EnvU;

	AnsiString.MaximumLength = (USHORT)Length + 1;
	AnsiString.Length = 0;
	AnsiString.Buffer = EnvPtr;

	DPRINT ("UnicodeString.Buffer \'%S\'\n", UnicodeString.Buffer);

	while (*(UnicodeString.Buffer))
	{
		UnicodeString.Length = wcslen (UnicodeString.Buffer) * sizeof(WCHAR);
		UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);
		if (UnicodeString.Length > 0)
		{
			AnsiString.Length = 0;
			AnsiString.MaximumLength = (USHORT)Length + 1 - (AnsiString.Buffer - EnvPtr);

			RtlUnicodeStringToAnsiString (&AnsiString,
			                              &UnicodeString,
			                              FALSE);

			AnsiString.Buffer += (AnsiString.Length + 1);
			UnicodeString.Buffer += ((UnicodeString.Length / sizeof(WCHAR)) + 1);
		}
	}
	*(AnsiString.Buffer) = 0;

	return EnvPtr;
}


/*
 * @implemented
 */
LPWSTR
WINAPI
GetEnvironmentStringsW (
	VOID
	)
{
	return (LPWSTR)(NtCurrentPeb ()->ProcessParameters->Environment);
}


/*
 * @implemented
 */
BOOL
WINAPI
FreeEnvironmentStringsA (
	LPSTR	EnvironmentStrings
	)
{
	if (EnvironmentStrings == NULL)
		return FALSE;

	RtlFreeHeap (RtlGetProcessHeap (),
	             0,
	             EnvironmentStrings);

	return TRUE;
}


/*
 * @implemented
 */
BOOL
WINAPI
FreeEnvironmentStringsW (
	LPWSTR	EnvironmentStrings
	)
{
 return TRUE;
}


/*
 * @implemented
 */
DWORD
WINAPI
ExpandEnvironmentStringsA (
	LPCSTR	lpSrc,
	LPSTR	lpDst,
	DWORD	nSize
	)
{
	ANSI_STRING Source;
	ANSI_STRING Destination;
	UNICODE_STRING SourceU;
	UNICODE_STRING DestinationU;
	NTSTATUS Status;
	ULONG Length = 0;

	RtlInitAnsiString (&Source,
	                   (LPSTR)lpSrc);
	Status = RtlAnsiStringToUnicodeString (&SourceU,
	                                       &Source,
	                                       TRUE);
        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError (Status);
            return 0;
        }

    /* make sure we don't overflow the maximum ANSI_STRING size */
    if (nSize > 0x7fff)
        nSize = 0x7fff;

	Destination.Length = 0;
	Destination.MaximumLength = (USHORT)nSize;
	Destination.Buffer = lpDst;

	DestinationU.Length = 0;
	DestinationU.MaximumLength = (USHORT)nSize * sizeof(WCHAR);
	DestinationU.Buffer = RtlAllocateHeap (RtlGetProcessHeap (),
	                                       0,
	                                       DestinationU.MaximumLength);
        if (DestinationU.Buffer == NULL)
        {
            RtlFreeUnicodeString(&SourceU);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }

	Status = RtlExpandEnvironmentStrings_U (NULL,
	                                        &SourceU,
	                                        &DestinationU,
	                                        &Length);

	RtlFreeUnicodeString (&SourceU);

	if (!NT_SUCCESS(Status))
	{
		BaseSetLastNTError (Status);
		if (Status != STATUS_BUFFER_TOO_SMALL)
		{
			RtlFreeHeap (RtlGetProcessHeap (),
			             0,
			             DestinationU.Buffer);
			return 0;
		}
	}

	RtlUnicodeStringToAnsiString (&Destination,
	                              &DestinationU,
	                              FALSE);

	RtlFreeHeap (RtlGetProcessHeap (),
	             0,
	             DestinationU.Buffer);

	return (Length / sizeof(WCHAR));
}


/*
 * @implemented
 */
DWORD
WINAPI
ExpandEnvironmentStringsW (
	LPCWSTR	lpSrc,
	LPWSTR	lpDst,
	DWORD	nSize
	)
{
	UNICODE_STRING Source;
	UNICODE_STRING Destination;
	NTSTATUS Status;
	ULONG Length = 0;

	RtlInitUnicodeString (&Source,
	                      (LPWSTR)lpSrc);

    /* make sure we don't overflow the maximum UNICODE_STRING size */
    if (nSize > 0x7fff)
        nSize = 0x7fff;

	Destination.Length = 0;
	Destination.MaximumLength = (USHORT)nSize * sizeof(WCHAR);
	Destination.Buffer = lpDst;

	Status = RtlExpandEnvironmentStrings_U (NULL,
	                                        &Source,
	                                        &Destination,
	                                        &Length);
	if (!NT_SUCCESS(Status))
	{
		BaseSetLastNTError (Status);
		if (Status != STATUS_BUFFER_TOO_SMALL)
			return 0;
	}

	return (Length / sizeof(WCHAR));
}

/*
 * @implemented
 */
BOOL
WINAPI
SetEnvironmentStringsA(IN LPCH NewEnvironment)
{
    STUB;
    return FALSE;
}

/*
 * @implemented
 */
BOOL
WINAPI
SetEnvironmentStringsW(IN LPWCH NewEnvironment)
{
    STUB;
    return FALSE;
}

/* EOF */
