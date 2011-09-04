#ifndef _UNDOCUSER_H
#define _UNDOCUSER_H

/* Built in class atoms */
#define WC_MENU       (MAKEINTATOM(0x8000))
#define WC_DESKTOP    (MAKEINTATOM(0x8001))
#define WC_DIALOG     (MAKEINTATOM(0x8002))
#define WC_SWITCH     (MAKEINTATOM(0x8003))
#define WC_ICONTITLE  (MAKEINTATOM(0x8004))

/* Non SDK Styles */
#define ES_COMBO 0x200 /* Parent is a combobox */
#define WS_MAXIMIZED  WS_MAXIMIZE
#define WS_MINIMIZED  WS_MINIMIZE

/* Non SDK ExStyles */
#define WS_EX_DRAGDETECT               0x00000002
#define WS_EX_MAKEVISIBLEWHENUNGHOSTED 0x00000800
#define WS_EX_FORCELEGACYRESIZENCMETR  0x00800000
#define WS_EX_UISTATEACTIVE            0x04000000
#define WS_EX_REDIRECTED               0x20000000
#define WS_EX_UISTATEKBACCELHIDDEN     0x40000000
#define WS_EX_UISTATEFOCUSRECTHIDDEN   0x80000000
#define WS_EX_SETANSICREATOR           0x80000000 // For WNDS_ANSICREATOR

/* Non SDK Window Message types. */
#define WM_SETVISIBLE      0x00000009
#define WM_ALTTABACTIVE    0x00000029
#define WM_ISACTIVEICON    0x00000035
#define WM_QUERYPARKICON   0x00000036
#define WM_CLIENTSHUTDOWN  0x0000003B
#define WM_COPYGLOBALDATA  0x00000049
#define WM_LOGONNOTIFY     0x0000004c
#define WM_KEYF1           0x0000004d
#define WM_SYSTIMER        0x00000118
#define WM_LBTRACKPOINT    0x00000131
#define LB_CARETON         0x000001a3
#define LB_CARETOFF        0x000001a4
#define WM_DROPOBJECT	   0x0000022A
#define WM_QUERYDROPOBJECT 0x0000022B
#define WM_BEGINDRAG       0x0000022C
#define WM_DRAGLOOP	       0x0000022D
#define WM_DRAGSELECT	   0x0000022E
#define WM_DRAGMOVE	       0x0000022F
#define WM_POPUPSYSTEMMENU 0x00000313
#define WM_CBT             0x000003FF // Odyssey only.
#define WM_MAXIMUM         0x0001FFFF

/* Non SDK DCE types.*/
#define DCX_USESTYLE     0x00010000
#define DCX_KEEPCLIPRGN  0x00040000
#define DCX_KEEPLAYOUT   0x40000000
#define DCX_PROCESSOWNED 0x80000000

/* Caret timer ID */
#define IDCARETTIMER (0xffff)

/* SetWindowPos undocumented flags */
#define SWP_NOCLIENTSIZE 0x0800
#define SWP_NOCLIENTMOVE 0x1000
#define SWP_STATECHANGED 0x8000

/* Non SDK Queue state flags. */
#define QS_SMRESULT 0x8000 /* see "Undoc. Windows" */

//
// Definitions used by WM_CLIENTSHUTDOWN
//
// Client Shutdown messages
#define MCS_SHUTDOWNTIMERS  1
#define MCS_QUERYENDSESSION 2
// Client Shutdown returns
#define MCSR_GOODFORSHUTDOWN  1
#define MCSR_SHUTDOWNFINISHED 2
#define MCSR_DONOTSHUTDOWN    3

//
// Definitions used by WM_LOGONNOTIFY
//
#define LN_SHELL_EXITED       0x2
#define LN_START_TASK_MANAGER 0x4
#define LN_LOCK_WORKSTATION   0x5
#define LN_UNLOCK_WORKSTATION 0x6
#define LN_MESSAGE_BEEP       0x9
#define LN_START_SCREENSAVE   0xA

#define STARTF_SCRNSAVER 0x80000000

#define CW_USEDEFAULT16 ((short)0x8000)

