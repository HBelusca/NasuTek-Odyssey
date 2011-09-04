/*
 * PROJECT:     Odyssey Character Map
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        base/applications/charmap/charmap.c
 * PURPOSE:     main dialog implementation
 * COPYRIGHT:   Copyright 2007 Ged Murphy <gedmurphy@odyssey.org>
 *
 */

#include <precomp.h>

#define ID_ABOUT    0x1

typedef struct {
    BOOL    IsAdvancedView;
} SETTINGS;

HINSTANCE hInstance;
HWND      hCharmapDlg;
HWND      hAdvancedDlg;
HWND      hStatusWnd;
HICON     hSmIcon;
HICON     hBgIcon;
SETTINGS  Settings;

/* Font-enumeration callback */
static
int
CALLBACK
EnumFontNames(ENUMLOGFONTEXW *lpelfe,
              NEWTEXTMETRICEXW *lpntme,
              DWORD FontType,
              LPARAM lParam)
{
    HWND hwndCombo = (HWND)lParam;
    LPWSTR pszName  = lpelfe->elfLogFont.lfFaceName;

    /* make sure font doesn't already exist in our list */
    if(SendMessageW(hwndCombo,
                    CB_FINDSTRING,
                    0,
                    (LPARAM)pszName) == CB_ERR)
    {
        INT idx;
        BOOL fFixed;
        BOOL fTrueType;

        /* add the font */
        idx = (INT)SendMessageW(hwndCombo,
                                CB_ADDSTRING,
                                0,
                                (LPARAM)pszName);

        /* record the font's attributes (Fixedwidth and Truetype) */
        fFixed = (lpelfe->elfLogFont.lfPitchAndFamily & FIXED_PITCH) ? TRUE : FALSE;
        fTrueType = (lpelfe->elfLogFont.lfOutPrecision == OUT_STROKE_PRECIS) ? TRUE : FALSE;

        /* store this information in the list-item's userdata area */
        SendMessageW(hwndCombo,
                     CB_SETITEMDATA,
                     idx,
                     MAKEWPARAM(fFixed, fTrueType));
    }

    return 1;
}


