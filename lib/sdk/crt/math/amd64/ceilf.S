/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey system libraries
 * PURPOSE:           Implementation of tan
 * FILE:              lib/sdk/crt/math/amd64/ceilf.S
 * PROGRAMMER:        Timo Kreuzer (timo.kreuzer@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <asm.inc>
#include <ksamd64.inc>

/* CODE **********************************************************************/
.code64

PUBLIC ceilf
FUNC ceilf
    sub rsp, 16
    .ENDPROLOG

    /* Put parameter on the stack */
    movss dword ptr [rsp], xmm0
    fld dword ptr [rsp]

    /* Change fpu control word to round up */
    fstcw [rsp + 8]
    mov eax, [rsp + 8]
    or  eax, HEX(00800)
    and eax, HEX(0fbff)
    mov [rsp + 12], eax
    fldcw [rsp + 12]

    /* Round to integer */
    frndint

    /* Restore fpu control word */
    fldcw [rsp + 8]

    fstp dword ptr [rsp]
    movss xmm0, dword ptr [rsp]

    add rsp, 16
    ret
ENDFUNC ceilf

END
