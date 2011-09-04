/*
 * PROJECT:     PAINT for Odyssey
 * LICENSE:     LGPL
 * FILE:        base/applications/paint/main.c
 * PURPOSE:     Initializing everything
 * PROGRAMMERS: Benedikt Freisen
 */

/* INCLUDES *********************************************************/

#include "precomp.h"

/* FUNCTIONS ********************************************************/

HDC hDrawingDC;
HDC hSelDC;
int *bmAddress;
BITMAPINFO bitmapinfo;
int imgXRes = 400;
int imgYRes = 300;

HBITMAP hBms[HISTORYSIZE];
int currInd = 0;
int undoSteps = 0;
int redoSteps = 0;
BOOL imageSaved = TRUE;

LONG startX;
LONG startY;
LONG lastX;
LONG lastY;
int lineWidth = 1;
int shapeStyle = 0;
int brushStyle = 0;
int activeTool = TOOL_PEN;
int airBrushWidth = 5;
int rubberRadius = 4;
int transpBg = 0;
int zoom = 1000;
int rectSel_src[4];
int rectSel_dest[4];
HWND hSelection;
HWND hImageArea;
HBITMAP hSelBm;
HBITMAP hSelMask;

/* initial palette colors; may be changed by the user during execution */
int palColors[28] = { 0x000000, 0x464646, 0x787878, 0x300099, 0x241ced, 0x0078ff, 0x0ec2ff,
    0x00f2ff, 0x1de6a8, 0x4cb122, 0xefb700, 0xf36d4d, 0x99362f, 0x98316f,
    0xffffff, 0xdcdcdc, 0xb4b4b4, 0x3c5a9c, 0xb1a3ff, 0x7aaae5, 0x9ce4f5,
    0xbdf9ff, 0xbcf9d3, 0x61bb9d, 0xead999, 0xd19a70, 0x8e6d54, 0xd5a5b5
};

/* foreground and background colors with initial value */
int fgColor = 0x00000000;
int bgColor = 0x00ffffff;

HWND hStatusBar;
HWND hScrollbox;
HWND hMainWnd;
HWND hPalWin;
HWND hToolBoxContainer;
HWND hToolSettings;
HWND hTrackbarZoom;
CHOOSECOLOR choosecolor;
OPENFILENAME ofn;
OPENFILENAME sfn;
HICON hNontranspIcon;
HICON hTranspIcon;

HCURSOR hCurFill;
HCURSOR hCurColor;
HCURSOR hCurZoom;
HCURSOR hCurPen;
HCURSOR hCurAirbrush;

HWND hScrlClient;

HWND hToolBtn[16];

HINSTANCE hProgInstance;

TCHAR filename[256];
TCHAR filepathname[1000];
BOOL isAFile = FALSE;
int fileSize;
int fileHPPM = 2834;
int fileVPPM = 2834;
SYSTEMTIME fileTime;

BOOL showGrid = FALSE;
BOOL showMiniature = FALSE;

HWND hwndMiniature;

HWND hSizeboxLeftTop;
HWND hSizeboxCenterTop;
HWND hSizeboxRightTop;
HWND hSizeboxLeftCenter;
HWND hSizeboxRightCenter;
HWND hSizeboxLeftBottom;
HWND hSizeboxCenterBottom;
HWND hSizeboxRightBottom;

/* entry point */

