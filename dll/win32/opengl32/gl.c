/*
 * COPYRIGHT:            See COPYING in the top level directory
 * PROJECT:              Odyssey kernel
 * FILE:                 lib/opengl32/gl.c
 * PURPOSE:              OpenGL32 lib, glXXX functions
 * PROGRAMMER:           Anich Gregor (blight)
 * UPDATE HISTORY:
 *                       Feb 2, 2004: Created
 */

/* On a x86 we call the ICD functions in a special-way:
 *
 * For every glXXX function we export a glXXX entry-point which loads the
 * matching "real" function pointer from the NtCurrentTeb()->glDispatchTable
 * for gl functions in teblist.h and for others it gets the pointer from
 * NtCurrentTeb()->glTable and jmps to the address, leaving the stack alone and
 * letting the "real" function return for us.
 * Royce has implemented this in NASM =D
 *
 * On other machines we use C to forward the calls (slow...)
 */

#include "opengl32.h"

#if defined(_M_IX86)
C_ASSERT(FIELD_OFFSET(TEB, glTable) == 0xbe8);
#endif

int WINAPI glEmptyFunc0( void ) { return 0; }
int WINAPI glEmptyFunc4( long l1 ) { return 0; }
int WINAPI glEmptyFunc8( long l1, long l2 ) { return 0; }
int WINAPI glEmptyFunc12( long l1, long l2, long l3 ) { return 0; }
int WINAPI glEmptyFunc16( long l1, long l2, long l3, long l4 ) { return 0; }
int WINAPI glEmptyFunc20( long l1, long l2, long l3, long l4, long l5 )
                          { return 0; }
int WINAPI glEmptyFunc24( long l1, long l2, long l3, long l4, long l5,
                          long l6 ) { return 0; }
int WINAPI glEmptyFunc28( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7 ) { return 0; }
int WINAPI glEmptyFunc32( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8 ) { return 0; }
int WINAPI glEmptyFunc36( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8, long l9 ) { return 0; }
int WINAPI glEmptyFunc40( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8, long l9, long l10 )
                          { return 0; }
int WINAPI glEmptyFunc44( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8, long l9, long l10,
                          long l11 ) { return 0; }
int WINAPI glEmptyFunc48( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8, long l9, long l10,
                          long l11, long l12 ) { return 0; }
int WINAPI glEmptyFunc52( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8, long l9, long l10,
                          long l11, long l12, long l13 ) { return 0; }
int WINAPI glEmptyFunc56( long l1, long l2, long l3, long l4, long l5,
                          long l6, long l7, long l8, long l9, long l10,
                          long l11, long l12, long l13, long l14 )
                          { return 0; }

#if defined(_M_IX86)
# define FOO(x) #x

#ifdef __GNUC__
# define X(func, ret, typeargs, args, icdidx, tebidx, stack) \
__asm__(".align 4"                                    "\n\t" \
        ".globl _"#func"@"#stack                      "\n\t" \
        "_"#func"@"#stack":"                          "\n\t" \
        "       movl %fs:0x18, %eax"                  "\n\t" \
        "       movl 0xbe8(%eax), %eax"               "\n\t" \
        "       jmp *"FOO((icdidx*4))"(%eax)"         "\n\t");
#elif defined(_MSC_VER)
# define X(func, ret, typeargs, args, icdidx, tebidx, stack) \
__declspec(naked) ret WINAPI func typeargs                   \
{                                                            \
	__asm { mov eax, dword ptr fs:[18h] };                   \
	__asm { mov eax, dword ptr [eax+0be8h] };                \
	__asm { jmp dword ptr [eax+icdidx*4] };                  \
}
#else
# define X(func, ret, typeargs, args, icdidx, tebidx, stack) \
ret WINAPI func typeargs                                     \
{                                                            \
    PROC *table;                                             \
    PROC fn;                                                 \
    if (tebidx >= 0 && 0)                                    \
    {                                                        \
        table = (PROC *)NtCurrentTeb()->glDispatchTable;     \
        fn = table[tebidx];                                  \
    }                                                        \
    else                                                     \
    {                                                        \
        table = (PROC *)NtCurrentTeb()->glTable;             \
        fn = table[icdidx];                                  \
    }                                                        \
    return (ret)((ret(*)typeargs)fn)args;                    \
}
#endif

GLFUNCS_MACRO
# undef FOO
# undef X
#else /* defined(_M_IX86) */
# define X(func, ret, typeargs, args, icdidx, tebidx, stack) \
ret WINAPI func typeargs                                     \
{                                                            \
    PROC *table;                                             \
    PROC fn;                                                 \
    if (tebidx >= 0 && 0)                                    \
    {                                                        \
        table = (PROC *)NtCurrentTeb()->glDispatchTable;     \
        fn = table[tebidx];                                  \
    }                                                        \
    else                                                     \
    {                                                        \
        table = (PROC *)NtCurrentTeb()->glTable;             \
        fn = table[icdidx];                                  \
    }                                                        \
    return (ret)((ret(*)typeargs)fn)args;                    \
}

GLFUNCS_MACRO

# undef X
#endif /* !defined(_M_IX86) */

/* EOF */
