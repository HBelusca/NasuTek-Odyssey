/*
 * Copyright 2003, 2004 Martin Fuchs
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


 //
 // Explorer and Desktop clone
 //
 // startmenu.h
 //
 // Martin Fuchs, 16.08.2003
 //


#define	CLASSNAME_STARTMENU		TEXT("ReactosStartmenuClass")
#define	TITLE_STARTMENU			TEXT("Start Menu")


#define	STARTMENU_WIDTH_MIN					120
#define	STARTMENU_LINE_HEIGHT(icon_size)	(icon_size+4)
#define	STARTMENU_SEP_HEIGHT(icon_size)		(STARTMENU_LINE_HEIGHT(icon_size)/2)
#define	STARTMENU_TOP_BTN_SPACE				8


 // private message constants
#define	PM_STARTMENU_CLOSED		(WM_APP+0x11)
#define	PM_STARTENTRY_LAUNCHED	(WM_APP+0x12)

#ifndef _LIGHT_STARTMENU
#define	PM_STARTENTRY_FOCUSED	(WM_APP+0x13)
#endif

#define	PM_UPDATE_ICONS			(WM_APP+0x14)
#define	PM_SELECT_ENTRY			(WM_APP+0x15)


 /// StartMenuDirectory is used to store the base directory of start menus.
struct StartMenuDirectory
{
	StartMenuDirectory(const ShellDirectory& dir, const String& ignore="")
	 :	_dir(dir), _ignore(ignore)
	{
	}

	~StartMenuDirectory()
	{
		_dir.free_subentries();
	}

	ShellDirectory _dir;
	String	_ignore;
};

typedef list<StartMenuDirectory> StartMenuShellDirs;
typedef set<Entry*> ShellEntrySet;

 /// structure holding information about one start menu entry
struct StartMenuEntry
{
	StartMenuEntry() : _icon_id(ICID_UNKNOWN) {}

	String	_title;
	ICON_ID	_icon_id;
	ShellEntrySet _entries;
};


extern int GetStartMenuBtnTextWidth(HDC hdc, LPCTSTR title, HWND hwnd);


#ifndef _LIGHT_STARTMENU

 /**
	StartMenuButton draws the face of a StartMenuCtrl button control.
 */
struct StartMenuButton : public OwnerdrawnButton
{
	typedef OwnerdrawnButton super;

	StartMenuButton(HWND hwnd, ICON_ID icon_id, bool hasSubmenu)
	 :	super(hwnd), _hIcon(hIcon), _hasSubmenu(hasSubmenu) {}

protected:
	LRESULT	WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
	virtual void DrawItem(LPDRAWITEMSTRUCT dis);

	ICON_ID	_icon_id;
	bool	_hasSubmenu;
};


 /**
	To create a Startmenu button control, construct a StartMenuCtrl object.
 */
struct StartMenuCtrl : public Button
{
	StartMenuCtrl(HWND parent, int x, int y, int w, LPCTSTR title,
					UINT id, HICON hIcon=0, bool hasSubmenu=false, DWORD style=WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, DWORD exStyle=0)
	 :	Button(parent, title, x, y, w, STARTMENU_LINE_HEIGHT(icon_size), id, style, exStyle)
	{
		*new StartMenuButton(_hwnd, hIcon, hasSubmenu);

		SetWindowFont(_hwnd, GetStockFont(DEFAULT_GUI_FONT), FALSE);
	}
};


 /// separator between start menu entries
struct StartMenuSeparator : public Static
{
	StartMenuSeparator(HWND parent, int x, int y, int w, DWORD style=WS_VISIBLE|WS_CHILD|WS_DISABLED|SS_ETCHEDHORZ, DWORD exStyle=0)
	 :	Static(parent, NULL, x, y+STARTMENU_SEP_HEIGHT(icon_size)/2-1, w, 2, -1, style, exStyle)
	{
	}
};

#endif


