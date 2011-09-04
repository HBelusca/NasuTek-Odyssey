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
 // Explorer clone
 //
 // filechild.cpp
 //
 // Martin Fuchs, 23.07.2003
 //


#include <precomp.h>

#include "ntobjfs.h"
#include "regfs.h"
#include "fatfs.h"

#include "../resource.h"


FileChildWndInfo::FileChildWndInfo(HWND hmdiclient, LPCTSTR path, ENTRY_TYPE etype)
 :	super(hmdiclient),
	_etype(etype)
{
#ifndef _NO_WIN_FS
	if (etype == ET_UNKNOWN)
#ifdef __WINE__
		if (*path == '/')
			_etype = ET_UNIX;
		else
#endif
			_etype = ET_WINDOWS;
#endif

	_path = path;

	_pos.length = sizeof(WINDOWPLACEMENT);
	_pos.flags = 0;
	_pos.showCmd = SW_SHOWNORMAL;
	_pos.rcNormalPosition.left = CW_USEDEFAULT;
	_pos.rcNormalPosition.top = CW_USEDEFAULT;
	_pos.rcNormalPosition.right = CW_USEDEFAULT;
	_pos.rcNormalPosition.bottom = CW_USEDEFAULT;

	_open_mode = OWM_EXPLORE|OWM_DETAILS;
}


ShellChildWndInfo::ShellChildWndInfo(HWND hmdiclient, LPCTSTR path, const ShellPath& root_shell_path)
 :	FileChildWndInfo(hmdiclient, path, ET_SHELL),
	_shell_path(path&&*path? path: root_shell_path),
	_root_shell_path(root_shell_path)
{
}


NtObjChildWndInfo::NtObjChildWndInfo(HWND hmdiclient, LPCTSTR path)
 :	FileChildWndInfo(hmdiclient, path, ET_NTOBJS)
{
}


RegistryChildWndInfo::RegistryChildWndInfo(HWND hmdiclient, LPCTSTR path)
 :	FileChildWndInfo(hmdiclient, path, ET_REGISTRY)
{
}


FATChildWndInfo::FATChildWndInfo(HWND hmdiclient, LPCTSTR path)
 :	FileChildWndInfo(hmdiclient, path, ET_FAT)
{
}


WebChildWndInfo::WebChildWndInfo(HWND hmdiclient, LPCTSTR url)
 :	FileChildWndInfo(hmdiclient, url, ET_WEB)
{
}


INT_PTR CALLBACK ExecuteDialog::WndProc(HWND hwnd, UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	static struct ExecuteDialog* dlg;

	switch(nmsg) {
	  case WM_INITDIALOG:
		dlg = (struct ExecuteDialog*) lparam;
		return 1;

	  case WM_COMMAND: {
		int id = (int)wparam;

		if (id == IDOK) {
			GetWindowText(GetDlgItem(hwnd, 201), dlg->cmd, COUNTOF(dlg->cmd));
			dlg->cmdshow = Button_GetState(GetDlgItem(hwnd,214))&BST_CHECKED?
											SW_SHOWMINIMIZED: SW_SHOWNORMAL;
			EndDialog(hwnd, id);
		} else if (id == IDCANCEL)
			EndDialog(hwnd, id);

		return 1;}
	}

	return 0;
}


 // FileChildWindow

