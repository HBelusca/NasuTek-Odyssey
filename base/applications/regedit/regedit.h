#ifndef _REGEDIT_H
#define _REGEDIT_H

#define WIN32_LEAN_AND_MEAN     /* Exclude rarely-used stuff from Windows headers */
#define WIN32_NO_STATUS
#include <windows.h>

#define NTOS_MODE_USER
#include <ndk/cmtypes.h>

#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <tchar.h>
#include <process.h>
#include <stdio.h>
#include <limits.h>
#include <accctrl.h>
#include <unknwn.h>
#include <aclapi.h>
#include <commdlg.h>
#include <cderr.h>
#include <ole2.h>
#include <objbase.h>
#include <objsel.h>
#include <assert.h>
#include "main.h"
#include "regproc.h"
#include "hexedit.h"
#include "security.h"
#endif