/* Initialize the font-list by enumeration all system fonts */
static
VOID
FillFontStyleComboList(HWND hwndCombo)
{
    HDC hdc;
    LOGFONTW lf;

    /* FIXME: for fun, draw each font in its own style */
    HFONT hFont = GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(hwndCombo,
                 WM_SETFONT,
                 (WPARAM)hFont,
                 0);

    ZeroMemory(&lf, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;

    hdc = GetDC(hwndCombo);

    /* store the list of fonts in the combo */
    EnumFontFamiliesExW(hdc,
                        &lf,
                        (FONTENUMPROCW)EnumFontNames,
                        (LPARAM)hwndCombo,
                        0);

    ReleaseDC(hwndCombo,
              hdc);

    SendMessageW(hwndCombo,
                 CB_SETCURSEL,
                 0,
                 0);
}


static
VOID
ChangeMapFont(HWND hDlg)
{
    HWND hCombo;
    HWND hMap;
    LPWSTR lpFontName;
    INT Len;

    hCombo = GetDlgItem(hDlg, IDC_FONTCOMBO);

    Len = GetWindowTextLengthW(hCombo);

    if (Len != 0)
    {
        lpFontName = HeapAlloc(GetProcessHeap(),
                               0,
                               (Len + 1) * sizeof(WCHAR));

        if (lpFontName)
        {
            SendMessageW(hCombo,
                         WM_GETTEXT,
                         Len + 1,
                         (LPARAM)lpFontName);

            hMap = GetDlgItem(hDlg, IDC_FONTMAP);

            SendMessageW(hMap,
                         FM_SETFONT,
                         0,
                         (LPARAM)lpFontName);
        }

        HeapFree(GetProcessHeap(),
                 0,
                 lpFontName);
    }
}

// Copy collected characters into the clipboard
static
void
CopyCharacters(HWND hDlg)
{
    HWND hText = GetDlgItem(hDlg, IDC_TEXTBOX);
    DWORD dwStart, dwEnd;

    // Acquire selection limits
    SendMessage(hText, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

    // Test if the whose text is unselected
    if(dwStart == dwEnd) {
        
        // Select the whole text
        SendMessageW(hText, EM_SETSEL, 0, -1);

        // Copy text
        SendMessageW(hText, WM_COPY, 0, 0);

        // Restore previous values
        SendMessageW(hText, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);

    } else {

        // Copy text
        SendMessageW(hText, WM_COPY, 0, 0);
    }
}

// Recover charset for the given font
static
BYTE
GetFontMetrics(HWND hWnd, HFONT hFont)
{
    TEXTMETRIC tmFont;
    HGDIOBJ    hOldObj;
    HDC        hDC;

    hDC = GetDC(hWnd);
    hOldObj = SelectObject(hDC, hFont);
    GetTextMetrics(hDC, &tmFont);
    SelectObject(hDC, hOldObj);
    ReleaseDC(hWnd, hDC);

    return tmFont.tmCharSet;
}

// Select a new character
static
VOID
AddCharToSelection(HWND hDlg, WCHAR ch)
{
    HWND    hMap = GetDlgItem(hDlg, IDC_FONTMAP);
    HWND    hText = GetDlgItem(hDlg, IDC_TEXTBOX);
    HFONT   hFont;
    LOGFONT lFont;
    CHARFORMAT cf;

    // Retrieve current character selected
    if (ch == 0)
    {
        ch = (WCHAR) SendMessageW(hMap, FM_GETCHAR, 0, 0);
        if (!ch)
            return;
    }

    // Retrieve current selected font
    hFont = (HFONT)SendMessage(hMap, FM_GETHFONT, 0, 0);

    // Recover LOGFONT structure from hFont
    if (!GetObject(hFont, sizeof(LOGFONT), &lFont))
        return;

    // Recover font properties of Richedit control
    ZeroMemory(&cf, sizeof(cf));
    cf.cbSize = sizeof(cf);
    SendMessage(hText, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Apply properties of the new font
    cf.bCharSet = GetFontMetrics(hText, hFont);

    // Update font name
    wcscpy(cf.szFaceName, lFont.lfFaceName);

    // Update font properties
    SendMessage(hText, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Send selected character to Richedit
    SendMessage(hText, WM_CHAR, (WPARAM)ch, 0);
}


static
void
UpdateSettings(HWND hDlg)
{
    if (hDlg == hCharmapDlg)
    {
        Settings.IsAdvancedView =
        SendDlgItemMessage(hDlg, IDC_CHECK_ADVANCED, BM_GETCHECK, 0, 0);
    }

    if (hDlg == hAdvancedDlg)
    {
    }
}

static
void
ChangeView(HWND hWnd)
{
    RECT rcCharmap;
    RECT rcAdvanced;
    RECT rcPanelExt;
    RECT rcPanelInt;
    RECT rcStatus;
    UINT DeX, DeY;
    UINT xPos, yPos;
    UINT Width, Height;
    UINT DeskTopWidth, DeskTopHeight;

    GetClientRect(hCharmapDlg, &rcCharmap);
    GetClientRect(hAdvancedDlg, &rcAdvanced);
    GetWindowRect(hWnd, &rcPanelExt);
    GetClientRect(hWnd, &rcPanelInt);
    GetClientRect(hStatusWnd, &rcStatus);

    DeskTopWidth = GetSystemMetrics(SM_CXFULLSCREEN);
    DeskTopHeight = GetSystemMetrics(SM_CYFULLSCREEN);

    DeX = (rcPanelExt.right - rcPanelExt.left) - rcPanelInt.right;
    DeY = (rcPanelExt.bottom - rcPanelExt.top) - rcPanelInt.bottom;

    MoveWindow(hCharmapDlg, 0, 0, rcCharmap.right, rcCharmap.bottom, FALSE);
    MoveWindow(hAdvancedDlg, 0, rcCharmap.bottom, rcAdvanced.right, rcAdvanced.bottom, FALSE);

    ShowWindow(hAdvancedDlg, (Settings.IsAdvancedView) ? SW_SHOW : SW_HIDE);

    xPos = rcPanelExt.left;
    yPos = rcPanelExt.top;

    Width = DeX + rcCharmap.right;
    Height = DeY + rcCharmap.bottom + rcStatus.bottom;

    if (Settings.IsAdvancedView)
        Height += rcAdvanced.bottom;

    if ((xPos + Width) > DeskTopWidth)
        xPos += DeskTopWidth - (xPos + Width);

    if ((yPos + Height) > DeskTopHeight)
        yPos += DeskTopHeight - (yPos + Height);

    MoveWindow(hWnd,
               xPos, yPos,
               Width, Height,
               TRUE);
}

static
INT_PTR
CALLBACK
CharMapDlgProc(HWND hDlg,
               UINT Message,
               WPARAM wParam,
               LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:
        {
            DWORD evMask;

            FillFontStyleComboList(GetDlgItem(hDlg,
                                              IDC_FONTCOMBO));

            ChangeMapFont(hDlg);

            // Configure Richedi control for sending notification changes.
            evMask = SendDlgItemMessage(hDlg, IDC_TEXTBOX, EM_GETEVENTMASK, 0, 0);
            evMask |= ENM_CHANGE;
            SendDlgItemMessage(hDlg, IDC_TEXTBOX, EM_SETEVENTMASK, 0, (LPARAM)evMask);

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_FONTMAP:
                    switch (HIWORD(wParam))
                    {
                        case FM_SETCHAR:
                            AddCharToSelection(hDlg, LOWORD(lParam));
                            break;
                    }
                    break;

                case IDC_FONTCOMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        ChangeMapFont(hDlg);
                    }
                    break;

                case IDC_SELECT:
                    AddCharToSelection(hDlg, 0);
                    break;

                case IDC_TEXTBOX:
                    switch (HIWORD(wParam)) {
                    case EN_CHANGE:
                        if (GetWindowTextLength(GetDlgItem(hDlg, IDC_TEXTBOX)) == 0)
                            EnableWindow(GetDlgItem(hDlg, IDC_COPY), FALSE);
                        else
                            EnableWindow(GetDlgItem(hDlg, IDC_COPY), TRUE);
                        break;
                    }
                    break;

                case IDC_COPY:
                    CopyCharacters(hDlg);
                    break;

                case IDC_CHECK_ADVANCED:
                    UpdateSettings(hDlg);
                    ChangeView(GetParent(hDlg));
                    break;
            }
        }
        break;

        default:
            break;
    }

    return FALSE;
}

static
INT_PTR
CALLBACK
AdvancedDlgProc(HWND hDlg,
               UINT Message,
               WPARAM wParam,
               LPARAM lParam)
{
    switch(Message)
    {
        case WM_INITDIALOG:
            return TRUE;

        default:
            return FALSE;
    }

    return FALSE;
}

static int
OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HMENU hSysMenu;
    WCHAR lpAboutText[256];

    hCharmapDlg = CreateDialog(hInstance,
                               MAKEINTRESOURCE(IDD_CHARMAP),
                               hWnd,
                               CharMapDlgProc);

    hAdvancedDlg = CreateDialog(hInstance,
                               MAKEINTRESOURCE(IDD_ADVANCED),
                               hWnd,
                               AdvancedDlgProc);

    hStatusWnd = CreateWindow(STATUSCLASSNAME,
                              NULL,
                              WS_CHILD | WS_VISIBLE,
                              0, 0, 0, 0,
                              hWnd,
                              (HMENU)IDD_STATUSBAR,
                              hInstance,
                              NULL);

    // Set the status bar for multiple parts output
    SendMessage(hStatusWnd, SB_SIMPLE, (WPARAM)FALSE, (LPARAM)0);

    ChangeView(hWnd);

    hSysMenu = GetSystemMenu(hWnd, FALSE);

    if (hSysMenu != NULL)
    {
        if (LoadStringW(hInstance, IDS_ABOUT, lpAboutText, SIZEOF(lpAboutText)))
        {
            AppendMenuW(hSysMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hSysMenu, MF_STRING, ID_ABOUT, lpAboutText);
        }
    }

    return 0;
}