typedef list<ShellPath> StartMenuFolders;

 /// structor containing information for creating of start menus
struct StartMenuCreateInfo
{
	StartMenuCreateInfo() : _border_top(0) {}

	StartMenuFolders _folders;
	int		_border_top;
	String	_title;
	Window::CREATORFUNC_INFO _creator;
	void*	_info;
	String	_filter;
};

#define STARTMENU_CREATOR(WND_CLASS) WINDOW_CREATOR_INFO(WND_CLASS, StartMenuCreateInfo)

typedef map<int, StartMenuEntry> ShellEntryMap;


#ifdef _LIGHT_STARTMENU

struct SMBtnInfo
{
	SMBtnInfo(const StartMenuEntry& entry, int id, bool hasSubmenu=false, bool enabled=true)
	 :	_title(entry._title),
		_icon_id(entry._icon_id),
		_id(id),
		_hasSubmenu(hasSubmenu),
		_enabled(enabled)
	{
	}

	SMBtnInfo(LPCTSTR title, ICON_ID icon_id, int id, bool hasSubmenu=false, bool enabled=true)
	 :	_title(title),
		_icon_id(icon_id),
		_id(id),
		_hasSubmenu(hasSubmenu),
		_enabled(enabled)
	{
	}

	String	_title;
	ICON_ID	_icon_id;
	int		_id;
	bool	_hasSubmenu;
	bool	_enabled;
};

typedef vector<SMBtnInfo> SMBtnVector;

extern void DrawStartMenuButton(HDC hdc, const RECT& rect, LPCTSTR title, const SMBtnInfo& btn, bool has_focus, bool pushed, int icon_size);

#else

extern void DrawStartMenuButton(HDC hdc, const RECT& rect, LPCTSTR title, HICON hIcon,
								bool hasSubmenu, bool enabled, bool has_focus, bool pushed, int icon_size);

#endif


 /**
	Startmenu window.
	To create a start menu call its Create() function.
 */
struct StartMenu :
#ifdef _LIGHT_STARTMENU
	public ExtContextMenuHandlerT<OwnerDrawParent<Window> >
#else
	public ExtContextMenuHandlerT<OwnerDrawParent<DialogWindow> >