#define SBRG_SCROLLBAR     0 /* the scrollbar itself */
#define SBRG_TOPRIGHTBTN   1 /* the top or right button */
#define SBRG_PAGEUPRIGHT   2 /* the page up or page right region */
#define SBRG_SCROLLBOX     3 /* the scroll box */
#define SBRG_PAGEDOWNLEFT  4 /* the page down or page left region */
#define SBRG_BOTTOMLEFTBTN 5 /* the bottom or left button */

BOOL WINAPI SetLogonNotifyWindow(HWND Wnd, HWINSTA WinSta);
BOOL WINAPI KillSystemTimer(HWND,UINT_PTR);
UINT_PTR WINAPI SetSystemTimer(HWND,UINT_PTR,UINT,TIMERPROC);
DWORD_PTR WINAPI SetSysColorsTemp(const COLORREF *, const HBRUSH *, DWORD_PTR);
BOOL WINAPI SetDeskWallPaper(LPCSTR);
VOID WINAPI ScrollChildren(HWND,UINT,WPARAM,LPARAM);
void WINAPI CalcChildScroll(HWND, INT);
BOOL WINAPI RegisterLogonProcess(DWORD,BOOL);
DWORD WINAPI GetAppCompatFlags(HTASK hTask);
DWORD WINAPI GetAppCompatFlags2(HTASK hTask);
LONG WINAPI CsrBroadcastSystemMessageExW(DWORD dwflags,
                                         LPDWORD lpdwRecipients,
                                         UINT uiMessage,
                                         WPARAM wParam,
                                         LPARAM lParam,
                                         PBSMINFO pBSMInfo);
BOOL WINAPI CliImmSetHotKey(DWORD dwID, UINT uModifiers, UINT uVirtualKey, HKL hKl);
HWND WINAPI GetTaskmanWindow(VOID);
HWND WINAPI GetProgmanWindow(VOID);

//
// User api hook
//

typedef struct _USERAPIHOOKINFO
{
  DWORD m_size;
  LPCWSTR m_dllname1;
  LPCWSTR m_funname1;
  LPCWSTR m_dllname2;
  LPCWSTR m_funname2;
} USERAPIHOOKINFO,*PUSERAPIHOOKINFO;

typedef enum _UAPIHK
{
  uahLoadInit,
  uahStop,
  uahShutdown
} UAPIHK, *PUAPIHK;

typedef DWORD (CALLBACK * USERAPIHOOKPROC)(UAPIHK State, ULONG_PTR Info);

typedef LRESULT(CALLBACK *WNDPROC_OWP)(HWND,UINT,WPARAM,LPARAM,ULONG_PTR,PDWORD);

typedef struct _UAHOWP
{
  BYTE*  MsgBitArray;
  DWORD  Size;
} UAHOWP, *PUAHOWP;

typedef struct tagUSERAPIHOOK
{
  DWORD   size;
  WNDPROC DefWindowProcA;
  WNDPROC DefWindowProcW;
  UAHOWP  DefWndProcArray;
  FARPROC GetScrollInfo;
  FARPROC SetScrollInfo;
  FARPROC EnableScrollBar;
  FARPROC AdjustWindowRectEx;
  FARPROC SetWindowRgn;
  WNDPROC_OWP PreWndProc;
  WNDPROC_OWP PostWndProc;
  UAHOWP  WndProcArray;
  WNDPROC_OWP PreDefDlgProc;
  WNDPROC_OWP PostDefDlgProc;
  UAHOWP  DlgProcArray;
  FARPROC GetSystemMetrics;
  FARPROC SystemParametersInfoA;
  FARPROC SystemParametersInfoW;
  FARPROC ForceResetUserApiHook;
  FARPROC DrawFrameControl;
  FARPROC DrawCaption;
  FARPROC MDIRedrawFrame;
  FARPROC GetRealWindowOwner;
} USERAPIHOOK, *PUSERAPIHOOK;

BOOL WINAPI RegisterUserApiHook(PUSERAPIHOOKINFO puah);
BOOL WINAPI UnregisterUserApiHook(VOID);

#endif
