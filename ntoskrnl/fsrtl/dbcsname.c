/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/fsrtl/name.c
 * PURPOSE:         Provides DBCS parsing and other support routines for FSDs
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 *                  Pierre Schweitzer (pierre.schweitzer@odyssey.org) 
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* PUBLIC FUNCTIONS **********************************************************/

/*++
 * @name FsRtlDissectDbcs
 * @implemented
 *
 * Dissects a given path name into first and remaining part.
 *
 * @param Name
 *        ANSI string to dissect.
 *
 * @param FirstPart
 *        Pointer to user supplied ANSI_STRING, that will later point
 *        to the first part of the original name.
 *
 * @param RemainingPart
 *        Pointer to user supplied ANSI_STRING, that will later point
 *        to the remaining part of the original name.
 *
 * @return None
 *
 * @remarks Example:
 *          Name:           \test1\test2\test3
 *          FirstPart:      test1
 *          RemainingPart:  test2\test3
 *
 *--*/
VOID
NTAPI
FsRtlDissectDbcs(IN ANSI_STRING Name,
                 OUT PANSI_STRING FirstPart,
                 OUT PANSI_STRING RemainingPart)
{
    USHORT FirstPosition, i;
    USHORT SkipFirstSlash = 0;
    PAGED_CODE();

    /* Zero the strings before continuing */
    RtlZeroMemory(FirstPart, sizeof(ANSI_STRING));
    RtlZeroMemory(RemainingPart, sizeof(ANSI_STRING));

    /* Just quit if the string is empty */
    if (!Name.Length) return;

    /* Find first backslash */
    FirstPosition = Name.Length;
    for (i = 0; i < Name.Length; i++)
    {
        /* First make sure the character it's not the Lead DBCS */
        if (FsRtlIsLeadDbcsCharacter(Name.Buffer[i]))
        {
            i++;
        }
        /* If we found one... */
        else if (Name.Buffer[i] == '\\')
        {
            /* If it begins string, just notice it and continue */
            if (i == 0)
            {
                SkipFirstSlash = 1;
            }
            else
            {
                /* Else, save its position and break out of the loop */
                FirstPosition = i;
                break;
            }
        }
    }

    /* Set up the first result string */
    FirstPart->Buffer = Name.Buffer + SkipFirstSlash;
    FirstPart->Length = (FirstPosition - SkipFirstSlash);
    FirstPart->MaximumLength = FirstPart->Length;

    /* And second one, if necessary */
    if (FirstPosition < (Name.Length))
    {
        RemainingPart->Buffer = Name.Buffer + FirstPosition + 1;
        RemainingPart->Length = Name.Length - (FirstPosition + 1);
        RemainingPart->MaximumLength = RemainingPart->Length;
    }
}

/*++
 * @name FsRtlDoesDbcsContainWildCards
 * @implemented
 *
 * Returns TRUE if the given DbcsName contains wildcards such as *, ?, 
 * ANSI_DOS_STAR, ANSI_DOS_DOT, and ANSI_DOS_QM
 *
 * @param Name
 *        The Name to check
 *
 * @return TRUE if there are wildcards, FALSE otherwise
 *
 * @remarks None
 *
 *--*/
BOOLEAN
NTAPI
FsRtlDoesDbcsContainWildCards(IN PANSI_STRING Name)
{
    USHORT i;
    PAGED_CODE();

    /* Check every character */
    for (i = 0; i < Name->Length; i++)
    {
        /* First make sure it's not the Lead DBCS */
        if (FsRtlIsLeadDbcsCharacter(Name->Buffer[i]))
        {
            i++;
        }
        else if (FsRtlIsAnsiCharacterWild(Name->Buffer[i]))
        {
            /* Now return if it has a wildcard */
            return TRUE;
        }
    }

    /* We didn't return above...so none found */
    return FALSE;
}

/*++
 * @name FsRtlIsDbcsInExpression
 * @implemented
 *
 * Check if the Name string is in the Expression string.
 *
 * @param Expression
 *        The string in which we've to find Name. It can contains wildcards
 *
 * @param Name
 *        The string to find. It cannot contain wildcards.
 *
 * @return TRUE if Name is found in Expression, FALSE otherwise
 *
 * @remarks
 *
 *--*/
