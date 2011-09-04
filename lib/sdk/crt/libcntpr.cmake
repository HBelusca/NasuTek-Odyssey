
list(APPEND LIBCNTPR_SOURCE
    float/isnan.c
    math/abs.c
    math/div.c
    math/labs.c
    math/rand_nt.c
    mbstring/mbstrlen.c
    mem/memccpy.c
    mem/memcmp.c
    mem/memicmp.c
    printf/_snprintf.c
    printf/_snwprintf.c
    printf/_vcprintf.c
    printf/_vsnprintf.c
    printf/_vsnwprintf.c
    printf/sprintf.c
    printf/streamout.c
    printf/swprintf.c
    printf/vprintf.c
    printf/vsprintf.c
    printf/vswprintf.c
    printf/wstreamout.c
    search/bsearch.c
    search/lfind.c
    stdlib/qsort.c
    string/ctype.c
    string/scanf.c
    string/strcspn.c
    string/stricmp.c
    string/strnicmp.c
    string/strlwr.c
    string/strrev.c
    string/strset.c
    string/strstr.c
    string/strupr.c
    string/strpbrk.c
    string/strspn.c
    string/atoi64.c
    string/atoi.c
    string/atol.c
    string/itoa.c
    string/itow.c
    string/mbstowcs_nt.c
    string/splitp.c
    string/strtol.c
    string/strtoul.c
    string/strtoull.c
    string/wcs.c
    string/wcstol.c
    string/wcstombs_nt.c
    string/wcstoul.c
    string/wsplitp.c
    string/wtoi64.c
    string/wtoi.c
    string/wtol.c
    wstring/wcsicmp.c
    wstring/wcslwr.c
    wstring/wcsnicmp.c
    wstring/wcsupr.c
    wstring/wcscspn.c
    wstring/wcsspn.c
    wstring/wcsstr.c)

if(ARCH MATCHES i386)
    list(APPEND LIBCNTPR_SOURCE
        except/i386/chkstk_asm.s
        except/i386/seh.s
        except/i386/seh_prolog.s
        setjmp/i386/setjmp.s
        math/i386/alldiv_asm.s
        math/i386/alldvrm_asm.s
        math/i386/allmul_asm.s
        math/i386/allrem_asm.s
        math/i386/allshl_asm.s
        math/i386/allshr_asm.s
        math/i386/atan_asm.s
        math/i386/aulldiv_asm.s
        math/i386/aulldvrm_asm.s
        math/i386/aullrem_asm.s
        math/i386/aullshr_asm.s
        math/i386/ceil_asm.s
        math/i386/cos_asm.s
        math/i386/fabs_asm.s
        math/i386/floor_asm.s
        math/i386/ftol_asm.s
        math/i386/ftol2_asm.s
        math/i386/log_asm.s
        math/i386/log10_asm.s
        math/i386/pow_asm.s
        math/i386/sin_asm.s
        math/i386/sqrt_asm.s
        math/i386/tan_asm.s
        math/i386/ci.c
        misc/i386/readcr4.S)
    if(NOT MSVC)
        list(APPEND LIBCNTPR_SOURCE except/i386/chkstk_ms.s)
    endif()
elseif(ARCH MATCHES amd64)
    list(APPEND LIBCNTPR_SOURCE
        except/amd64/ehandler.c
        except/amd64/chkstk_asm.s
        except/amd64/seh.s
        setjmp/amd64/setjmp.s
        math/cos.c
        math/sin.c
        math/amd64/alldiv.S
        math/amd64/atan.S
        math/amd64/atan2.S
        math/amd64/ceil.S
        math/amd64/exp.S
        math/amd64/fabs.S
        math/amd64/floor.S
        math/amd64/fmod.S
        math/amd64/ldexp.S
        math/amd64/log.S
        math/amd64/log10.S
        math/amd64/pow.S
        math/amd64/sqrt.S
        math/amd64/tan.S)
endif()

if(ARCH MATCHES i386)
    list(APPEND LIBCNTPR_SOURCE
        mem/i386/memchr_asm.s
        mem/i386/memmove_asm.s
        mem/i386/memset_asm.s
        string/i386/strcat_asm.s
        string/i386/strchr_asm.s
        string/i386/strcmp_asm.s
        string/i386/strcpy_asm.s
        string/i386/strlen_asm.s
        string/i386/strncat_asm.s
        string/i386/strncmp_asm.s
        string/i386/strncpy_asm.s
        string/i386/strnlen_asm.s
        string/i386/strrchr_asm.s
        string/i386/wcscat_asm.s
        string/i386/wcschr_asm.s
        string/i386/wcscmp_asm.s
        string/i386/wcscpy_asm.s
        string/i386/wcslen_asm.s
        string/i386/wcsncat_asm.s
        string/i386/wcsncmp_asm.s
        string/i386/wcsncpy_asm.s
        string/i386/wcsnlen_asm.s
        string/i386/wcsrchr_asm.s)
else()
    list(APPEND LIBCNTPR_SOURCE
        mem/memchr.c
        mem/memcpy.c
        mem/memmove.c
        mem/memset.c
        string/strcat.c
        string/strchr.c
        string/strcmp.c
        string/strcpy.c
        string/strlen.c
        string/strncat.c
        string/strncmp.c
        string/strncpy.c
        string/strnlen.c
        string/strrchr.c
        string/wcscat.c
        string/wcschr.c
        string/wcscmp.c
        string/wcscpy.c
        string/wcslen.c
        string/wcsncat.c
        string/wcsncmp.c
        string/wcsncpy.c
        string/wcsnlen.c
        string/wcsrchr.c)
endif()

add_library(libcntpr ${LIBCNTPR_SOURCE})
set_property(TARGET libcntpr PROPERTY COMPILE_DEFINITIONS NO_RTL_INLINES _NTSYSTEM_ _NTDLLBUILD_ _LIBCNT_ __CRT__NO_INLINE)
add_dependencies(libcntpr psdk asm)
