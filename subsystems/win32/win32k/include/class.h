#pragma once

#define IS_ATOM(x) \
  (((ULONG_PTR)(x) > 0x0) && ((ULONG_PTR)(x) < 0x10000))

typedef struct _WNDPROC_INFO
{
    WNDPROC WindowProc;
    BOOL IsUnicode;
} WNDPROC_INFO, *PWNDPROC_INFO;

static __inline BOOL
IsCallProcHandle(IN WNDPROC lpWndProc)
{
    /* FIXME - check for 64 bit architectures... */
    return ((ULONG_PTR)lpWndProc & 0xFFFF0000) == 0xFFFF0000;
}

VOID
DestroyCallProc(IN PDESKTOPINFO Desktop,
                IN OUT PCALLPROCDATA CallProc);

PCALLPROCDATA
CreateCallProc(IN PDESKTOP Desktop,
               IN WNDPROC WndProc,
               IN BOOL Unicode,
               IN PPROCESSINFO pi);

BOOL
UserGetCallProcInfo(IN HANDLE hCallProc,
                    OUT PWNDPROC_INFO wpInfo);

void FASTCALL
DestroyProcessClasses(PPROCESSINFO Process );

VOID
IntDereferenceClass(IN OUT PCLS Class,
                    IN PDESKTOPINFO Desktop,
                    IN PPROCESSINFO pi);

PCLS
IntGetAndReferenceClass(PUNICODE_STRING ClassName, HINSTANCE hInstance);

BOOL FASTCALL UserRegisterSystemClasses(VOID);

VOID
UserAddCallProcToClass(IN OUT PCLS Class,
                       IN PCALLPROCDATA CallProc);

BOOL
IntGetAtomFromStringOrAtom(IN PUNICODE_STRING ClassName,
                           OUT RTL_ATOM *Atom);

BOOL
IntCheckProcessDesktopClasses(IN PDESKTOP Desktop,
                              IN BOOL FreeOnFailure);

ULONG_PTR FASTCALL UserGetCPD(PVOID,GETCPD,ULONG_PTR);

/* EOF */
