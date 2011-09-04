/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey system libraries
 * PURPOSE:           Implementation of _chkstk and _alloca_probe
 * FILE:              lib/sdk/crt/math/amd64/chkstk_asm.s
 * PROGRAMMER:        Timo Kreuzer (timo.kreuzer@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <asm.inc>

/* CODE **********************************************************************/
.code64

MsgUnimplemented:
    .ascii "Unimplemented", CR, LF, NUL

FUNC __chkstk
    .endprolog
    UNIMPLEMENTED chkstk
    ret
ENDFUNC __chkstk

FUNC __alloca_probe
    .endprolog
    UNIMPLEMENTED alloca_probe
    ret
ENDFUNC __alloca_probe

END
/* EOF */
