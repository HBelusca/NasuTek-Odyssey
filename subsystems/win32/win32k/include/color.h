#pragma once

#ifndef CLR_INVALID
#define CLR_INVALID         0xffffffff
#endif
#define PC_SYS_USED     0x80		/* palentry is used (both system and logical) */
#define PC_SYS_RESERVED 0x40		/* system palentry is not to be mapped to */
#define PC_SYS_MAPPED   0x10		/* logical palentry is a direct alias for system palentry */

#define NB_RESERVED_COLORS              20 /* number of fixed colors in system palette */

typedef struct _COLORSPACE
{
  BASEOBJECT  BaseObject;
  LOGCOLORSPACEW lcsColorSpace;
  DWORD dwFlags;
} COLORSPACE, *PCOLORSPACE;


#define  COLORSPACEOBJ_AllocCS() ((PCOLORSPACE) GDIOBJ_AllocObj(GDIObjType_ICMLCS_TYPE))
#define  COLORSPACEOBJ_AllocCSWithHandle() ((PCOLORSPACE) GDIOBJ_AllocObjWithHandle(GDI_OBJECT_TYPE_COLORSPACE, sizeof(COLORSPACE)))
#define  COLORSPACEOBJ_FreeCS(pCS) GDIOBJ_FreeObj((POBJ)pCS, GDIObjType_ICMLCS_TYPE)
#define  COLORSPACEOBJ_FreeCSByHandle(hCS) GDIOBJ_FreeObjByHandle((HGDIOBJ)hCS, GDI_OBJECT_TYPE_COLORSPACE)
#define  COLORSPACEOBJ_LockCS(hCS) ((PCOLORSPACE)GDIOBJ_LockObject((HGDIOBJ)hCS, GDIObjType_ICMLCS_TYPE))
#define  COLORSPACEOBJ_UnlockCS(pCS) GDIOBJ_vUnlockObject((POBJ)pCS)

typedef struct _COLORTRANSFORMOBJ
{
  BASEOBJECT BaseObject;
  HANDLE     hColorTransform;
} GDICLRXFORM, COLORTRANSFORMOBJ, *PCOLORTRANSFORMOBJ;

extern HCOLORSPACE hStockColorSpace;

UINT FASTCALL IntGdiRealizePalette (HDC);