int WINAPI
_tWinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPTSTR lpszArgument, int nFunsterStil)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    
    WNDCLASSEX wclScroll;
    WNDCLASSEX wincl;
    WNDCLASSEX wclPal;
    WNDCLASSEX wclSettings;
    WNDCLASSEX wclSelection;
    
    TCHAR progtitle[1000];
    TCHAR resstr[100];
    HMENU menu;
    HWND hToolbar;
    HIMAGELIST hImageList;
    HANDLE haccel;
    HBITMAP tempBm;
    int i;
    TCHAR tooltips[16][30];
    HDC hDC;
    
    TCHAR *c;
    TCHAR sfnFilename[1000];
    TCHAR sfnFiletitle[256];
    TCHAR sfnFilter[1000];
    TCHAR ofnFilename[1000];
    TCHAR ofnFiletitle[256];
    TCHAR ofnFilter[1000];
    TCHAR miniaturetitle[100];
    static int custColors[16] = { 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff
    };

    hProgInstance = hThisInstance;

    /* Necessary */
    InitCommonControls();

    /* initializing and registering the window class used for the main window */
    wincl.hInstance         = hThisInstance;
    wincl.lpszClassName     = _T("WindowsApp");
    wincl.lpfnWndProc       = WindowProcedure;
    wincl.style             = CS_DBLCLKS;
    wincl.cbSize            = sizeof(WNDCLASSEX);
    wincl.hIcon             = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_APPICON));
    wincl.hIconSm           = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_APPICON));
    wincl.hCursor           = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName      = NULL;
    wincl.cbClsExtra        = 0;
    wincl.cbWndExtra        = 0;
    wincl.hbrBackground     = GetSysColorBrush(COLOR_BTNFACE);
    RegisterClassEx (&wincl);

    /* initializing and registering the window class used for the scroll box */
    wclScroll.hInstance     = hThisInstance;
    wclScroll.lpszClassName = _T("Scrollbox");
    wclScroll.lpfnWndProc   = WindowProcedure;
    wclScroll.style         = 0;
    wclScroll.cbSize        = sizeof(WNDCLASSEX);
    wclScroll.hIcon         = NULL;
    wclScroll.hIconSm       = NULL;
    wclScroll.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wclScroll.lpszMenuName  = NULL;
    wclScroll.cbClsExtra    = 0;
    wclScroll.cbWndExtra    = 0;
    wclScroll.hbrBackground = GetSysColorBrush(COLOR_APPWORKSPACE);
    RegisterClassEx (&wclScroll);

    /* initializing and registering the window class used for the palette window */
    wclPal.hInstance        = hThisInstance;
    wclPal.lpszClassName    = _T("Palette");
    wclPal.lpfnWndProc      = PalWinProc;
    wclPal.style            = CS_DBLCLKS;
    wclPal.cbSize           = sizeof(WNDCLASSEX);
    wclPal.hIcon            = NULL;
    wclPal.hIconSm          = NULL;
    wclPal.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wclPal.lpszMenuName     = NULL;
    wclPal.cbClsExtra       = 0;
    wclPal.cbWndExtra       = 0;
    wclPal.hbrBackground    = GetSysColorBrush(COLOR_BTNFACE);
    RegisterClassEx (&wclPal);

    /* initializing and registering the window class for the settings window */
    wclSettings.hInstance       = hThisInstance;
    wclSettings.lpszClassName   = _T("ToolSettings");
    wclSettings.lpfnWndProc     = SettingsWinProc;
    wclSettings.style           = CS_DBLCLKS;
    wclSettings.cbSize          = sizeof(WNDCLASSEX);
    wclSettings.hIcon           = NULL;
    wclSettings.hIconSm         = NULL;
    wclSettings.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wclSettings.lpszMenuName    = NULL;
    wclSettings.cbClsExtra      = 0;
    wclSettings.cbWndExtra      = 0;
    wclSettings.hbrBackground   = GetSysColorBrush(COLOR_BTNFACE);
    RegisterClassEx (&wclSettings);

    /* initializing and registering the window class for the selection frame */
    wclSelection.hInstance      = hThisInstance;
    wclSelection.lpszClassName  = _T("Selection");
    wclSelection.lpfnWndProc    = SelectionWinProc;
    wclSelection.style          = CS_DBLCLKS;
    wclSelection.cbSize         = sizeof(WNDCLASSEX);
    wclSelection.hIcon          = NULL;
    wclSelection.hIconSm        = NULL;
    wclSelection.hCursor        = LoadCursor(NULL, IDC_SIZEALL);
    wclSelection.lpszMenuName   = NULL;
    wclSelection.cbClsExtra     = 0;
    wclSelection.cbWndExtra     = 0;
    wclSelection.hbrBackground  = NULL;
    RegisterClassEx (&wclSelection);

    /* initializing and registering the window class for the size boxes */
    wclSettings.hInstance       = hThisInstance;
    wclSettings.lpszClassName   = _T("Sizebox");
    wclSettings.lpfnWndProc     = SizeboxWinProc;
    wclSettings.style           = CS_DBLCLKS;
    wclSettings.cbSize          = sizeof(WNDCLASSEX);
    wclSettings.hIcon           = NULL;
    wclSettings.hIconSm         = NULL;
    wclSettings.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wclSettings.lpszMenuName    = NULL;
    wclSettings.cbClsExtra      = 0;
    wclSettings.cbWndExtra      = 0;
    wclSettings.hbrBackground   = GetSysColorBrush(COLOR_HIGHLIGHT);
    RegisterClassEx (&wclSettings);

    LoadString(hThisInstance, IDS_DEFAULTFILENAME, filename, SIZEOF(filename));
    LoadString(hThisInstance, IDS_WINDOWTITLE, resstr, SIZEOF(resstr));
    _stprintf(progtitle, resstr, filename);
    LoadString(hThisInstance, IDS_MINIATURETITLE, miniaturetitle, SIZEOF(miniaturetitle));
    
    /* create main window */
    hwnd =
        CreateWindowEx(0, _T("WindowsApp"), progtitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 544,
                       375, HWND_DESKTOP, NULL, hThisInstance, NULL);
    hMainWnd = hwnd;

    hwndMiniature =
        CreateWindowEx(WS_EX_PALETTEWINDOW, _T("WindowsApp"), miniaturetitle,
                       WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, 180, 200, 120, 100, hwnd,
                       NULL, hThisInstance, NULL);

    /* loading and setting the window menu from resource */
    menu = LoadMenu(hThisInstance, MAKEINTRESOURCE(ID_MENU));
    SetMenu(hwnd, menu);
    haccel = LoadAccelerators(hThisInstance, MAKEINTRESOURCE(800));

    /* preloading the draw transparent/nontransparent icons for later use */
    hNontranspIcon =
        LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_NONTRANSPARENT), IMAGE_ICON, 40, 30, LR_DEFAULTCOLOR);
    hTranspIcon =
        LoadImage(hThisInstance, MAKEINTRESOURCE(IDI_TRANSPARENT), IMAGE_ICON, 40, 30, LR_DEFAULTCOLOR);

    hCurFill     = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDC_FILL));
    hCurColor    = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDC_COLOR));
    hCurZoom     = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDC_ZOOM));
    hCurPen      = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDC_PEN));
    hCurAirbrush = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDC_AIRBRUSH));

    CreateWindowEx(0, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 0, 0, 5000, 2, hwnd, NULL,
                   hThisInstance, NULL);

    hToolBoxContainer =
        CreateWindowEx(0, _T("WindowsApp"), _T(""), WS_CHILD | WS_VISIBLE, 2, 2, 52, 350, hwnd, NULL,
                       hThisInstance, NULL);
    /* creating the 16 bitmap radio buttons and setting the bitmap */


    /* 
     * FIXME: Unintentionally there is a line above the tool bar. 
     * To prevent cropping of the buttons height has been increased from 200 to 205
     */
    hToolbar =
        CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
                       WS_CHILD | WS_VISIBLE | CCS_NOPARENTALIGN | CCS_VERT | CCS_NORESIZE | TBSTYLE_TOOLTIPS,
                       1, 1, 50, 205, hToolBoxContainer, NULL, hThisInstance, NULL);
    hImageList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 16, 0);
    SendMessage(hToolbar, TB_SETIMAGELIST, 0, (LPARAM) hImageList);
    tempBm = LoadImage(hThisInstance, MAKEINTRESOURCE(IDB_TOOLBARICONS), IMAGE_BITMAP, 256, 16, 0);
    ImageList_AddMasked(hImageList, tempBm, 0xff00ff);
    DeleteObject(tempBm);
    SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    
    for(i = 0; i < 16; i++)
    {
        TBBUTTON tbbutton;
        int wrapnow = 0;

        if (i % 2 == 1)
            wrapnow = TBSTATE_WRAP;

        LoadString(hThisInstance, IDS_TOOLTIP1 + i, tooltips[i], 30);
        ZeroMemory(&tbbutton, sizeof(TBBUTTON));
        tbbutton.iString   = (INT_PTR) tooltips[i];
        tbbutton.fsStyle   = TBSTYLE_CHECKGROUP;
        tbbutton.fsState   = TBSTATE_ENABLED | wrapnow;
        tbbutton.idCommand = ID_FREESEL + i;
        tbbutton.iBitmap   = i;
        SendMessage(hToolbar, TB_ADDBUTTONS, 1, (LPARAM) &tbbutton);
    }
    
    SendMessage(hToolbar, TB_CHECKBUTTON, ID_PEN, MAKELONG(TRUE, 0));
    SendMessage(hToolbar, TB_SETMAXTEXTROWS, 0, 0);
    SendMessage(hToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(25, 25));

    /* creating the tool settings child window */
    hToolSettings =
        CreateWindowEx(0, _T("ToolSettings"), _T(""), WS_CHILD | WS_VISIBLE, 5, 208, 42, 140,
                       hToolBoxContainer, NULL, hThisInstance, NULL);
    hTrackbarZoom =
        CreateWindowEx(0, TRACKBAR_CLASS, _T(""), WS_CHILD | TBS_VERT | TBS_AUTOTICKS, 1, 1, 40, 64,
                       hToolSettings, NULL, hThisInstance, NULL);
    SendMessage(hTrackbarZoom, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 6));
    SendMessage(hTrackbarZoom, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) 3);

    /* creating the palette child window */
    hPalWin =
        CreateWindowEx(0, _T("Palette"), _T(""), WS_CHILD | WS_VISIBLE, 56, 9, 255, 32, hwnd, NULL,
                       hThisInstance, NULL);

    /* creating the scroll box */
    hScrollbox =
        CreateWindowEx(WS_EX_CLIENTEDGE, _T("Scrollbox"), _T(""),
                       WS_CHILD | WS_GROUP | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE, 56, 49, 472, 248, hwnd,
                       NULL, hThisInstance, NULL);

    /* creating the status bar */
    hStatusBar =
        CreateWindowEx(0, STATUSCLASSNAME, _T(""), SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd,
                       NULL, hThisInstance, NULL);
    SendMessage(hStatusBar, SB_SETMINHEIGHT, 21, 0);

    hScrlClient =
        CreateWindowEx(0, _T("Scrollbox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 500, 500, hScrollbox, NULL,
                       hThisInstance, NULL);

    /* create selection window (initially hidden) */
    hSelection =
        CreateWindowEx(WS_EX_TRANSPARENT, _T("Selection"), _T(""), WS_CHILD | BS_OWNERDRAW, 350, 0, 100, 100,
                       hScrlClient, NULL, hThisInstance, NULL);

    /* creating the window inside the scroll box, on which the image in hDrawingDC's bitmap is drawn */
    hImageArea =
        CreateWindowEx(0, _T("Scrollbox"), _T(""), WS_CHILD | WS_VISIBLE, 3, 3, imgXRes, imgYRes, hScrlClient,
                       NULL, hThisInstance, NULL);

    hDC = GetDC(hImageArea);
    hDrawingDC = CreateCompatibleDC(hDC);
    hSelDC     = CreateCompatibleDC(hDC);
    ReleaseDC(hImageArea, hDC);
    SelectObject(hDrawingDC, CreatePen(PS_SOLID, 0, fgColor));
    SelectObject(hDrawingDC, CreateSolidBrush(bgColor));

    hBms[0] = CreateDIBWithProperties(imgXRes, imgYRes);
    SelectObject(hDrawingDC, hBms[0]);
    Rectangle(hDrawingDC, 0 - 1, 0 - 1, imgXRes + 1, imgYRes + 1);

    if (lpszArgument[0] != 0)
    {
        HBITMAP bmNew = NULL;
        LoadDIBFromFile(&bmNew, lpszArgument, &fileTime, &fileSize, &fileHPPM, &fileVPPM);
        if (bmNew != NULL)
        {
            TCHAR tempstr[1000];
            TCHAR resstr[100];
            TCHAR *temp;
            insertReversible(bmNew);
            GetFullPathName(lpszArgument, SIZEOF(filepathname), filepathname, &temp);
            _tcscpy(filename, temp);
            LoadString(hProgInstance, IDS_WINDOWTITLE, resstr, SIZEOF(resstr));
            _stprintf(tempstr, resstr, filename);
            SetWindowText(hMainWnd, tempstr);
            clearHistory();
            isAFile = TRUE;
        }
    }

    /* initializing the CHOOSECOLOR structure for use with ChooseColor */
    choosecolor.lStructSize    = sizeof(CHOOSECOLOR);
    choosecolor.hwndOwner      = hwnd;
    choosecolor.hInstance      = NULL;
    choosecolor.rgbResult      = 0x00ffffff;
    choosecolor.lpCustColors   = (COLORREF*) &custColors;
    choosecolor.Flags          = 0;
    choosecolor.lCustData      = 0;
    choosecolor.lpfnHook       = NULL;
    choosecolor.lpTemplateName = NULL;

    /* initializing the OPENFILENAME structure for use with GetOpenFileName and GetSaveFileName */
    CopyMemory(ofnFilename, filename, sizeof(filename));
    LoadString(hThisInstance, IDS_OPENFILTER, ofnFilter, SIZEOF(ofnFilter));
    for(c = ofnFilter; *c; c++)
        if (*c == '\1')
            *c = '\0';
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize    = sizeof(OPENFILENAME);
    ofn.hwndOwner      = hwnd;
    ofn.hInstance      = hThisInstance;
    ofn.lpstrFilter    = ofnFilter;
    ofn.lpstrFile      = ofnFilename;
    ofn.nMaxFile       = SIZEOF(ofnFilename);
    ofn.lpstrFileTitle = ofnFiletitle;
    ofn.nMaxFileTitle  = SIZEOF(ofnFiletitle);
    ofn.Flags          = OFN_HIDEREADONLY;

    CopyMemory(sfnFilename, filename, sizeof(filename));
    LoadString(hThisInstance, IDS_SAVEFILTER, sfnFilter, SIZEOF(sfnFilter));
    for(c = sfnFilter; *c; c++)
        if (*c == '\1')
            *c = '\0';
    ZeroMemory(&sfn, sizeof(OPENFILENAME));
    sfn.lStructSize    = sizeof(OPENFILENAME);
    sfn.hwndOwner      = hwnd;
    sfn.hInstance      = hThisInstance;
    sfn.lpstrFilter    = sfnFilter;
    sfn.lpstrFile      = sfnFilename;
    sfn.nMaxFile       = SIZEOF(sfnFilename);
    sfn.lpstrFileTitle = sfnFiletitle;
    sfn.nMaxFileTitle  = SIZEOF(sfnFiletitle);
    sfn.Flags          = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

    /* creating the size boxes */
    hSizeboxLeftTop =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxCenterTop =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxRightTop =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxLeftCenter =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxRightCenter =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxLeftBottom =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxCenterBottom =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    hSizeboxRightBottom =
        CreateWindowEx(0, _T("Sizebox"), _T(""), WS_CHILD | WS_VISIBLE, 0, 0, 3, 3, hScrlClient, NULL,
                       hThisInstance, NULL);
    /* placing the size boxes around the image */
    SendMessage(hImageArea, WM_SIZE, 0, 0);

    /* by moving the window, the things in WM_SIZE are done */
    MoveWindow(hwnd, 100, 100, 600, 450, TRUE);

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nFunsterStil);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateAccelerator(hwnd, haccel, &messages);

        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}
