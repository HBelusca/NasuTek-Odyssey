/*
 * Copyright 2004, 2006 Martin Fuchs
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
 // favorites.cpp
 //
 // Martin Fuchs, 04.04.2004
 //


#include <precomp.h>

#include "startmenu.h"


String DecodeURLString(const char* s)
{
	TCHAR buffer[BUFFER_LEN];
	LPTSTR o = buffer;

	for(const char* p=s; *p; ++p)
		if (*p == '%') {
			if (!strncmp(p+1, "20", 2)) {
				*o++ = ' ';
				p += 2;
			} else
				*o++ = *p;
		} else
			*o++ = *p;

	return String(buffer, o-buffer);
}


 /// read .URL file
bool Bookmark::read_url(LPCTSTR path)
{
	char line[BUFFER_LEN];

	tifstream in(path);

	while(in.good()) {
		in.getline(line, BUFFER_LEN);

		const char* p = line;
		while(isspace(*p))
			++p;

		const char* keyword = p;
		const char* eq = strchr(p, '=');

		if (eq) {
			const char* cont = eq + 1;
			while(isspace(*cont))
				++cont;

			if (!_strnicmp(keyword, "URL", 3))
				_url = DecodeURLString(cont);
			else if (!_strnicmp(keyword, "IconFile", 8))
				_icon_path = DecodeURLString(cont);
		}
	}

	return true;
}

 /// convert XBEL bookmark node
bool Bookmark::read(const_XMLPos& pos)
{
	_url = pos.get("href").c_str();

	if (pos.go_down("title")) {
		_name = pos->get_content();
		pos.back();
	}

	if (pos.go_down("desc")) {
		_description = pos->get_content();
		pos.back();
	}

	if (pos.go_down("info")) {
		const_XMLChildrenFilter metadata(pos, "metadata");

		for(const_XMLChildrenFilter::const_iterator it=metadata.begin(); it!=metadata.end(); ++it) {
			const XMLNode& node = **it;
			const_XMLPos sub_pos(&node);

			if (node.get("owner") == "ros-explorer") {
				if (sub_pos.go_down("icon")) {
					_icon_path = sub_pos.get("path").c_str();
					_icon_idx = XS_toi(sub_pos.get("index"));

					sub_pos.back();	// </icon>
				}
			}
		}

		pos.back();	// </metadata>
		pos.back();	// </info>
	}

	return !_url.empty();	// _url is mandatory.
}

 /// write XBEL bookmark node
void Bookmark::write(XMLPos& pos) const
{
	pos.create("bookmark");

	pos["href"] = _url.c_str();

	if (!_name.empty()) {
		pos.create("title");
		pos->set_content(_name);
		pos.back();
	}

	if (!_description.empty()) {
		pos.create("desc");
		pos->set_content(_description);
		pos.back();
	}

	if (!_icon_path.empty()) {
		pos.create("info");
		pos.create("metadata");
		pos["owner"] = "ros-explorer";
		pos.create("icon");
		pos["path"] = _icon_path.c_str();
		pos["index"].printf(XS_TEXT("%d"), _icon_idx);
		pos.back();	// </icon>
		pos.back();	// </metadata>
		pos.back();	// </info>
	}

	pos.back();
}


 /// read bookmark folder from XBEL formated XML tree
void BookmarkFolder::read(const_XMLPos& pos)
{
	if (pos.go_down("title")) {
		_name = pos->get_content();
		pos.back();
	}

	if (pos.go_down("desc")) {
		_description = pos->get_content();
		pos.back();
	}

	_bookmarks.read(pos);
}

 /// write bookmark folder content from XBEL formated XML tree
void BookmarkFolder::write(XMLPos& pos) const
{
	pos.create("folder");

	if (!_name.empty()) {
		pos.create("title");
		pos->set_content(_name);
		pos.back();
	}

	if (!_description.empty()) {
		pos.create("desc");
		pos->set_content(_description);
		pos.back();
	}

	_bookmarks.write(pos);
}


BookmarkNode::BookmarkNode()
 :	_type(BMNT_NONE)
{
	_pbookmark = NULL;
}

BookmarkNode::BookmarkNode(const Bookmark& bm)
 :	_type(BMNT_BOOKMARK)
{
	_pbookmark = new Bookmark(bm);
}

BookmarkNode::BookmarkNode(const BookmarkFolder& bmf)
 :	_type(BMNT_FOLDER)
{
	_pfolder = new BookmarkFolder(bmf);
}

BookmarkNode::BookmarkNode(const BookmarkNode& other)
 :	_type(other._type)
{
	if (other._type == BMNT_BOOKMARK)
		_pbookmark = new Bookmark(*other._pbookmark);
	else if (other._type == BMNT_FOLDER)
		_pfolder = new BookmarkFolder(*other._pfolder);
	else
		_pbookmark = NULL;
}

BookmarkNode::~BookmarkNode()
{
	if (_type == BMNT_BOOKMARK)
		delete _pbookmark;
	else if (_type == BMNT_FOLDER)
		delete _pfolder;
}

BookmarkNode& BookmarkNode::operator=(const Bookmark& bm)
{
	clear();

	_pbookmark = new Bookmark(bm);

	return *this;
}

BookmarkNode& BookmarkNode::operator=(const BookmarkFolder& bmf)
{
	clear();

	_pfolder = new BookmarkFolder(bmf);

	return *this;
}

BookmarkNode& BookmarkNode::operator=(const BookmarkNode& other)
{
	clear();

	_type = other._type;

	if (other._type == BMNT_BOOKMARK)
		_pbookmark = new Bookmark(*other._pbookmark);
	else if (other._type == BMNT_FOLDER)
		_pfolder = new BookmarkFolder(*other._pfolder);

	return *this;
}

void BookmarkNode::clear()
{
	if (_type == BMNT_BOOKMARK) {
		delete _pbookmark;
		_pbookmark = NULL;
	}
	else if (_type == BMNT_FOLDER) {
		delete _pfolder;
		_pfolder = NULL;
	}

	_type = BMNT_NONE;
}


 /// read bookmark list from XBEL formated XML tree
void BookmarkList::read(const_XMLPos& pos)
{
	const XMLNode::Children& children = pos->get_children();

	for(XMLNode::Children::const_iterator it=children.begin(); it!=children.end(); ++it) {
		const XMLNode& node = **it;
		const_XMLPos sub_pos(&node);

		if (node == "folder") {
			BookmarkFolder folder;

			folder.read(sub_pos);

			push_back(folder);
		} else if (node == "bookmark") {
			Bookmark bookmark;

			if (bookmark.read(sub_pos))
				push_back(bookmark);
		}
	}
}

 /// write bookmark list into XBEL formated XML tree
void BookmarkList::write(XMLPos& pos) const
{
	for(const_iterator it=begin(); it!=end(); ++it) {
		const BookmarkNode& node = *it;

		if (node._type == BookmarkNode::BMNT_FOLDER) {
			const BookmarkFolder& folder = *node._pfolder;

			folder.write(pos);

			pos.back();
		} else if (node._type == BookmarkNode::BMNT_BOOKMARK) {
			const Bookmark& bookmark = *node._pbookmark;

			if (!bookmark._url.empty())
				bookmark.write(pos);
		}
	}
}


 /// fill treeview control with bookmark tree content
void BookmarkList::fill_tree(HWND hwnd, HTREEITEM parent, HIMAGELIST himagelist, HDC hdc_wnd) const
{
	TV_INSERTSTRUCT tvi;

	tvi.hParent = parent;
	tvi.hInsertAfter = TVI_LAST;

	TV_ITEM& tv = tvi.item;
	tv.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;

	for(const_iterator it=begin(); it!=end(); ++it) {
		const BookmarkNode& node = *it;

		tv.lParam = (LPARAM)&node;

		if (node._type == BookmarkNode::BMNT_FOLDER) {
			const BookmarkFolder& folder = *node._pfolder;

			tv.pszText = (LPTSTR)folder._name.c_str();
			tv.iImage = 3;			// folder
			tv.iSelectedImage = 4;	// open folder
			HTREEITEM hitem = TreeView_InsertItem(hwnd, &tvi);

			folder._bookmarks.fill_tree(hwnd, hitem, himagelist, hdc_wnd);
		} else if (node._type == BookmarkNode::BMNT_BOOKMARK) {
			const Bookmark& bookmark = *node._pbookmark;

			tv.pszText = (LPTSTR)bookmark._name.c_str();
			tv.iImage = 1;			// bookmark
			tv.iSelectedImage = 2;	// selected bookmark

			if (!bookmark._icon_path.empty()) {
				const Icon& icon = g_Globals._icon_cache.extract(bookmark._icon_path, bookmark._icon_idx);

				if ((ICON_ID)icon != ICID_NONE)
					tv.iImage = tv.iSelectedImage = icon.add_to_imagelist(himagelist, hdc_wnd);
			}

			(void)TreeView_InsertItem(hwnd, &tvi);
		}
	}
}


 /// import Internet Explorer bookmarks from Favorites folder into bookmark list
void BookmarkList::import_IE_favorites(ShellDirectory& dir, HWND hwnd)
{
	TCHAR path[MAX_PATH], ext[_MAX_EXT];

	dir.smart_scan(SORT_NAME, SCAN_DONT_EXTRACT_ICONS);

	for(Entry*entry=dir._down; entry; entry=entry->_next) {
		if (entry->_shell_attribs & SFGAO_HIDDEN)	// ignore files like "desktop.ini"
			continue;

		String name;

		if (entry->_etype == ET_SHELL)
			name = dir._folder.get_name(static_cast<ShellEntry*>(entry)->_pidl);
		else
			name = entry->_display_name;

		if (entry->_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			BookmarkFolder new_folder;

			new_folder._name = DecodeXMLString(name);

			if (entry->_etype == ET_SHELL) {
				ShellDirectory new_dir(dir._folder, static_cast<ShellEntry*>(entry)->_pidl, hwnd);
				new_folder._bookmarks.import_IE_favorites(new_dir, hwnd);
			} else {
				entry->get_path(path, COUNTOF(path));
				ShellDirectory new_dir(GetDesktopFolder(), path, hwnd);
				new_folder._bookmarks.import_IE_favorites(new_dir, hwnd);
			}

			push_back(new_folder);
		} else {
			Bookmark bookmark;

			bookmark._name = DecodeXMLString(name);

			entry->get_path(path, COUNTOF(path));
			_tsplitpath_s(path, NULL, 0, NULL, 0, NULL, 0, ext, COUNTOF(ext));

			if (!_tcsicmp(ext, TEXT(".url"))) {
				bookmark.read_url(path);
				push_back(bookmark);
			} else {
				///@todo read shell links
				//assert(0);
			}
		}
	}
}


 /// read XBEL bookmark file
bool Favorites::read(LPCTSTR path)
{
	XMLDoc xbel;

	if (!xbel.read_file(path)) {
		if (!xbel._errors.empty())
			MessageBox(g_Globals._hwndDesktop, xbel._errors.str(),
						TEXT("ROS Explorer - reading bookmark file"), MB_OK);
	}

	const_XMLPos pos(&xbel);

	if (!pos.go_down("xbel"))
		return false;

	super::read(pos);

	pos.back();

	return true;
}

 /// write XBEL bookmark file
void Favorites::write(LPCTSTR path) const
{
	XMLDoc xbel;

	XMLPos pos(&xbel);
	pos.create("xbel");
	super::write(pos);
	pos.back();

	xbel._format._doctype._name = "xbel";
	xbel._format._doctype._public = "+//IDN python.org//DTD XML Bookmark Exchange Language 1.0//EN//XML";
	xbel._format._doctype._system = "http://www.python.org/topics/xml/dtds/xbel-1.0.dtd";

	xbel.write_file(path);
}

 /// import Internet Explorer bookmarks from Favorites folder
bool Favorites::import_IE_favorites(HWND hwnd)
{
	WaitCursor wait;

	StartMenuShellDirs dirs;

	try {
		dirs.push_back(ShellDirectory(GetDesktopFolder(), SpecialFolderPath(CSIDL_COMMON_FAVORITES, hwnd), hwnd));
		dirs.push_back(ShellDirectory(GetDesktopFolder(), SpecialFolderPath(CSIDL_FAVORITES, hwnd), hwnd));
	} catch(COMException&) {
	}

	for(StartMenuShellDirs::iterator it=dirs.begin(); it!=dirs.end(); ++it) {
		StartMenuDirectory& smd = *it;
		ShellDirectory& dir = smd._dir;

		try {
			super::import_IE_favorites(dir, hwnd);
		} catch(COMException&) {
		}
	}

	return true;
}
