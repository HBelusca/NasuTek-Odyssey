/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey system libraries
 * PURPOSE:           Implementation of _chkstk and _alloca_probe
 * PROGRAMMERS        Richard Henderson <rth@redhat.com>
 *                    Kai Tietz <kai.tietz@onevision.com>
 *                    Timo Kreuzer (timo.kreuzer@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <asm.inc>
#define PAGE_SIZE 4096

/* CODE **********************************************************************/
.code64

PUBLIC ___chkstk_ms

    //cfi_startproc()
___chkstk_ms:
    push rcx                    /* save temps */
    //cfi_push(%rcx)
    push rax
    //cfi_push(%rax)

    cmp rax, PAGE_SIZE          /* > 4k ?*/
    lea rcx, [rsp + 24]         /* point past return addr */
    jb l_LessThanAPage

l_MoreThanAPage:
    sub rcx, PAGE_SIZE          /* yes, move pointer down 4k */
    or byte ptr [rcx], 0        /* probe there */
    sub rax, PAGE_SIZE          /* decrement count */

    cmp rax, PAGE_SIZE
    ja l_MoreThanAPage          /* and do it again */

l_LessThanAPage:
    sub rcx, rax
    or byte ptr [rcx], 0        /* less than 4k, just peek here */

    pop rax
    //cfi_pop(%rax)
    pop rcx
    //cfi_pop(%rcx)
    ret
    //cfi_endproc()

END
/* EOF */
