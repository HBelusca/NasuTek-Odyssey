/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey system libraries
 * PURPOSE:           Implementation of tan
 * FILE:              lib/sdk/crt/math/amd64/floorf.S
 * PROGRAMMER:        Timo Kreuzer (timo.kreuzer@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <asm.inc>
#include <ksamd64.inc>

/* CODE **********************************************************************/
.code64

PUBLIC floorf
FUNC floorf
    sub rsp, 16
    .ENDPROLOG

    /* Put parameter on the stack */
    movss dword ptr [rsp], xmm0
    fld   dword ptr [rsp]

    /* Change fpu control word to round down */
    fstcw [rsp]
    mov   eax, [rsp]
    or    eax, HEX(000400)
    and   eax, HEX(00f7ff)
    mov   [rsp + 8], eax
    fldcw [rsp + 8]

    /* Round to integer */
    frndint

    /* Restore fpu control word */
    fldcw [rsp]

    fstp  dword ptr [rsp]
    movss xmm0, dword ptr [rsp]
    add rsp, 16
    ret
ENDFUNC floorf

END