static LRESULT CALLBACK
PanelWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        return OnCreate(hWnd, wParam, lParam);

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_SIZE:
        SendMessage(hStatusWnd, msg, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SYSCOMMAND:
        switch(wParam) {
        case ID_ABOUT:
            ShowAboutDlg(hWnd);
            break;
        }
        break;

    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static HWND
InitInstance(HINSTANCE hInst)
{
    WCHAR       szClass[] = L"CharMap";
    WCHAR       szTitle[256];
    WNDCLASSEXW wc;
    HWND        hWnd;

    LoadStringW(hInst, IDS_TITLE, szTitle, SIZEOF(szTitle));

    hSmIcon = LoadImage(hInstance,
                        MAKEINTRESOURCE(IDI_ICON),
                        IMAGE_ICON,
                        16,
                        16,
                        0);

    hBgIcon = LoadImage(hInstance,
                        MAKEINTRESOURCE(IDI_ICON),
                        IMAGE_ICON,
                        32,
                        32,
                        0);

    // Create workspace
    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = PanelWndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = hBgIcon;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClass;
    wc.hIconSm       = hSmIcon;

    RegisterClassExW(&wc);

    hWnd = CreateWindowW(
            szClass,
            szTitle,
            WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            hInst,
            NULL);

    if (hWnd != NULL)
    {
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }

    return hWnd;
}

INT
WINAPI
wWinMain(HINSTANCE hInst,
         HINSTANCE hPrev,
         LPWSTR Cmd,
         int iCmd)
{
    INITCOMMONCONTROLSEX iccx;
    INT Ret = 1;
    HMODULE hRichEd20;
    MSG Msg;

    hInstance = hInst;

    iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccx.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&iccx);

    if (RegisterMapClasses(hInstance))
    {
        hRichEd20 = LoadLibraryW(L"RICHED20.DLL");

        if (hRichEd20 != NULL)
        {
            InitInstance(hInst);

            for (;;)
            {
                if (GetMessage(&Msg, NULL, 0, 0) <= 0)
                {
                    Ret = Msg.wParam;
                    break;
                }

                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }

            FreeLibrary(hRichEd20);
        }
        UnregisterMapClasses(hInstance);
    }

    return Ret;
}
