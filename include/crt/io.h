
/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */
#ifndef _IO_H_
#define _IO_H_

#include <crtdefs.h>
#include <string.h>

#pragma pack(push,_CRT_PACKING)

#ifndef _POSIX_

#ifdef __cplusplus
extern "C" {
#endif

_CRTIMP char* __cdecl _getcwd (char*, int);
#ifndef _FSIZE_T_DEFINED
  typedef unsigned long _fsize_t;
#define _FSIZE_T_DEFINED
#endif

#ifndef _FINDDATA_T_DEFINED

  struct _finddata_t {
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    _fsize_t size;
    char name[260];
  };

  struct _finddata32_t {
    unsigned attrib;
    __time32_t time_create;
    __time32_t time_access;
    __time32_t time_write;
    _fsize_t size;
    char name[260];
  };

#if _INTEGRAL_MAX_BITS >= 64

  struct _finddatai64_t {
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    __MINGW_EXTENSION __int64 size;
    char name[260];
  };

  struct _finddata32i64_t {
    unsigned attrib;
    __time32_t time_create;
    __time32_t time_access;
    __time32_t time_write;
    __MINGW_EXTENSION __int64 size;
    char name[260];
  };

  struct _finddata64i32_t {
    unsigned attrib;
    __time64_t time_create;
    __time64_t time_access;
    __time64_t time_write;
    _fsize_t size;
    char name[260];
  };

  struct __finddata64_t {
    unsigned attrib;
    __time64_t time_create;
    __time64_t time_access;
    __time64_t time_write;
    __MINGW_EXTENSION __int64 size;
    char name[260];
  };
#endif /* _INTEGRAL_MAX_BITS >= 64 */

#define _FINDDATA_T_DEFINED
#endif

#ifndef _WFINDDATA_T_DEFINED

  struct _wfinddata_t {
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    _fsize_t size;
    wchar_t name[260];
  };

  struct _wfinddata32_t {
    unsigned attrib;
    __time32_t time_create;
    __time32_t time_access;
    __time32_t time_write;
    _fsize_t size;
    wchar_t name[260];
  };

#if _INTEGRAL_MAX_BITS >= 64

  struct _wfinddatai64_t {
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    __MINGW_EXTENSION __int64 size;
    wchar_t name[260];
  };

  struct _wfinddata32i64_t {
    unsigned attrib;
    __time32_t time_create;
    __time32_t time_access;
    __time32_t time_write;
    __MINGW_EXTENSION __int64 size;
    wchar_t name[260];
  };

  struct _wfinddata64i32_t {
    unsigned attrib;
    __time64_t time_create;
    __time64_t time_access;
    __time64_t time_write;
    _fsize_t size;
    wchar_t name[260];
  };

  struct _wfinddata64_t {
    unsigned attrib;
    __time64_t time_create;
    __time64_t time_access;
    __time64_t time_write;
    __MINGW_EXTENSION __int64 size;
    wchar_t name[260];
  };
#endif

#define _WFINDDATA_T_DEFINED
#endif

#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH 0x20

  /* Some defines for _access nAccessMode (MS doesn't define them, but
  * it doesn't seem to hurt to add them). */
#define	F_OK	0	/* Check for file existence */
#define	X_OK	1	/* Check for execute permission. */
#define	W_OK	2	/* Check for write permission */
#define	R_OK	4	/* Check for read permission */

  _CRTIMP int __cdecl _access(const char *_Filename,int _AccessMode);
  _CRTIMP int __cdecl _chmod(const char *_Filename,int _Mode);
  _CRTIMP int __cdecl _chsize(int _FileHandle,long _Size);
  _CRTIMP int __cdecl _close(int _FileHandle);
  _CRTIMP int __cdecl _commit(int _FileHandle);
  _CRTIMP int __cdecl _creat(const char *_Filename,int _PermissionMode);
  _CRTIMP int __cdecl _dup(int _FileHandle);
  _CRTIMP int __cdecl _dup2(int _FileHandleSrc,int _FileHandleDst);
  _CRTIMP int __cdecl _eof(int _FileHandle);
  _CRTIMP long __cdecl _filelength(int _FileHandle);
  _CRTIMP intptr_t __cdecl _findfirst(const char *_Filename, struct _finddata_t *_FindData);
  _CRTIMP intptr_t __cdecl _findfirst32(const char *_Filename,struct _finddata32_t *_FindData);
  _CRTIMP int __cdecl _findnext(intptr_t _FindHandle,struct _finddata_t *_FindData);
  _CRTIMP int __cdecl _findnext32(intptr_t _FindHandle,struct _finddata32_t *_FindData);
  _CRTIMP int __cdecl _findclose(intptr_t _FindHandle);
  _CRTIMP int __cdecl _isatty(int _FileHandle);
  _CRTIMP int __cdecl _locking(int _FileHandle,int _LockMode,long _NumOfBytes);
  _CRTIMP long __cdecl _lseek(int _FileHandle,long _Offset,int _Origin);
  _CRTIMP char *__cdecl _mktemp(char *_TemplateName);
  _CRTIMP int __cdecl _pipe(int *_PtHandles,unsigned int _PipeSize,int _TextMode);
  _CRTIMP int __cdecl _read(int _FileHandle,void *_DstBuf,unsigned int _MaxCharCount);

#ifndef _CRT_DIRECTORY_DEFINED
#define _CRT_DIRECTORY_DEFINED
  int __cdecl remove(const char *_Filename);
  int __cdecl rename(const char *_OldFilename,const char *_NewFilename);
  _CRTIMP int __cdecl _unlink(const char *_Filename);
#ifndef	NO_OLDNAMES
  _CRTIMP int __cdecl unlink(const char *_Filename);
#endif
#endif

  _CRTIMP int __cdecl _setmode(int _FileHandle,int _Mode);
  _CRTIMP long __cdecl _tell(int _FileHandle);
  _CRTIMP int __cdecl _umask(int _Mode);
  _CRTIMP int __cdecl _write(int _FileHandle,const void *_Buf,unsigned int _MaxCharCount);

#if _INTEGRAL_MAX_BITS >= 64
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _filelengthi64(int _FileHandle);
  _CRTIMP intptr_t __cdecl _findfirst32i64(const char *_Filename,struct _finddata32i64_t *_FindData);
  _CRTIMP intptr_t __cdecl _findfirst64i32(const char *_Filename,struct _finddata64i32_t *_FindData);
  _CRTIMP intptr_t __cdecl _findfirst64(const char *_Filename,struct __finddata64_t *_FindData);
  _CRTIMP int __cdecl _findnext32i64(intptr_t _FindHandle,struct _finddata32i64_t *_FindData);
  _CRTIMP int __cdecl _findnext64i32(intptr_t _FindHandle,struct _finddata64i32_t *_FindData);
  _CRTIMP int __cdecl _findnext64(intptr_t _FindHandle,struct __finddata64_t *_FindData);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _lseeki64(int _FileHandle,__int64 _Offset,int _Origin);
  __MINGW_EXTENSION _CRTIMP __int64 __cdecl _telli64(int _FileHandle);
#ifdef __cplusplus
#include <string.h>
#endif
  __CRT_INLINE intptr_t __cdecl _findfirst64i32(const char *_Filename,struct _finddata64i32_t *_FindData)
  {
    struct __finddata64_t fd;
    intptr_t ret = _findfirst64(_Filename,&fd);
    _FindData->attrib=fd.attrib;
    _FindData->time_create=fd.time_create;
    _FindData->time_access=fd.time_access;
    _FindData->time_write=fd.time_write;
    _FindData->size=(_fsize_t) fd.size;
    strncpy(_FindData->name,fd.name,260);
    return ret;
  }
  __CRT_INLINE int __cdecl _findnext64i32(intptr_t _FindHandle,struct _finddata64i32_t *_FindData)
  {
    struct __finddata64_t fd;
    int ret = _findnext64(_FindHandle,&fd);
    _FindData->attrib=fd.attrib;
    _FindData->time_create=fd.time_create;
    _FindData->time_access=fd.time_access;
    _FindData->time_write=fd.time_write;
    _FindData->size=(_fsize_t) fd.size;
    strncpy(_FindData->name,fd.name,260);
    return ret;
  }
#endif

#ifndef NO_OLDNAMES
#ifndef _UWIN
  _CRTIMP int __cdecl chdir (const char *);
  _CRTIMP char *__cdecl getcwd (char *, int);
  _CRTIMP int __cdecl mkdir (const char *);
  _CRTIMP char *__cdecl mktemp(char *);
  _CRTIMP int __cdecl rmdir (const char*);
  _CRTIMP int __cdecl chmod (const char *, int);
#endif /* _UWIN */
#endif /* Not NO_OLDNAMES */

  _CRTIMP errno_t __cdecl _sopen_s(int *_FileHandle,const char *_Filename,int _OpenFlag,int _ShareFlag,int _PermissionMode);

#ifndef __cplusplus
  _CRTIMP int __cdecl _open(const char *_Filename,int _OpenFlag,...);
  _CRTIMP int __cdecl _sopen(const char *_Filename,int _OpenFlag,int _ShareFlag,...);
#else
  extern "C++" _CRTIMP int __cdecl _open(const char *_Filename,int _Openflag,int _PermissionMode = 0);
  extern "C++" _CRTIMP int __cdecl _sopen(const char *_Filename,int _Openflag,int _ShareFlag,int _PermissionMode = 0);
#endif

#ifndef _WIO_DEFINED
#define _WIO_DEFINED
  _CRTIMP int __cdecl _waccess(const wchar_t *_Filename,int _AccessMode);
  _CRTIMP int __cdecl _wchmod(const wchar_t *_Filename,int _Mode);
  _CRTIMP int __cdecl _wcreat(const wchar_t *_Filename,int _PermissionMode);
  _CRTIMP intptr_t __cdecl _wfindfirst32(const wchar_t *_Filename,struct _wfinddata32_t *_FindData);
  _CRTIMP int __cdecl _wfindnext32(intptr_t _FindHandle,struct _wfinddata32_t *_FindData);
  _CRTIMP int __cdecl _wunlink(const wchar_t *_Filename);
  _CRTIMP int __cdecl _wrename(const wchar_t *_NewFilename,const wchar_t *_OldFilename);
  _CRTIMP wchar_t *__cdecl _wmktemp(wchar_t *_TemplateName);

#if _INTEGRAL_MAX_BITS >= 64
  _CRTIMP intptr_t __cdecl _wfindfirst32i64(const wchar_t *_Filename,struct _wfinddata32i64_t *_FindData);
  intptr_t __cdecl _wfindfirst64i32(const wchar_t *_Filename,struct _wfinddata64i32_t *_FindData);
  _CRTIMP intptr_t __cdecl _wfindfirst64(const wchar_t *_Filename,struct _wfinddata64_t *_FindData);
  _CRTIMP int __cdecl _wfindnext32i64(intptr_t _FindHandle,struct _wfinddata32i64_t *_FindData);
  int __cdecl _wfindnext64i32(intptr_t _FindHandle,struct _wfinddata64i32_t *_FindData);
  _CRTIMP int __cdecl _wfindnext64(intptr_t _FindHandle,struct _wfinddata64_t *_FindData);
#endif

  _CRTIMP errno_t __cdecl _wsopen_s(int *_FileHandle,const wchar_t *_Filename,int _OpenFlag,int _ShareFlag,int _PermissionFlag);

#if !defined(__cplusplus) || !(defined(_X86_) && !defined(__x86_64))
  _CRTIMP int __cdecl _wopen(const wchar_t *_Filename,int _OpenFlag,...);
  _CRTIMP int __cdecl _wsopen(const wchar_t *_Filename,int _OpenFlag,int _ShareFlag,...);
#else
  extern "C++" _CRTIMP int __cdecl _wopen(const wchar_t *_Filename,int _OpenFlag,int _PermissionMode = 0);
  extern "C++" _CRTIMP int __cdecl _wsopen(const wchar_t *_Filename,int _OpenFlag,int _ShareFlag,int _PermissionMode = 0);
#endif

#endif /* !_WIO_DEFINED */

  int __cdecl __lock_fhandle(int _Filehandle);
  void __cdecl _unlock_fhandle(int _Filehandle);
  _CRTIMP intptr_t __cdecl _get_osfhandle(int _FileHandle);
  _CRTIMP int __cdecl _open_osfhandle(intptr_t _OSFileHandle,int _Flags);

#ifndef	NO_OLDNAMES
  _CRTIMP int __cdecl access(const char *_Filename,int _AccessMode);
  _CRTIMP int __cdecl chmod(const char *_Filename,int _AccessMode);
  _CRTIMP int __cdecl chsize(int _FileHandle,long _Size);
  _CRTIMP int __cdecl close(int _FileHandle);
  _CRTIMP int __cdecl creat(const char *_Filename,int _PermissionMode);
  _CRTIMP int __cdecl dup(int _FileHandle);
  _CRTIMP int __cdecl dup2(int _FileHandleSrc,int _FileHandleDst);
  _CRTIMP int __cdecl eof(int _FileHandle);
  _CRTIMP long __cdecl filelength(int _FileHandle);
  _CRTIMP int __cdecl isatty(int _FileHandle);
  _CRTIMP int __cdecl locking(int _FileHandle,int _LockMode,long _NumOfBytes);
  _CRTIMP long __cdecl lseek(int _FileHandle,long _Offset,int _Origin);
  _CRTIMP char *__cdecl mktemp(char *_TemplateName);
  _CRTIMP int __cdecl open(const char *_Filename,int _OpenFlag,...);
  _CRTIMP int __cdecl read(int _FileHandle,void *_DstBuf,unsigned int _MaxCharCount);
  _CRTIMP int __cdecl setmode(int _FileHandle,int _Mode);
  _CRTIMP int __cdecl sopen(const char *_Filename,int _OpenFlag,int _ShareFlag,...);
  _CRTIMP long __cdecl tell(int _FileHandle);
  _CRTIMP int __cdecl umask(int _Mode);
  _CRTIMP int __cdecl write(int _Filehandle,const void *_Buf,unsigned int _MaxCharCount);
#endif

#ifdef __cplusplus
}
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Misc stuff */
char *getlogin(void);
#ifdef __USE_MINGW_ALARM
unsigned int alarm(unsigned int seconds);
#endif

#ifdef __USE_MINGW_ACCESS
/*  Old versions of MSVCRT access() just ignored X_OK, while the version
    shipped with Vista, returns an error code.  This will restore the
    old behaviour  */
static inline int __mingw_access (const char *__fname, int __mode) {
  return  _access (__fname, __mode & ~X_OK);
}

#define access(__f,__m)  __mingw_access (__f, __m)
#endif


#ifdef __cplusplus
}
#endif


#pragma pack(pop)

#include <sec_api/io_s.h>

#endif /* End _IO_H_ */

