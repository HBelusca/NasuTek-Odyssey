/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
#ifndef _INC_STDLIB
#define _INC_STDLIB

#include <crtdefs.h>
#include <limits.h>

#pragma pack(push,_CRT_PACKING)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#ifdef __cplusplus
#ifndef _WIN64
#define NULL 0
#else
#define NULL 0LL
#endif  /* W64 */
#else
#define NULL ((void *)0)
#endif
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifndef _ONEXIT_T_DEFINED
#define _ONEXIT_T_DEFINED

  typedef int (__cdecl *_onexit_t)(void);

#ifndef	NO_OLDNAMES
#define onexit_t _onexit_t
#endif
#endif

#ifndef _DIV_T_DEFINED
#define _DIV_T_DEFINED

  typedef struct _div_t {
    int quot;
    int rem;
  } div_t;

  typedef struct _ldiv_t {
    long quot;
    long rem;
  } ldiv_t;
#endif

#ifndef _CRT_DOUBLE_DEC
#define _CRT_DOUBLE_DEC

#pragma pack(4)
  typedef struct {
    unsigned char ld[10];
  } _LDOUBLE;
#pragma pack()

#define _PTR_LD(x) ((unsigned char *)(&(x)->ld))

  typedef struct {
    double x;
  } _CRT_DOUBLE;

  typedef struct {
    float f;
  } _CRT_FLOAT;
#if __MINGW_GNUC_PREREQ(4,4)
#pragma push_macro("long")
#undef long
#endif

  typedef struct {
    long double x;
  } _LONGDOUBLE;

#if __MINGW_GNUC_PREREQ(4,4)
#pragma pop_macro("long")
#endif

#pragma pack(4)
  typedef struct {
    unsigned char ld12[12];
  } _LDBL12;
#pragma pack()
#endif

#define RAND_MAX 0x7fff

#ifndef MB_CUR_MAX
#define MB_CUR_MAX ___mb_cur_max_func()
#ifdef _M_CEE_PURE
  _CRTIMP int* __cdecl __p___mb_cur_max();
  #define __mb_cur_max (*__p___mb_cur_max())
#else /* !_M_CEE_PURE */
  _CRTIMP extern int __mb_cur_max;
#endif /* !_M_CEE_PURE */
  _CRTIMP int __cdecl ___mb_cur_max_func(void);
  _CRTIMP int __cdecl ___mb_cur_max_l_func(_locale_t);
#endif /* !MB_CUR_MAX */

#define __max(a,b) (((a) > (b)) ? (a) : (b))
#define __min(a,b) (((a) < (b)) ? (a) : (b))

#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256

#define _OUT_TO_DEFAULT 0
#define _OUT_TO_STDERR 1
#define _OUT_TO_MSGBOX 2
#define _REPORT_ERRMODE 3

#define _WRITE_ABORT_MSG 0x1
#define _CALL_REPORTFAULT 0x2

#define _MAX_ENV 32767

  typedef void (__cdecl *_purecall_handler)(void);

  _CRTIMP _purecall_handler __cdecl _set_purecall_handler(_purecall_handler _Handler);
  _CRTIMP _purecall_handler __cdecl _get_purecall_handler(void);

  typedef void (__cdecl *_invalid_parameter_handler)(const wchar_t *,const wchar_t *,const wchar_t *,unsigned int,uintptr_t);
  _invalid_parameter_handler __cdecl _set_invalid_parameter_handler(_invalid_parameter_handler _Handler);
  _invalid_parameter_handler __cdecl _get_invalid_parameter_handler(void);

#include <errno.h>
  _CRTIMP unsigned long *__cdecl __doserrno(void);
#define _doserrno (*__doserrno())
  errno_t __cdecl _set_doserrno(unsigned long _Value);
  errno_t __cdecl _get_doserrno(unsigned long *_Value);

  _CRTIMP extern char *_sys_errlist[];
  _CRTIMP extern int _sys_nerr;