FileChildWindow::FileChildWindow(HWND hwnd, const FileChildWndInfo& info)
 :	super(hwnd, info)
{
	CONTEXT("FileChildWindow::FileChildWindow()");

	TCHAR drv[_MAX_DRIVE+1];
	Entry* entry = NULL;

	_left = NULL;
	_right = NULL;

	switch(info._etype) {
#ifdef __WINE__
	  case ET_UNIX:
		_root._drive_type = GetDriveType(info._path);
		_root._sort_order = SORT_NAME;

		_tsplitpath(info._path, drv, NULL, NULL, NULL);
		lstrcat(drv, TEXT("/"));
		lstrcpy(_root._volname, TEXT("root fs"));
		_root._fs_flags = 0;
		lstrcpy(_root._fs, TEXT("unixfs"));
		lstrcpy(_root._path, TEXT("/"));
		_root._entry = new UnixDirectory(_root._path);
		entry = _root.read_tree(info._path+_tcslen(_root._path));
		break;
#endif

	  case ET_NTOBJS:
		_root._drive_type = DRIVE_UNKNOWN;
		_root._sort_order = SORT_NAME;

		_tsplitpath_s(info._path, drv, COUNTOF(drv), NULL, 0, NULL, 0, NULL, 0);
		lstrcat(drv, TEXT("\\"));
		lstrcpy(_root._volname, TEXT("NT Object Namespace"));
		lstrcpy(_root._fs, TEXT("NTOBJ"));
		lstrcpy(_root._path, drv);
		_root._entry = new NtObjDirectory(_root._path);
		entry = _root.read_tree(info._path+_tcslen(_root._path));
		break;

	  case ET_REGISTRY:
		_root._drive_type = DRIVE_UNKNOWN;
		_root._sort_order = SORT_NONE;

		_tsplitpath_s(info._path, drv, COUNTOF(drv), NULL, 0, NULL, 0, NULL, 0);
		lstrcat(drv, TEXT("\\"));
		lstrcpy(_root._volname, TEXT("Registry"));
		lstrcpy(_root._fs, TEXT("Registry"));
		lstrcpy(_root._path, drv);
		_root._entry = new RegistryRoot();
		entry = _root.read_tree(info._path+_tcslen(_root._path));
		break;

	  case ET_FAT: {
		_root._drive_type = DRIVE_UNKNOWN;
		_root._sort_order = SORT_NONE;

		_tsplitpath_s(info._path, drv, COUNTOF(drv), NULL, 0, NULL, 0, NULL, 0);
		lstrcat(drv, TEXT("\\"));
		lstrcpy(_root._volname, TEXT("FAT XXX"));	//@@
		lstrcpy(_root._fs, TEXT("FAT"));
		lstrcpy(_root._path, drv);
		FATDrive* drive = new FATDrive(TEXT("c:/odyssey-emu/c.img"));	//TEXT("\\\\.\\F:"));	//@@

		if (drive->_hDrive != INVALID_HANDLE_VALUE) {
			_root._entry = drive;
			entry = _root.read_tree(info._path+_tcslen(_root._path));
		}
		break;}

#ifndef _NO_WIN_FS
	  default:	// ET_WINDOWS
		_root._drive_type = GetDriveType(info._path);
		_root._sort_order = SORT_NAME;

		_tsplitpath_s(info._path, drv, COUNTOF(drv), NULL, 0, NULL, 0, NULL, 0);
		lstrcat(drv, TEXT("\\"));
		GetVolumeInformation(drv, _root._volname, _MAX_FNAME, 0, 0, &_root._fs_flags, _root._fs, COUNTOF(_root._fs));
		lstrcpy(_root._path, drv);
		_root._entry = new WinDirectory(_root._path);
		entry = _root.read_tree(info._path+_tcslen(_root._path));
		break;
#else
	default:
#endif

	  case ET_SHELL: {	//@@ separate FileChildWindow into ShellChildWindow, WinChildWindow, UnixChildWindow ?
		_root._drive_type = DRIVE_UNKNOWN;
		_root._sort_order = SORT_NAME;

		lstrcpy(drv, TEXT("\\"));
		lstrcpy(_root._volname, TEXT("Desktop"));
		_root._fs_flags = 0;
		lstrcpy(_root._fs, TEXT("Shell"));

		_root._entry = new ShellDirectory(GetDesktopFolder(), DesktopFolderPath(), hwnd);
		const ShellChildWndInfo& shell_info = static_cast<const ShellChildWndInfo&>(info);
		entry = _root.read_tree(&*shell_info._shell_path);
		break;}
	}

	if (_root._entry) {
		if (info._etype != ET_SHELL)
			wsprintf(_root._entry->_data.cFileName, TEXT("%s - %s"), drv, _root._fs);
	/*@@else
			lstrcpy(_root._entry->_data.cFileName, TEXT("GetDesktopFolder"));*/

		_root._entry->_data.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;


		///@todo use OWM_ROOTED flag

		if (info._open_mode & OWM_EXPLORE)	///@todo Is not-explore-mode for FileChildWindow completely implemented?
			_left_hwnd = *(_left=new Pane(_hwnd, IDW_TREE_LEFT, IDW_HEADER_LEFT, _root._entry, true, COL_CONTENT));

		_right_hwnd = *(_right=new Pane(_hwnd, IDW_TREE_RIGHT, IDW_HEADER_RIGHT, NULL, false,
										COL_TYPE|COL_SIZE|COL_DATE|COL_TIME|COL_ATTRIBUTES|COL_INDEX|COL_LINKS|COL_CONTENT));
	}

	_header_wdths_ok = false;

	if (!_left_hwnd && !_right_hwnd)
		return;

	if (entry)
		set_curdir(entry);
	else if (_root._entry)
		set_curdir(_root._entry);

	if (_left_hwnd) {
		int idx = ListBox_FindItemData(_left_hwnd, ListBox_GetCurSel(_left_hwnd), _left->_cur);
		ListBox_SetCurSel(_left_hwnd, idx);
		//SetFocus(_left_hwnd);
	}

	 // store path into history
	if (info._path && *info._path)
		_url_history.push(info._path);
}