BOOLEAN
NTAPI
FsRtlIsDbcsInExpression(IN PANSI_STRING Expression,
                        IN PANSI_STRING Name)
{
    USHORT ExpressionPosition = 0, NamePosition = 0, MatchingChars, StarFound = MAXUSHORT;
    PAGED_CODE();

    ASSERT(Name->Length);
    ASSERT(Expression->Length);
    ASSERT(!FsRtlDoesDbcsContainWildCards(Name));

    while (NamePosition < Name->Length && ExpressionPosition < Expression->Length)
    {
        if ((Expression->Buffer[ExpressionPosition] == Name->Buffer[NamePosition]))
        {
            NamePosition++;
            ExpressionPosition++;
        }
        else if (StarFound != MAXUSHORT && (Expression->Buffer[StarFound + 1] == '*' ||
                 Expression->Buffer[StarFound + 1] == '?' || Expression->Buffer[StarFound + 1] == ANSI_DOS_DOT))
        {
            ExpressionPosition = StarFound + 1;
            switch (Expression->Buffer[ExpressionPosition])
            {
                case '*':
                    StarFound = MAXUSHORT;
                    break;

                case '?':
                    if (++ExpressionPosition == Expression->Length)
                    {
                        NamePosition = Name->Length;
                        break;
                    }

                    MatchingChars = NamePosition;
                    while (NamePosition < Name->Length &&
                           Name->Buffer[NamePosition] != Expression->Buffer[ExpressionPosition])
                    {
                        NamePosition++;
                    }

                    if (NamePosition - MatchingChars > 0)
                    {
                        StarFound = MAXUSHORT;
                    }
                    break;

                case ANSI_DOS_DOT:
                    while (NamePosition < Name->Length && Name->Buffer[NamePosition] != '.')
                    {
                        NamePosition++;
                    }
                    ExpressionPosition++;
                    StarFound = MAXUSHORT;
                    break;

                default:
                    /* Should never happen */
                    ASSERT(FALSE);                   
            }
        }
        else if ((Expression->Buffer[ExpressionPosition] == '?') || (Expression->Buffer[ExpressionPosition] == ANSI_DOS_QM) ||
                 (Expression->Buffer[ExpressionPosition] == ANSI_DOS_DOT && Name->Buffer[NamePosition] == '.'))
        {
            NamePosition++;
            ExpressionPosition++;
            StarFound = MAXUSHORT;
        }
        else if (Expression->Buffer[ExpressionPosition] == '*')
        {
            StarFound = ExpressionPosition++;
            if (ExpressionPosition == Expression->Length)
            {
                NamePosition = Name->Length;
                break;
            }
        }
        else if (Expression->Buffer[ExpressionPosition] == ANSI_DOS_STAR)
        {
            StarFound = MAXUSHORT;
            MatchingChars = NamePosition;
            while (MatchingChars < Name->Length)
            {
                if (Name->Buffer[MatchingChars] == '.')
                {
                    NamePosition = MatchingChars;
                }
                MatchingChars++;
            }
            ExpressionPosition++;
        }
        else if (StarFound != MAXUSHORT)
        {
            ExpressionPosition = StarFound + 1;
            while (NamePosition < Name->Length &&
                   Name->Buffer[NamePosition] != Expression->Buffer[ExpressionPosition])
            {
                NamePosition++;
            }
        }
        else
        {
            break;
        }
    }
    if (ExpressionPosition + 1 == Expression->Length && NamePosition == Name->Length &&
        Expression->Buffer[ExpressionPosition] == ANSI_DOS_DOT)
    {
        ExpressionPosition++;
    }

    return (ExpressionPosition == Expression->Length && NamePosition == Name->Length);
}