#if defined(_DLL) && defined(_M_IX86)
  _CRTIMP int *__cdecl __p___argc(void);
  _CRTIMP char ***__cdecl __p___argv(void);
  _CRTIMP wchar_t ***__cdecl __p___wargv(void);
  _CRTIMP char ***__cdecl __p__environ(void);
  _CRTIMP wchar_t ***__cdecl __p__wenviron(void);
  _CRTIMP char **__cdecl __p__pgmptr(void);
  _CRTIMP wchar_t **__cdecl __p__wpgmptr(void);
#endif

// FIXME: move inside _M_CEE_PURE section
  _CRTIMP int *__cdecl __p___argc();
  _CRTIMP char ***__cdecl __p___argv();
  _CRTIMP wchar_t ***__cdecl __p___wargv();
  _CRTIMP char ***__cdecl __p__environ();
  _CRTIMP wchar_t ***__cdecl __p__wenviron();
  _CRTIMP char **__cdecl __p__pgmptr();
  _CRTIMP wchar_t **__cdecl __p__wpgmptr();

#ifdef _M_CEE_PURE
  #define __argv (*__p___argv())
  #define __argc (*__p___argc())
  #define __wargv (*__p___wargv())
  #define _environ   (*__p__environ())
  #define _wenviron  (*__p__wenviron())
  #define _pgmptr    (*__p__pgmptr())
  #define _wpgmptr   (*__p__wpgmptr())
#else /* !_M_CEE_PURE */
  _CRTIMP extern int __argc;
  _CRTIMP extern char **__argv;
  _CRTIMP extern wchar_t **__wargv;
  _CRTIMP extern char **_environ;
  _CRTIMP extern wchar_t **_wenviron;
  _CRTIMP extern char *_pgmptr;
  _CRTIMP extern wchar_t *_wpgmptr;
#endif /* !_M_CEE_PURE */

  _CRTIMP errno_t __cdecl _get_environ(char***);
  _CRTIMP errno_t __cdecl _get_wenviron(wchar_t***);
  _CRTIMP errno_t __cdecl _get_pgmptr(char **_Value);
  _CRTIMP errno_t __cdecl _get_wpgmptr(wchar_t **_Value);

#ifdef _M_CEE_PURE
  _CRTIMP int* __cdecl __p__fmode();
  #define _fmode (*__p__fmode())
#else
  _CRTIMP extern int _fmode;
#endif /* !_M_CEE_PURE */
  _CRTIMP errno_t __cdecl _set_fmode(int _Mode);
  _CRTIMP errno_t __cdecl _get_fmode(int *_PMode);

#ifdef _M_CEE_PURE
  _CRTIMP unsigned int* __cdecl __p__osplatform();
  _CRTIMP unsigned int* __cdecl __p__osver();
  _CRTIMP unsigned int* __cdecl __p__winver();
  _CRTIMP unsigned int* __cdecl __p__winmajor();
  _CRTIMP unsigned int* __cdecl __p__winminor();
#define _osplatform  (*__p__osplatform())
#define _osver       (*__p__osver())
#define _winver      (*__p__winver())
#define _winmajor    (*__p__winmajor())
#define _winminor    (*__p__winminor())
#else /* !_M_CEE_PURE */
  _CRTIMP extern unsigned int _osplatform;
  _CRTIMP extern unsigned int _osver;
  _CRTIMP extern unsigned int _winver;
  _CRTIMP extern unsigned int _winmajor;
  _CRTIMP extern unsigned int _winminor;
#endif /* !_M_CEE_PURE */

  errno_t __cdecl _get_osplatform(unsigned int *_Value);
  errno_t __cdecl _get_osver(unsigned int *_Value);
  errno_t __cdecl _get_winver(unsigned int *_Value);
  errno_t __cdecl _get_winmajor(unsigned int *_Value);
  errno_t __cdecl _get_winminor(unsigned int *_Value);