void FileChildWindow::set_curdir(Entry* entry)
{
	CONTEXT("FileChildWindow::set_curdir()");

	_path[0] = TEXT('\0');

	_left->_cur = entry;
	_right->_root = entry&&entry->_down? entry->_down: entry;
	_right->_cur = entry;

	if (entry) {
		WaitCursor wait;

		if (!entry->_scanned)
			scan_entry(entry);
		else {
			HiddenWindow hide(_right_hwnd);

			ListBox_ResetContent(_right_hwnd);
			_right->insert_entries(entry->_down);

			_right->calc_widths(false);	///@todo make configurable (This call takes really _very_ long compared to all other processing!)

			_right->set_header();
		}

		entry->get_path(_path, COUNTOF(_path));
	}

	if (_hwnd)	// only change window title if the window already exists
		SetWindowText(_hwnd, _path);

	if (_path[0])
	{
		if (SetCurrentDirectory(_path))
			set_url(_path);	//set_url(FmtString(TEXT("file://%s"), _path));
		else
			_path[0] = TEXT('\0');
	}
}


 // expand a directory entry

bool FileChildWindow::expand_entry(Entry* dir)
{
	int idx;
	Entry* p;

	if (!dir || dir->_expanded || !dir->_down)
		return false;

	p = dir->_down;

	if (p->_data.cFileName[0]=='.' && p->_data.cFileName[1]=='\0' && p->_next) {
		p = p->_next;

		if (p->_data.cFileName[0]=='.' && p->_data.cFileName[1]=='.' &&
				p->_data.cFileName[2]=='\0' && p->_next)
			p = p->_next;
	}

	 // no subdirectories ?
	if (!(p->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&	// not a directory?
		!p->_down)	// not a file with NTFS sub-streams?
		return FALSE;

	idx = ListBox_FindItemData(_left_hwnd, 0, dir);

	dir->_expanded = true;

	 // insert entries in left pane
	HiddenWindow hide(_left_hwnd);

	_left->insert_entries(p, idx);

	if (!_header_wdths_ok) {
		if (_left->calc_widths(false)) {
			_left->set_header();

			_header_wdths_ok = true;
		}
	}

	return true;
}


void FileChildWindow::collapse_entry(Pane* pane, Entry* dir)
{
	int idx = ListBox_FindItemData(*pane, 0, dir);

	SendMessage(*pane, WM_SETREDRAW, FALSE, 0);	//ShowWindow(*pane, SW_HIDE);

	 // hide sub entries
	for(;;) {
		LRESULT res = ListBox_GetItemData(*pane, idx+1);
		Entry* sub = (Entry*) res;

		if (res==LB_ERR || !sub || sub->_level<=dir->_level)
			break;

		ListBox_DeleteString(*pane, idx+1);
	}

	dir->_expanded = false;

	SendMessage(*pane, WM_SETREDRAW, TRUE, 0);	//ShowWindow(*pane, SW_SHOW);
}


FileChildWindow* FileChildWindow::create(const FileChildWndInfo& info)
{
	CONTEXT("FileChildWindow::create()");

	MDICREATESTRUCT mcs;

	mcs.szClass = CLASSNAME_WINEFILETREE;
	mcs.szTitle = (LPTSTR)info._path;
	mcs.hOwner	= g_Globals._hInstance;
	mcs.x		= info._pos.rcNormalPosition.left;
	mcs.y		= info._pos.rcNormalPosition.top;
	mcs.cx		= info._pos.rcNormalPosition.right - info._pos.rcNormalPosition.left;
	mcs.cy		= info._pos.rcNormalPosition.bottom - info._pos.rcNormalPosition.top;
	mcs.style	= 0;
	mcs.lParam	= 0;

	FileChildWindow* child = static_cast<FileChildWindow*>(
		create_mdi_child(info, mcs, WINDOW_CREATOR_INFO(FileChildWindow,FileChildWndInfo)));

	if (!child->_left_hwnd && !child->_right_hwnd) {
		SendMessage(info._hmdiclient, WM_MDIDESTROY, (WPARAM)child->_hwnd, 0);
		MessageBox(info._hmdiclient, TEXT("Error opening child window"), TEXT("ROS Explorer"), MB_OK);
	}

	return child;
}


void FileChildWindow::resize_children(int cx, int cy)
{
	HDWP hdwp = BeginDeferWindowPos(4);
	RECT rt;

	rt.left   = 0;
	rt.top    = 0;
	rt.right  = cx;
	rt.bottom = cy;

	cx = _split_pos + SPLIT_WIDTH/2;

	if (_left && _right) {
		WINDOWPOS wp;
		HD_LAYOUT hdl;

		hdl.prc   = &rt;
		hdl.pwpos = &wp;

		Header_Layout(_left->_hwndHeader, &hdl);

		hdwp = DeferWindowPos(hdwp, _left->_hwndHeader, wp.hwndInsertAfter,
							wp.x-1, wp.y, _split_pos-SPLIT_WIDTH/2+1, wp.cy, wp.flags);

		hdwp = DeferWindowPos(hdwp, _right->_hwndHeader, wp.hwndInsertAfter,
								rt.left+cx+1, wp.y, wp.cx-cx+2, wp.cy, wp.flags);
	}

	if (_left_hwnd)
		hdwp = DeferWindowPos(hdwp, _left_hwnd, 0, rt.left, rt.top, _split_pos-SPLIT_WIDTH/2-rt.left, rt.bottom-rt.top, SWP_NOZORDER|SWP_NOACTIVATE);

	if (_right_hwnd)
		hdwp = DeferWindowPos(hdwp, _right_hwnd, 0, rt.left+cx+1, rt.top, rt.right-cx, rt.bottom-rt.top, SWP_NOZORDER|SWP_NOACTIVATE);

	EndDeferWindowPos(hdwp);
}


LRESULT FileChildWindow::WndProc(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	switch(nmsg) {
		case WM_DRAWITEM: {
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lparam;
			Entry* entry = (Entry*) dis->itemData;

			if (dis->CtlID == IDW_TREE_LEFT) {
				_left->draw_item(dis, entry);
				return TRUE;
			} else if (dis->CtlID == IDW_TREE_RIGHT) {
				_right->draw_item(dis, entry);
				return TRUE;
			}

			goto def;}

		case WM_SIZE:
			if (wparam != SIZE_MINIMIZED)
				resize_children(LOWORD(lparam), HIWORD(lparam));
			return DefMDIChildProc(_hwnd, nmsg, wparam, lparam);

		case PM_GET_FILEWND_PTR:
			return (LRESULT)this;

		case WM_SETFOCUS: {
			TCHAR path[MAX_PATH];

			if (_left && _left->_cur) {
				_left->_cur->get_path(path, COUNTOF(path));
				SetCurrentDirectory(path);
			}

			SetFocus(_focus_pane? _right_hwnd: _left_hwnd);
			goto def;}

		case PM_DISPATCH_COMMAND: {
			Pane* pane = GetFocus()==_left_hwnd? _left: _right;

			switch(LOWORD(wparam)) {
			  case ID_WINDOW_NEW: {CONTEXT("FileChildWindow PM_DISPATCH_COMMAND ID_WINDOW_NEW");
				if (_root._entry->_etype == ET_SHELL)
					FileChildWindow::create(ShellChildWndInfo(GetParent(_hwnd)/*_hmdiclient*/, _path, DesktopFolderPath()));
				else
					FileChildWindow::create(FileChildWndInfo(GetParent(_hwnd)/*_hmdiclient*/, _path));
				break;}

			  case ID_REFRESH: {CONTEXT("ID_REFRESH");
				refresh();
				break;}

			  case ID_ACTIVATE: {CONTEXT("ID_ACTIVATE");
				activate_entry(pane);
				break;}

			  default:
				if (pane->command(LOWORD(wparam)))
					return TRUE;
				else
					return super::WndProc(nmsg, wparam, lparam);
			}

			return TRUE;}

		case WM_CONTEXTMENU: {
			 // first select the current item in the listbox
			HWND hpanel = (HWND) wparam;
			POINT pt;
			pt.x = LOWORD(lparam);
			pt.y = HIWORD(lparam);
			POINT pt_screen = pt;
			ScreenToClient(hpanel, &pt);
			SendMessage(hpanel, WM_LBUTTONDOWN, 0, MAKELONG(pt.x, pt.y));
			SendMessage(hpanel, WM_LBUTTONUP, 0, MAKELONG(pt.x, pt.y));

			 // now create the popup menu using shell namespace and IContextMenu
			Pane* pane = GetFocus()==_left_hwnd? _left: _right;
			int idx = ListBox_GetCurSel(*pane);
			if (idx != -1) {
				Entry* entry = (Entry*) ListBox_GetItemData(*pane, idx);

				HRESULT hr = entry->do_context_menu(_hwnd, pt_screen, _cm_ifs);

				if (SUCCEEDED(hr))
					refresh();
				else
					CHECKERROR(hr);
			}
			break;}

		default: def:
			return super::WndProc(nmsg, wparam, lparam);
	}

	return 0;
}


void FileChildWindow::refresh()
{
	WaitCursor wait;
	bool expanded = _left->_cur->_expanded;

	scan_entry(_left->_cur);

	if (expanded)
		expand_entry(_left->_cur);
}


int FileChildWindow::Command(int id, int code)
{
	Pane* pane = GetFocus()==_left_hwnd? _left: _right;

	switch(code) {
	  case LBN_SELCHANGE: {
		int idx = ListBox_GetCurSel(*pane);
		Entry* entry = (Entry*) ListBox_GetItemData(*pane, idx);

		if (pane == _left)
			set_curdir(entry);
		else
			pane->_cur = entry;
		break;}

	  case LBN_DBLCLK:
		activate_entry(pane);
		break;
	}

	return 0;
}


void FileChildWindow::activate_entry(Pane* pane)	///@todo enable using RETURN key accelerator
{
	Entry* entry = pane->_cur;

	if (!entry)
		return;

	WaitCursor wait;

	if ((entry->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||	// a directory?
		entry->_down)	// a file with NTFS sub-streams?
	{
		int scanned_old = entry->_scanned;

		if (!scanned_old)
			scan_entry(entry);

		if (entry->_data.cFileName[0]==TEXT('.') && entry->_data.cFileName[1]==TEXT('\0'))
			return;

		if (entry->_data.cFileName[0]==TEXT('.') && entry->_data.cFileName[1]==TEXT('.') && entry->_data.cFileName[2]==TEXT('\0')) {
			entry = _left->_cur->_up;
			collapse_entry(_left, entry);
			goto focus_entry;
		} else if (entry->_expanded)
			collapse_entry(pane, _left->_cur);
		else {
			expand_entry(_left->_cur);

			if (!pane->_treePane) focus_entry: {
				int idx = ListBox_FindItemData(_left_hwnd, ListBox_GetCurSel(_left_hwnd), entry);
				ListBox_SetCurSel(_left_hwnd, idx);

				set_curdir(entry);
			}
		}

		if (!scanned_old) {
			pane->calc_widths(false);

			pane->set_header();
		}
	} else {
		entry->launch_entry(_hwnd);
	}
}


void FileChildWindow::scan_entry(Entry* entry)
{
	CONTEXT("FileChildWindow::scan_entry()");

	int idx = ListBox_GetCurSel(_left_hwnd);

	 // delete sub entries in left pane
	for(;;) {
		LRESULT res = ListBox_GetItemData(_left_hwnd, idx+1);
		Entry* sub = (Entry*) res;

		if (res==LB_ERR || !sub || sub->_level<=entry->_level)
			break;

		ListBox_DeleteString(_left_hwnd, idx+1);
	}

	 // empty right pane
	ListBox_ResetContent(_right_hwnd);

	 // release memory
	entry->free_subentries();
	entry->_expanded = false;

	 // read contents from disk
	entry->read_directory_base(_root._sort_order);	///@todo use modifyable sort order instead of fixed file system default

	 // insert found entries in right pane
	HiddenWindow hide(_right_hwnd);
	_right->insert_entries(entry->_down);

	_right->calc_widths(false);
	_right->set_header();

	_header_wdths_ok = false;
}


int FileChildWindow::Notify(int id, NMHDR* pnmh)
{
	return (pnmh->idFrom==IDW_HEADER_LEFT? _left: _right)->Notify(id, pnmh);
}


String FileChildWindow::jump_to_int(LPCTSTR url)
{
	String dir, fname;

	if (SplitFileSysURL(url, dir, fname)) {
		Entry* entry = NULL;

		 // call read_tree() to iterate through the hierarchy and open all folders to reach dir
		if (_root._entry)
			switch(_root._entry->_etype) {
			  case ET_SHELL: {	//@@ separate into FileChildWindow in ShellChildWindow, WinChildWindow, UnixChildWindow ?
				ShellPath shell_path(dir);
				entry = _root.read_tree(&*shell_path);
				break;}

#ifdef __WINE__
			  case ET_UNIX: {
				LPCTSTR path = dir;

				if (!_tcsicmp(path, _root._path, _tcslen(_root._path)))
					path += _tcslen(_root._path);

				entry = _root.read_tree(path);
				break;}
#endif

			  default: { // ET_NTOBJS, ET_REGISTRY, ET_FAT, ET_WINDOWS
				LPCTSTR path = dir;

				if (!_tcsnicmp(path, _root._path, _tcslen(_root._path)))
					path += _tcslen(_root._path);

				entry = _root.read_tree(path);
				break;}
			}

			if (entry) {
				 // refresh left pane entries
				HiddenWindow hide(_left_hwnd);

				ListBox_ResetContent(_left_hwnd);

				_left->insert_entries(_root._entry);

				if (!_header_wdths_ok) {
					if (_left->calc_widths(false)) {
						_left->set_header();

						_header_wdths_ok = true;
					}
				}

				set_curdir(entry);

				if (_left_hwnd) {
					int idx = ListBox_FindItemData(_left_hwnd, -1, entry);

					if (idx != -1) { // The item should always be found.
						ListBox_SetCurSel(_left_hwnd, idx);
						SetFocus(_left_hwnd);
					}
				}

				///@todo use fname

				return dir;	//FmtString(TEXT("file://%s"), (LPCTSTR)dir);
			}
	}

	return String();
}
