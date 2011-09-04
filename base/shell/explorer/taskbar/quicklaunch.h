/*
 * Copyright 2003, 2004, 2005 Martin Fuchs
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
 // quicklaunch.h
 //
 // Martin Fuchs, 22.08.2003
 //


#define	QUICKLAUNCH_WIDTH_DEF	70
#define	QUICKLAUNCH_WIDTH_DEF	70

#define	IDW_QUICKLAUNCHBAR	101

#define	PM_UPDATE_DESKTOP	(WM_APP+0x23)

#define	IDC_FIRST_QUICK_ID	0x4000


#define	QUICKLAUNCH_FOLDER	TEXT("Microsoft\\Internet Explorer\\Quick Launch")


 /// quick launch bar shortcut
struct QuickLaunchEntry
{
	QuickLaunchEntry();

	HBITMAP	_hbmp;
	String	_title;
	Entry*	_entry;
};

 /// map for managing the task bar buttons
struct QuickLaunchMap : public map<int, QuickLaunchEntry>
{
	~QuickLaunchMap();
};


 /// quick launch bar window
struct QuickLaunchBar : public ExtContextMenuHandlerT<SubclassedWindow>
{
	typedef ExtContextMenuHandlerT<SubclassedWindow> super;

	QuickLaunchBar(HWND hwnd);
	~QuickLaunchBar();

	static HWND	Create(HWND hwndParent);

protected:
	LRESULT WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam);
	int		Command(int id, int code);
	int		Notify(int id, NMHDR* pnmh);

	ShellDirectory*	_dir;

	int				_next_id;
	QuickLaunchMap	_entries;
	int				_btn_dist;
	int				_size;

	void	AddShortcuts();
	void	AddButton(int id, HBITMAP hbmp, LPCTSTR name, Entry* entry, int flags=TBSTATE_ENABLED);
	void	UpdateDesktopButtons(int desktop_idx);
};
