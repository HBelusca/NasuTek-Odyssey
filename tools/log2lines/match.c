/*
 * Odyssey log2lines
 * Written by Jan Roeloffzen
 *
 * - Custom match routines
 */

#include <string.h>

#include "config.h"
#include "log2lines.h"
#include "match.h"

// break pattern: show source+line
static int match_break(FILE *outFile, char *Line, int processed)
{
    static int state = 0;

    if ( processed ) return processed;
    switch (state)
    {
    case 1:
        state = 0;
        break;
    default:
        state = 0;
    }
    return 1;
}
// "mod" command: update relocated addresses
static int match_mod(FILE *outFile, char *Line, int processed)
{
    static int state = 0;
    char Image[NAMESIZE];
    DWORD Base;
    DWORD Size;
    PLIST_MEMBER plm;

    int cnt;

    if ( processed ) return processed;
    if ( (cnt = sscanf(Line," Base Size %5s", Image)) == 1 )
    {
        l2l_dbg(1, "Module relocate list:\n");
        state = 1;
        return 0;
    }
    switch (state)
    {
    case 1:
        if ( (cnt = sscanf(Line,"%lx %lx %20s", &Base, &Size, Image)) == 3 )
        {
            if (( plm = entry_lookup(&cache, Image) ))
            {
                plm->RelBase = Base;
                plm->Size = Size;
                l2l_dbg(1, "Relocated: %s %08x -> %08x\n", Image, plm->ImageBase, plm->RelBase);
            }
            return 0;
        }
        else
        {
            state = 0;
        }
        break;
    default:
        state = 0;
    }
    return 1;
}

int match_line(FILE *outFile, char *Line)
{
    int processed = 1;

    if ( *Line == '\n' || *Line == '\0' )
        return 1;
    if ( strncmp(Line, KDBG_CONT, sizeof(KDBG_CONT)-1 ) == 0 )
        return 1;

    processed = match_mod(outFile, Line, processed);
    processed = match_break(outFile, Line, processed);
    /* more to be appended here:
     * processed = match_xxx(outFile, Line, processed );
     * ...
     */

    return (int)(Line[0]);
}

/* EOF */