/*++
 * @name FsRtlIsFatDbcsLegal
 * @implemented
 *
 * Returns TRUE if the given DbcsName is a valid FAT filename (in 8.3)
 *
 * @param DbcsName
 *        The filename to check. It can also contains pathname.
 *
 * @param WildCardsPermissible
 *        If this is set to FALSE and if filename contains wildcard, the function 
 *        will fail
 *
 * @param PathNamePermissible
 *        If this is set to FALSE and if the filename comes with a pathname, the
 *        function will fail
 *
 * @param LeadingBackslashPermissible
 *        If this is set to FALSE and if the filename starts with a backslash, the
 *        function will fail
 *
 * @return TRUE if the DbcsName is legal, FALSE otherwise
 *
 * @remarks None
 *
 *--*/
BOOLEAN
NTAPI
FsRtlIsFatDbcsLegal(IN ANSI_STRING DbcsName,
                    IN BOOLEAN WildCardsPermissible,
                    IN BOOLEAN PathNamePermissible,
                    IN BOOLEAN LeadingBackslashPermissible)
{
    ANSI_STRING FirstPart, RemainingPart, Name;
    BOOLEAN LastDot;
    USHORT i;
    PAGED_CODE();

    /* Just quit if the string is empty */
    if (!DbcsName.Length)
        return FALSE;

    /* DbcsName wasn't supposed to be started with \ */
    if (!LeadingBackslashPermissible && DbcsName.Buffer[0] == '\\')
        return FALSE;
    /* DbcsName was allowed to be started with \, but now, remove it */
    else if (LeadingBackslashPermissible && DbcsName.Buffer[0] == '\\')
    {
        DbcsName.Buffer = DbcsName.Buffer + 1;
        DbcsName.Length = DbcsName.Length - 1;
        DbcsName.MaximumLength = DbcsName.MaximumLength - 1;
    }

    /* Extract first part of the DbcsName to work on */
    FsRtlDissectDbcs(DbcsName, &FirstPart, &RemainingPart);
    while (FirstPart.Length > 0)
    {
        /* Reset dots count */
        LastDot = FALSE;

        /* Accept special filename if wildcards are allowed */
        if (WildCardsPermissible && (FirstPart.Length == 1 || FirstPart.Length == 2) && FirstPart.Buffer[0] == '.')
        {
            if (FirstPart.Length == 2)
            {
                if (FirstPart.Buffer[1] == '.')
                {
                    goto EndLoop;
                }
            }
            else
            {
                goto EndLoop;
            }
        }

        /* Filename must be 8.3 filename */
        if (FirstPart.Length < 3 || FirstPart.Length > 12)
            return FALSE;

        /* Now, we will parse the filename to find everything bad in */
        for (i = 0; i < FirstPart.Length; i++)
        {
            /* First make sure the character it's not the Lead DBCS */
            if (FsRtlIsLeadDbcsCharacter(FirstPart.Buffer[i]))
            {
                if (i == (FirstPart.Length) - 1)
                    return FALSE;
                i++;
            }
            /* Then check for bad characters */
            else if (!FsRtlIsAnsiCharacterLegalFat(FirstPart.Buffer[i], WildCardsPermissible))
            {
                return FALSE;
            }
            else if (FirstPart.Buffer[i] == '.')
            {
                /* Filename can only contain one dot */
                if (LastDot)
                    return FALSE;

                LastDot = TRUE;

                /* We mustn't have spaces before dot or at the end of the filename
                 * and no dot at the beginning of the filename */
                if ((i == (FirstPart.Length) - 1) || i == 0)
                    return FALSE;

                if (i > 0)
                    if (FirstPart.Buffer[i - 1] == ' ')
                        return FALSE;

                /* Filename must be 8.3 filename and not 3.8 filename */
                if ((FirstPart.Length - 1) - i > 3)
                    return FALSE;
            }
        }

        /* Filename mustn't finish with a space */
        if (FirstPart.Buffer[FirstPart.Length - 1] == ' ')
            return FALSE;

        EndLoop:
        /* Preparing next loop */
        Name.Buffer = RemainingPart.Buffer;
        Name.Length = RemainingPart.Length;
        Name.MaximumLength = RemainingPart.MaximumLength;

        /* Call once again our dissect function */
        FsRtlDissectDbcs(Name, &FirstPart, &RemainingPart);

        /* We found a pathname, it wasn't allowed */
        if (FirstPart.Length > 0 && !PathNamePermissible)
            return FALSE;
    }
    return TRUE;
}