#endif
{
#ifdef _LIGHT_STARTMENU
	typedef ExtContextMenuHandlerT<OwnerDrawParent<Window> > super;
#else
	typedef ExtContextMenuHandlerT<OwnerDrawParent<DialogWindow> > super;
#endif

	StartMenu(HWND hwnd, int icon_size=ICON_SIZE_SMALL);
	StartMenu(HWND hwnd, const StartMenuCreateInfo& create_info, int icon_size=ICON_SIZE_SMALL);
	~StartMenu();

	static HWND Create(int x, int y, const StartMenuFolders&, HWND hwndParent, LPCTSTR title,
						CREATORFUNC_INFO creator=s_def_creator, void* info=NULL, const String& filter="");
	static CREATORFUNC_INFO s_def_creator;

protected:
	 // overridden member functions
	LRESULT	Init(LPCREATESTRUCT pcs);
	LRESULT	WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
	int		Command(int id, int code);

	 // window class
	static BtnWindowClass& GetWndClasss();

	 // data members
	int		_next_id;
	ShellEntryMap _entries;
	StartMenuShellDirs _dirs;

	int		_submenu_id;
	WindowHandle _submenu;

	int		_border_left;	// left border in pixels
	int		_border_top;	// top border in pixels
	int		_bottom_max;	// limit display area for long start menus

	bool	_floating_btn;
	bool	_arrow_btns;

	POINT	_last_pos;
	enum SCROLL_MODE {SCROLL_NOT, SCROLL_UP, SCROLL_DOWN} _scroll_mode;
	int		_scroll_pos;
	int		_invisible_lines;

	StartMenuCreateInfo _create_info;	// copy of the original create info

	int		_icon_size;

#ifdef _LIGHT_STARTMENU
	SMBtnVector _buttons;
	int		_selected_id;
	LPARAM	_last_mouse_pos;

	void	ResizeToButtons();
	int		ButtonHitTest(POINT pt);
	void	InvalidateSelection();
	const SMBtnInfo* GetButtonInfo(int id) const;
	bool	SelectButton(int id, bool open_sub=true);
	bool	SelectButtonIndex(int idx, bool open_sub=true);
	int		GetSelectionIndex();
	virtual void ProcessKey(int vk);
	bool	Navigate(int step);
	bool	OpenSubmenu(bool select_first=false);
	bool	JumpToNextShortcut(char c);
#endif

	 // member functions
	void	ResizeButtons(int cx);

	virtual void AddEntries();

	ShellEntryMap::iterator AddEntry(const String& title, ICON_ID icon_id, Entry* entry);
	ShellEntryMap::iterator AddEntry(const String& title, ICON_ID icon_id=ICID_NONE, int id=-1);
	ShellEntryMap::iterator AddEntry(const ShellFolder folder, ShellEntry* entry);
	ShellEntryMap::iterator AddEntry(const ShellFolder folder, Entry* entry);

	void	AddShellEntries(const ShellDirectory& dir, int max=-1, const String& ignore="");

	void	AddButton(LPCTSTR title, ICON_ID icon_id=ICID_NONE, bool hasSubmenu=false, int id=-1, bool enabled=true);
	void	AddSeparator();

	bool	CloseSubmenus() {return CloseOtherSubmenus();}
	bool	CloseOtherSubmenus(int id=0);
	void	CreateSubmenu(int id, LPCTSTR title, CREATORFUNC_INFO creator=s_def_creator, void*info=NULL);
	bool	CreateSubmenu(int id, int folder, LPCTSTR title, CREATORFUNC_INFO creator=s_def_creator, void*info=NULL);
	bool	CreateSubmenu(int id, int folder1, int folder2, LPCTSTR title, CREATORFUNC_INFO creator=s_def_creator, void*info=NULL);
	void	CreateSubmenu(int id, const StartMenuFolders& new_folders, LPCTSTR title, CREATORFUNC_INFO creator=s_def_creator, void*info=NULL);
	void	ActivateEntry(int id, const ShellEntrySet& entries);
	virtual void CloseStartMenu(int id=0);

	bool	GetButtonRect(int id, PRECT prect) const;

	void	DrawFloatingButton(HDC hdc);
	void	GetFloatingButtonRect(LPRECT prect);
	void	GetArrowButtonRects(LPRECT prect_up, LPRECT prect_down, int icon_size);

	void	DrawArrows(HDC hdc, int icon_size);

	void	Paint(PaintCanvas& canvas);
	void	UpdateIcons(/*int idx*/);
};


 // declare shell32's "Run..." dialog export function
typedef	void (WINAPI* RUNFILEDLG)(HWND hwndOwner, HICON hIcon, LPCSTR lpstrDirectory, LPCSTR lpstrTitle, LPCSTR lpstrDescription, UINT uFlags);

 //
 // Flags for RunFileDlg
 //

#define	RFF_NOBROWSE		0x01	// Removes the browse button.
#define	RFF_NODEFAULT		0x02	// No default item selected.
#define	RFF_CALCDIRECTORY	0x04	// Calculates the working directory from the file name.
#define	RFF_NOLABEL			0x08	// Removes the edit box label.
#define	RFF_NOSEPARATEMEM	0x20	// Removes the Separate Memory Space check box (Windows NT only).


 // declare more previously undocumented shell32 functions
