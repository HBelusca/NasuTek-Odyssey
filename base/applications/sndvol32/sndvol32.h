#ifndef __SNDVOL32_H
#define __SNDVOL32_H

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include "resources.h"
#include <assert.h>

typedef struct _MIXER_WINDOW
{
  HWND hWnd;
  HWND hStatusBar;
  struct _SND_MIXER *Mixer;
  UINT SelectedLine;
  UINT WindowCount;
  HWND * Window;


} MIXER_WINDOW, *PMIXER_WINDOW;

extern HINSTANCE hAppInstance;
extern ATOM MainWindowClass;
extern HWND hMainWnd;
extern HANDLE hAppHeap;

#define SZ_APP_CLASS TEXT("Volume Control")

ULONG DbgPrint(PCH , ...);
#define DPRINT DbgPrint("SNDVOL32: %s:%i: ", __FILE__, __LINE__); DbgPrint


/*
 * MIXER
 */

typedef struct _SND_MIXER_CONNECTION
{
  struct _SND_MIXER_CONNECTION *Next;
  MIXERLINE Info;
  LPMIXERCONTROL Controls;
  UINT DisplayControls;
} SND_MIXER_CONNECTION, *PSND_MIXER_CONNECTION;


typedef struct _SND_MIXER_DESTINATION
{
  struct _SND_MIXER_DESTINATION *Next;
  MIXERLINE Info;
  LPMIXERCONTROL Controls;
  UINT DisplayControls;
  PSND_MIXER_CONNECTION Connections;
} SND_MIXER_DESTINATION, *PSND_MIXER_DESTINATION;

typedef struct _SND_MIXER
{
  UINT MixersCount;
  HWND hWndNotification;
  UINT MixerId;
  HMIXER hmx;
  MIXERCAPS Caps;
  PSND_MIXER_DESTINATION Lines;
} SND_MIXER, *PSND_MIXER;

typedef struct _PREFERENCES_CONTEXT
{
    PMIXER_WINDOW MixerWindow;
    PSND_MIXER Mixer;
    HWND hwndDlg;

    UINT Selected;
    DWORD SelectedLine;
    DWORD PlaybackID;
    DWORD RecordingID;
    UINT OtherLines;
    TCHAR DeviceName[128];

    DWORD Count;
    DWORD tmp;
} PREFERENCES_CONTEXT, *PPREFERENCES_CONTEXT;

typedef struct
{
    WCHAR LineName[MIXER_LONG_NAME_CHARS];
    UINT SliderPos;
    BOOL bVertical;
    BOOL bSwitch;

}SET_VOLUME_CONTEXT, *PSET_VOLUME_CONTEXT;

/* NOTE: do NOT modify SNDVOL_REG_LINESTATE for binary compatibility with XP! */
typedef struct _SNDVOL_REG_LINESTATE
{
    DWORD Flags;
    WCHAR LineName[MIXER_LONG_NAME_CHARS];
} SNDVOL_REG_LINESTATE, *PSNDVOL_REG_LINESTATE;


typedef BOOL (CALLBACK *PFNSNDMIXENUMLINES)(PSND_MIXER Mixer, LPMIXERLINE Line, UINT DisplayControls, PVOID Context);
typedef BOOL (CALLBACK *PFNSNDMIXENUMCONNECTIONS)(PSND_MIXER Mixer, DWORD LineID, LPMIXERLINE Line, PVOID Context);
typedef BOOL (CALLBACK *PFNSNDMIXENUMPRODUCTS)(PSND_MIXER Mixer, UINT Id, LPCTSTR ProductName, PVOID Context);

PSND_MIXER SndMixerCreate(HWND hWndNotification);
VOID SndMixerDestroy(PSND_MIXER Mixer);
VOID SndMixerClose(PSND_MIXER Mixer);
BOOL SndMixerSelect(PSND_MIXER Mixer, UINT MixerId);
UINT SndMixerGetSelection(PSND_MIXER Mixer);
INT SndMixerSetVolumeControlDetails(PSND_MIXER Mixer, DWORD dwControlID, DWORD cbDetails, LPVOID paDetails);
INT SndMixerGetVolumeControlDetails(PSND_MIXER Mixer, DWORD dwControlID, DWORD cbDetails, LPVOID paDetails);
INT SndMixerGetProductName(PSND_MIXER Mixer, LPTSTR lpBuffer, UINT uSize);
INT SndMixerGetLineName(PSND_MIXER Mixer, DWORD LineID, LPTSTR lpBuffer, UINT uSize, BOOL LongName);
BOOL SndMixerEnumProducts(PSND_MIXER Mixer, PFNSNDMIXENUMPRODUCTS EnumProc, PVOID Context);
INT SndMixerGetDestinationCount(PSND_MIXER Mixer);
BOOL SndMixerEnumLines(PSND_MIXER Mixer, PFNSNDMIXENUMLINES EnumProc, PVOID Context);
BOOL SndMixerEnumConnections(PSND_MIXER Mixer, DWORD LineID, PFNSNDMIXENUMCONNECTIONS EnumProc, PVOID Context);
BOOL SndMixerIsDisplayControl(PSND_MIXER Mixer, LPMIXERCONTROL Control);
BOOL SndMixerQueryControls(PSND_MIXER Mixer, PUINT DisplayControls, LPMIXERLINE LineInfo, LPMIXERCONTROL *Controls);

/*
 * dialog.c
 */
VOID LoadDialogCtrls(PPREFERENCES_CONTEXT PrefContext);
VOID UpdateDialogLineSliderControl(PPREFERENCES_CONTEXT PrefContext, LPMIXERLINE Line, DWORD dwControlID, DWORD DialogID, DWORD Position);
VOID UpdateDialogLineSwitchControl(PPREFERENCES_CONTEXT PrefContext, LPMIXERLINE Line, LONG fValue);

/*
 * MISC
 */

extern HKEY hAppSettingsKey;

BOOL
InitAppConfig(VOID);

VOID
CloseAppConfig(VOID);

INT
AllocAndLoadString(OUT LPWSTR *lpTarget,
                   IN HINSTANCE hInst,
                   IN UINT uID);

DWORD
LoadAndFormatString(IN HINSTANCE hInstance,
                    IN UINT uID,
                    OUT LPWSTR *lpTarget,
                    ...);

BOOL
ReadLineConfig(IN LPTSTR szDeviceName,
               IN LPTSTR szLineName,
               IN LPTSTR szControlName,
               OUT DWORD *Flags);

BOOL
WriteLineConfig(IN LPTSTR szDeviceName,
                IN LPTSTR szLineName,
                IN PSNDVOL_REG_LINESTATE LineState,
                IN DWORD cbSize);

#endif /* __SNDVOL32_H */