/*++
 * @name FsRtlIsHpfsDbcsLegal
 * @implemented
 *
 * Returns TRUE if the given DbcsName is a valid HPFS filename
 *
 * @param DbcsName
 *        The filename to check. It can also contains pathname.
 *
 * @param WildCardsPermissible
 *        If this is set to FALSE and if filename contains wildcard, the function 
 *        will fail
 *
 * @param PathNamePermissible
 *        If this is set to FALSE and if the filename comes with a pathname, the
 *        function will fail
 *
 * @param LeadingBackslashPermissible
 *        If this is set to FALSE and if the filename starts with a backslash, the
 *        function will fail
 *
 * @return TRUE if the DbcsName is legal, FALSE otherwise
 *
 * @remarks None
 *
 *--*/
BOOLEAN
NTAPI
FsRtlIsHpfsDbcsLegal(IN ANSI_STRING DbcsName,
                     IN BOOLEAN WildCardsPermissible,
                     IN BOOLEAN PathNamePermissible,
                     IN BOOLEAN LeadingBackslashPermissible)
{
    ANSI_STRING FirstPart, RemainingPart, Name;
    USHORT i;
    PAGED_CODE();

    /* Just quit if the string is empty */
    if (!DbcsName.Length)
        return FALSE;

    /* DbcsName wasn't supposed to be started with \ */
    if (!LeadingBackslashPermissible && DbcsName.Buffer[0] == '\\')
        return FALSE;
    /* DbcsName was allowed to be started with \, but now, remove it */
    else if (LeadingBackslashPermissible && DbcsName.Buffer[0] == '\\')
    {
        DbcsName.Buffer = DbcsName.Buffer + 1;
        DbcsName.Length = DbcsName.Length - 1;
        DbcsName.MaximumLength = DbcsName.MaximumLength - 1;
    }

    /* Extract first part of the DbcsName to work on */
    FsRtlDissectDbcs(DbcsName, &FirstPart, &RemainingPart);
    while (FirstPart.Length > 0)
    {
        /* Accept special filename if wildcards are allowed */
        if (WildCardsPermissible && (FirstPart.Length == 1 || FirstPart.Length == 2) && FirstPart.Buffer[0] == '.')
        {
            if (FirstPart.Length == 2)
            {
                if (FirstPart.Buffer[1] == '.')
                {
                    goto EndLoop;
                }
            }
            else
            {
                goto EndLoop;
            }
        }

        /* Filename must be 255 bytes maximum */
        if (FirstPart.Length > 255)
            return FALSE;

        /* Now, we will parse the filename to find everything bad in */
        for (i = 0; i < FirstPart.Length; i++)
        {
            /* First make sure the character it's not the Lead DBCS */
            if (FsRtlIsLeadDbcsCharacter(FirstPart.Buffer[i]))
            {
                if (i == (FirstPart.Length) - 1)
                    return FALSE;
                i++;
            }
            /* Then check for bad characters */
            else if (!FsRtlIsAnsiCharacterLegalHpfs(FirstPart.Buffer[i], WildCardsPermissible))
            {
                return FALSE;
            }
        }

        /* Filename mustn't finish with a space or a dot */
        if ((FirstPart.Buffer[FirstPart.Length - 1] == ' ') ||
            (FirstPart.Buffer[FirstPart.Length - 1] == '.'))
            return FALSE;

        EndLoop:
        /* Preparing next loop */
        Name.Buffer = RemainingPart.Buffer;
        Name.Length = RemainingPart.Length;
        Name.MaximumLength = RemainingPart.MaximumLength;

        /* Call once again our dissect function */
        FsRtlDissectDbcs(Name, &FirstPart, &RemainingPart);

        /* We found a pathname, it wasn't allowed */
        if (FirstPart.Length > 0 && !PathNamePermissible)
            return FALSE;
    }
    return TRUE;
}