#ifndef _countof
#ifndef __cplusplus
#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#else
  extern "C++" {
    template <typename _CountofType,size_t _SizeOfArray> 
       char (*__countof_helper(/*UNALIGNED*/ _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) sizeof(*__countof_helper(_Array))
  }
#endif
#endif

#ifndef _CRT_TERMINATE_DEFINED
#define _CRT_TERMINATE_DEFINED
  __declspec(noreturn) void __cdecl exit(int _Code);
  _CRTIMP __declspec(noreturn) void __cdecl _exit(int _Code);
#if !defined __NO_ISOCEXT /* extern stub in static libmingwex.a */
  /* C99 function name */
  __declspec(noreturn) void __cdecl _Exit(int); /* Declare to get noreturn attribute.  */
  __CRT_INLINE void __cdecl _Exit(int status)
  {  _exit(status); }
#endif
#if __MINGW_GNUC_PREREQ(4,4)
#pragma push_macro("abort")
#undef abort
#endif
  __declspec(noreturn) void __cdecl abort(void);
#if __MINGW_GNUC_PREREQ(4,4)
#pragma pop_macro("abort")
#endif
#endif

  _CRTIMP unsigned int __cdecl _set_abort_behavior(unsigned int _Flags,unsigned int _Mask);

#ifndef _CRT_ABS_DEFINED
#define _CRT_ABS_DEFINED
  int __cdecl abs(int _X);
  long __cdecl labs(long _X);
#endif

#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION __int64 __cdecl _abs64(__int64);
#endif
  int __cdecl atexit(void (__cdecl *)(void));
#ifndef _CRT_ATOF_DEFINED
#define _CRT_ATOF_DEFINED
  double __cdecl atof(const char *_String);
  double __cdecl _atof_l(const char *_String,_locale_t _Locale);
#endif
  int __cdecl atoi(const char *_Str);
  _CRTIMP int __cdecl _atoi_l(const char *_Str,_locale_t _Locale);
  long __cdecl atol(const char *_Str);
  _CRTIMP long __cdecl _atol_l(const char *_Str,_locale_t _Locale);
#ifndef _CRT_ALGO_DEFINED
#define _CRT_ALGO_DEFINED
  void *__cdecl bsearch(const void *_Key,const void *_Base,size_t _NumOfElements,size_t _SizeOfElements,int (__cdecl *_PtFuncCompare)(const void *,const void *));
  void __cdecl qsort(void *_Base,size_t _NumOfElements,size_t _SizeOfElements,int (__cdecl *_PtFuncCompare)(const void *,const void *));
#endif

#if !defined(__GNUC__) && !defined(__clang)
  unsigned short __cdecl _byteswap_ushort(unsigned short _Short);
  unsigned long __cdecl _byteswap_ulong (unsigned long _Long);
#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION unsigned __int64 __cdecl _byteswap_uint64(unsigned __int64 _Int64);
#endif
#endif

  div_t __cdecl div(int _Numerator,int _Denominator);
  char *__cdecl getenv(const char *_VarName);
  _CRTIMP char *__cdecl _itoa(int _Value,char *_Dest,int _Radix);
#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION _CRTIMP char *__cdecl _i64toa(__int64 _Val,char *_DstBuf,int _Radix);
  __MINGW_EXTENSION _CRTIMP char *__cdecl _ui64toa(unsigned __int64 _Val,char *_DstBuf,int _Radix);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _atoi64(const char *_String);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _atoi64_l(const char *_String,_locale_t _Locale);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _strtoi64(const char *_String,char **_EndPtr,int _Radix);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _strtoi64_l(const char *_String,char **_EndPtr,int _Radix,_locale_t _Locale);
  __MINGW_EXTENSION _CRTIMP unsigned __int64 __cdecl _strtoui64(const char *_String,char **_EndPtr,int _Radix);
  __MINGW_EXTENSION _CRTIMP unsigned __int64 __cdecl _strtoui64_l(const char *_String,char **_EndPtr,int _Radix,_locale_t _Locale);
#endif
  ldiv_t __cdecl ldiv(long _Numerator,long _Denominator);
  _CRTIMP char *__cdecl _ltoa(long _Value,char *_Dest,int _Radix);
  int __cdecl mblen(const char *_Ch,size_t _MaxCount);
  _CRTIMP int __cdecl _mblen_l(const char *_Ch,size_t _MaxCount,_locale_t _Locale);
  _CRTIMP size_t __cdecl _mbstrlen(const char *_Str);
  _CRTIMP size_t __cdecl _mbstrlen_l(const char *_Str,_locale_t _Locale);
  _CRTIMP size_t __cdecl _mbstrnlen(const char *_Str,size_t _MaxCount);
  _CRTIMP size_t __cdecl _mbstrnlen_l(const char *_Str,size_t _MaxCount,_locale_t _Locale);
  int __cdecl mbtowc(wchar_t *_DstCh,const char *_SrcCh,size_t _SrcSizeInBytes);
  _CRTIMP int __cdecl _mbtowc_l(wchar_t *_DstCh,const char *_SrcCh,size_t _SrcSizeInBytes,_locale_t _Locale);
  size_t __cdecl mbstowcs(wchar_t *_Dest,const char *_Source,size_t _MaxCount);
  _CRTIMP size_t __cdecl _mbstowcs_l(wchar_t *_Dest,const char *_Source,size_t _MaxCount,_locale_t _Locale);
  int __cdecl rand(void);
  _CRTIMP int __cdecl _set_error_mode(int _Mode);
  void __cdecl srand(unsigned int _Seed);
  double __cdecl strtod(const char *_Str,char **_EndPtr);
  float __cdecl strtof(const char *nptr, char **endptr);
#if !defined __NO_ISOCEXT  /* in libmingwex.a */
  float __cdecl strtof (const char * __restrict__, char ** __restrict__);
  long double __cdecl strtold(const char * __restrict__, char ** __restrict__);
#endif /* __NO_ISOCEXT */
  _CRTIMP double __cdecl _strtod_l(const char *_Str,char **_EndPtr,_locale_t _Locale);
  long __cdecl strtol(const char *_Str,char **_EndPtr,int _Radix);
  _CRTIMP long __cdecl _strtol_l(const char *_Str,char **_EndPtr,int _Radix,_locale_t _Locale);
  unsigned long __cdecl strtoul(const char *_Str,char **_EndPtr,int _Radix);
  _CRTIMP unsigned long __cdecl _strtoul_l(const char *_Str,char **_EndPtr,int _Radix,_locale_t _Locale);
#ifndef _CRT_SYSTEM_DEFINED
#define _CRT_SYSTEM_DEFINED
  int __cdecl system(const char *_Command);
#endif
  _CRTIMP char *__cdecl _ultoa(unsigned long _Value,char *_Dest,int _Radix);
  int __cdecl wctomb(char *_MbCh,wchar_t _WCh);
  _CRTIMP int __cdecl _wctomb_l(char *_MbCh,wchar_t _WCh,_locale_t _Locale);
  size_t __cdecl wcstombs(char *_Dest,const wchar_t *_Source,size_t _MaxCount);
  _CRTIMP size_t __cdecl _wcstombs_l(char *_Dest,const wchar_t *_Source,size_t _MaxCount,_locale_t _Locale);

#ifndef _CRT_ALLOCATION_DEFINED
#define _CRT_ALLOCATION_DEFINED
  void *__cdecl calloc(size_t _NumOfElements,size_t _SizeOfElements);
  void __cdecl free(void *_Memory);
  void *__cdecl malloc(size_t _Size);
  void *__cdecl realloc(void *_Memory,size_t _NewSize);
  _CRTIMP void *__cdecl _recalloc(void *_Memory,size_t _Count,size_t _Size);
  //_CRTIMP void __cdecl _aligned_free(void *_Memory);
  //_CRTIMP void *__cdecl _aligned_malloc(size_t _Size,size_t _Alignment);
  _CRTIMP void *__cdecl _aligned_offset_malloc(size_t _Size,size_t _Alignment,size_t _Offset);
  _CRTIMP void *__cdecl _aligned_realloc(void *_Memory,size_t _Size,size_t _Alignment);
  _CRTIMP void *__cdecl _aligned_recalloc(void *_Memory,size_t _Count,size_t _Size,size_t _Alignment);
  _CRTIMP void *__cdecl _aligned_offset_realloc(void *_Memory,size_t _Size,size_t _Alignment,size_t _Offset);
  _CRTIMP void *__cdecl _aligned_offset_recalloc(void *_Memory,size_t _Count,size_t _Size,size_t _Alignment,size_t _Offset);
#endif

#ifndef _WSTDLIB_DEFINED
#define _WSTDLIB_DEFINED

  _CRTIMP wchar_t *__cdecl _itow(int _Value,wchar_t *_Dest,int _Radix);
  _CRTIMP wchar_t *__cdecl _ltow(long _Value,wchar_t *_Dest,int _Radix);
  _CRTIMP wchar_t *__cdecl _ultow(unsigned long _Value,wchar_t *_Dest,int _Radix);
  double __cdecl wcstod(const wchar_t *_Str,wchar_t **_EndPtr);
  float __cdecl wcstof(const wchar_t *nptr, wchar_t **endptr);
#if !defined __NO_ISOCEXT /* in libmingwex.a */
  float __cdecl wcstof( const wchar_t * __restrict__, wchar_t ** __restrict__);
  long double __cdecl wcstold(const wchar_t * __restrict__, wchar_t ** __restrict__);
#endif /* __NO_ISOCEXT */
  _CRTIMP double __cdecl _wcstod_l(const wchar_t *_Str,wchar_t **_EndPtr,_locale_t _Locale);
  long __cdecl wcstol(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix);
  _CRTIMP long __cdecl _wcstol_l(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix,_locale_t _Locale);
  unsigned long __cdecl wcstoul(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix);
  _CRTIMP unsigned long __cdecl _wcstoul_l(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix,_locale_t _Locale);
  _CRTIMP wchar_t *__cdecl _wgetenv(const wchar_t *_VarName);
#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED
  _CRTIMP int __cdecl _wsystem(const wchar_t *_Command);
#endif
  _CRTIMP double __cdecl _wtof(const wchar_t *_Str);
  _CRTIMP double __cdecl _wtof_l(const wchar_t *_Str,_locale_t _Locale);
  _CRTIMP int __cdecl _wtoi(const wchar_t *_Str);
  _CRTIMP int __cdecl _wtoi_l(const wchar_t *_Str,_locale_t _Locale);
  _CRTIMP long __cdecl _wtol(const wchar_t *_Str);
  _CRTIMP long __cdecl _wtol_l(const wchar_t *_Str,_locale_t _Locale);

#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION _CRTIMP wchar_t *__cdecl _i64tow(__int64 _Val,wchar_t *_DstBuf,int _Radix);
  __MINGW_EXTENSION _CRTIMP wchar_t *__cdecl _ui64tow(unsigned __int64 _Val,wchar_t *_DstBuf,int _Radix);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _wtoi64(const wchar_t *_Str);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _wtoi64_l(const wchar_t *_Str,_locale_t _Locale);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _wcstoi64(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _wcstoi64_l(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix,_locale_t _Locale);
  __MINGW_EXTENSION _CRTIMP unsigned __int64 __cdecl _wcstoui64(const wchar_t *_Str,wchar_t **_EndPtr,int _Radix);
  __MINGW_EXTENSION _CRTIMP unsigned __int64 __cdecl _wcstoui64_l(const wchar_t *_Str ,wchar_t **_EndPtr,int _Radix,_locale_t _Locale);
#endif
#endif

#ifndef _POSIX_
#define _CVTBUFSIZE (309+40)
  _CRTIMP char *__cdecl _fullpath(char *_FullPath,const char *_Path,size_t _SizeInBytes);
  _CRTIMP char *__cdecl _ecvt(double _Val,int _NumOfDigits,int *_PtDec,int *_PtSign);
  _CRTIMP char *__cdecl _fcvt(double _Val,int _NumOfDec,int *_PtDec,int *_PtSign);
  _CRTIMP char *__cdecl _gcvt(double _Val,int _NumOfDigits,char *_DstBuf);
  _CRTIMP int __cdecl _atodbl(_CRT_DOUBLE *_Result,char *_Str);
  _CRTIMP int __cdecl _atoldbl(_LDOUBLE *_Result,char *_Str);
  _CRTIMP int __cdecl _atoflt(_CRT_FLOAT *_Result,char *_Str);
  _CRTIMP int __cdecl _atodbl_l(_CRT_DOUBLE *_Result,char *_Str,_locale_t _Locale);
  _CRTIMP int __cdecl _atoldbl_l(_LDOUBLE *_Result,char *_Str,_locale_t _Locale);
  _CRTIMP int __cdecl _atoflt_l(_CRT_FLOAT *_Result,char *_Str,_locale_t _Locale);
  unsigned long __cdecl _lrotl(unsigned long _Val,int _Shift);
  unsigned long __cdecl _lrotr(unsigned long _Val,int _Shift);
  _CRTIMP void __cdecl _makepath(char *_Path,const char *_Drive,const char *_Dir,const char *_Filename,const char *_Ext);
  _onexit_t __cdecl _onexit(_onexit_t _Func);

#ifndef _CRT_PERROR_DEFINED
#define _CRT_PERROR_DEFINED
  void __cdecl perror(const char *_ErrMsg);
#endif
  _CRTIMP int __cdecl _putenv(const char *_EnvString);
#if !defined(__GNUC__) && !defined(__clang)
  unsigned int __cdecl _rotl(unsigned int _Val,int _Shift);
#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION unsigned __int64 __cdecl _rotl64(unsigned __int64 _Val,int _Shift);
#endif
  unsigned int __cdecl _rotr(unsigned int _Val,int _Shift);
#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION unsigned __int64 __cdecl _rotr64(unsigned __int64 _Val,int _Shift);
#endif
#endif
  _CRTIMP void __cdecl _searchenv(const char *_Filename,const char *_EnvVar,char *_ResultPath);
  _CRTIMP void __cdecl _splitpath(const char *_FullPath,char *_Drive,char *_Dir,char *_Filename,char *_Ext);
  _CRTIMP void __cdecl _swab(char *_Buf1,char *_Buf2,int _SizeInBytes);

#ifndef _WSTDLIBP_DEFINED
#define _WSTDLIBP_DEFINED
  _CRTIMP wchar_t *__cdecl _wfullpath(wchar_t *_FullPath,const wchar_t *_Path,size_t _SizeInWords);
  _CRTIMP void __cdecl _wmakepath(wchar_t *_ResultPath,const wchar_t *_Drive,const wchar_t *_Dir,const wchar_t *_Filename,const wchar_t *_Ext);
#ifndef _CRT_WPERROR_DEFINED
#define _CRT_WPERROR_DEFINED
  _CRTIMP void __cdecl _wperror(const wchar_t *_ErrMsg);
#endif
  _CRTIMP int __cdecl _wputenv(const wchar_t *_EnvString);
  _CRTIMP void __cdecl _wsearchenv(const wchar_t *_Filename,const wchar_t *_EnvVar,wchar_t *_ResultPath);
  _CRTIMP void __cdecl _wsplitpath(const wchar_t *_FullPath,wchar_t *_Drive,wchar_t *_Dir,wchar_t *_Filename,wchar_t *_Ext);
#endif

  _CRTIMP __MINGW_ATTRIB_DEPRECATED void __cdecl _beep(unsigned _Frequency,unsigned _Duration);
  /* Not to be confused with  _set_error_mode (int).  */
  _CRTIMP __MINGW_ATTRIB_DEPRECATED void __cdecl _seterrormode(int _Mode);
  _CRTIMP __MINGW_ATTRIB_DEPRECATED void __cdecl _sleep(unsigned long _Duration);
#endif

#ifndef	NO_OLDNAMES
#ifndef _POSIX_
#if 0
#ifndef __cplusplus
#ifndef NOMINMAX
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#endif
#endif
#endif

#define sys_errlist _sys_errlist
#define sys_nerr _sys_nerr
#define environ _environ
  _CRTIMP char *__cdecl ecvt(double _Val,int _NumOfDigits,int *_PtDec,int *_PtSign);
  _CRTIMP char *__cdecl fcvt(double _Val,int _NumOfDec,int *_PtDec,int *_PtSign);
  _CRTIMP char *__cdecl gcvt(double _Val,int _NumOfDigits,char *_DstBuf);
  _CRTIMP char *__cdecl itoa(int _Val,char *_DstBuf,int _Radix);
  _CRTIMP char *__cdecl ltoa(long _Val,char *_DstBuf,int _Radix);
  _CRTIMP int __cdecl putenv(const char *_EnvString);
  _CRTIMP void __cdecl swab(char *_Buf1,char *_Buf2,int _SizeInBytes);
  _CRTIMP char *__cdecl ultoa(unsigned long _Val,char *_Dstbuf,int _Radix);
  onexit_t __cdecl onexit(onexit_t _Func);
#endif
#endif

#if !defined __NO_ISOCEXT /* externs in static libmingwex.a */

  __MINGW_EXTENSION typedef struct { long long quot, rem; } lldiv_t;

  __MINGW_EXTENSION lldiv_t __cdecl lldiv(long long, long long);

#ifndef _MSC_VER
  __MINGW_EXTENSION __CRT_INLINE long long __cdecl llabs(long long _j) { return (_j >= 0 ? _j : -_j); }
#endif

  __MINGW_EXTENSION long long  __cdecl strtoll(const char* __restrict__, char** __restrict, int);
  __MINGW_EXTENSION unsigned long long  __cdecl strtoull(const char* __restrict__, char** __restrict__, int);

  /* these are stubs for MS _i64 versions */
  __MINGW_EXTENSION long long  __cdecl atoll (const char *);

#ifndef __STRICT_ANSI__
  __MINGW_EXTENSION long long  __cdecl wtoll (const wchar_t *);
  __MINGW_EXTENSION char *__cdecl lltoa (long long, char *, int);
  __MINGW_EXTENSION char *__cdecl ulltoa (unsigned long long , char *, int);
  __MINGW_EXTENSION wchar_t *__cdecl lltow (long long, wchar_t *, int);
  __MINGW_EXTENSION wchar_t *__cdecl ulltow (unsigned long long, wchar_t *, int);

  /* __CRT_INLINE using non-ansi functions */
  __MINGW_EXTENSION __CRT_INLINE long long  __cdecl atoll (const char * _c) { return _atoi64 (_c); }
  __MINGW_EXTENSION __CRT_INLINE char *__cdecl lltoa (long long _n, char * _c, int _i) { return _i64toa (_n, _c, _i); }
  __MINGW_EXTENSION __CRT_INLINE char *__cdecl ulltoa (unsigned long long _n, char * _c, int _i) { return _ui64toa (_n, _c, _i); }
  __MINGW_EXTENSION __CRT_INLINE long long  __cdecl wtoll (const wchar_t * _w) { return _wtoi64 (_w); }
  __MINGW_EXTENSION __CRT_INLINE wchar_t *__cdecl lltow (long long _n, wchar_t * _w, int _i) { return _i64tow (_n, _w, _i); }
  __MINGW_EXTENSION __CRT_INLINE wchar_t *__cdecl ulltow (unsigned long long _n, wchar_t * _w, int _i) { return _ui64tow (_n, _w, _i); }
#endif /* (__STRICT_ANSI__)  */

#endif /* !__NO_ISOCEXT */

#ifdef __cplusplus
}
#endif

#pragma pack(pop)

#include <sec_api/stdlib_s.h>
#endif