typedef	void (WINAPI* EXITWINDOWSDLG)(HWND hwndOwner);
typedef	int (WINAPI* LOGOFFWINDOWSDIALOG)(UINT flags);
typedef	int (WINAPI* RESTARTWINDOWSDLG)(HWND hwndOwner, LPCWSTR reason, UINT flags);
typedef	int (WINAPI* RESTARTWINDOWSDLGEX)(HWND hWndOwner, LPCWSTR lpwstrReason, DWORD uFlags, DWORD uReason);
typedef	BOOL (WINAPI* SHFINDFILES)(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlSavedSearch);
typedef	BOOL (WINAPI* SHFINDCOMPUTER)(LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlSavedSearch);


 /// Handling of standard start menu commands
struct StartMenuHandler : public StartMenu
{
	typedef StartMenu super;

	StartMenuHandler(HWND hwnd, int icon_size=ICON_SIZE_SMALL)
	 :	super(hwnd, icon_size)
	{
	}

	StartMenuHandler(HWND hwnd, const StartMenuCreateInfo& create_info, int icon_size=ICON_SIZE_SMALL)
	 :	super(hwnd, create_info, icon_size)
	{
	}

protected:
	int		Command(int id, int code);

	static void	ShowLaunchDialog(HWND hwndOwner);
	static void	ShowLogoffDialog(HWND hwndOwner);
    static void	ShowRestartDialog(HWND hwndOwner, UINT flags);
	static void	ShowSearchDialog();
	static void	ShowSearchComputer();
};


struct StartMenuRootCreateInfo
{
	int	_icon_size;
};


 /// Startmenu root window
struct StartMenuRoot : public StartMenuHandler
{
	typedef StartMenuHandler super;

	StartMenuRoot(HWND hwnd, const StartMenuRootCreateInfo& info);

	static HWND Create(HWND hwndDesktopBar, int icon_size);
	void	TrackStartmenu();

	HWND	_hwndStartButton;

protected:
	LRESULT	Init(LPCREATESTRUCT pcs);
	LRESULT	WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);

	SIZE	_logo_size;

	virtual void AddEntries();
	virtual void ProcessKey(int vk);

	void	Paint(PaintCanvas& canvas);
	void	CloseStartMenu(int id=0);

	void	ReadLogoSize();
	UINT	GetLogoResId();
};


 /// Settings sub-startmenu
struct SettingsMenu : public StartMenuHandler
{
	typedef StartMenuHandler super;

	SettingsMenu(HWND hwnd, const StartMenuCreateInfo& create_info)
	 :	super(hwnd, create_info)
	{
	}

protected:
	virtual void AddEntries();
};


 /// "Browse Files..." sub-start menu
struct BrowseMenu : public StartMenuHandler
{
	typedef StartMenuHandler super;

	BrowseMenu(HWND hwnd, const StartMenuCreateInfo& create_info)
	 :	super(hwnd, create_info)
	{
	}

protected:
	virtual void AddEntries();
};


 /// Search sub-startmenu
struct SearchMenu : public StartMenuHandler
{
	typedef StartMenuHandler super;

	SearchMenu(HWND hwnd, const StartMenuCreateInfo& create_info)
	 :	super(hwnd, create_info)
	{
	}

protected:
	virtual void AddEntries();
};


#define	RECENT_DOCS_COUNT	20	///@todo read max. count of entries from registry

 /// "Recent Files" sub-start menu
struct RecentStartMenu : public StartMenu
{
	typedef StartMenu super;

	RecentStartMenu(HWND hwnd, const StartMenuCreateInfo& create_info)
	 :	super(hwnd, create_info)
	{
	}

protected:
	virtual void AddEntries();
};


#ifndef _SHELL32_FAVORITES

typedef map<int, BookmarkNode> BookmarkMap;

 /// Bookmarks sub-startmenu
struct FavoritesMenu : public StartMenu
{
	typedef StartMenu super;

	FavoritesMenu(HWND hwnd, const StartMenuCreateInfo& create_info)
	 :	super(hwnd, create_info),
		_bookmarks(*(BookmarkList*)create_info._info)
	{
	}

protected:
	virtual int Command(int id, int code);
	virtual void AddEntries();

	BookmarkList _bookmarks;
	BookmarkMap	_entries;
};

#endif
