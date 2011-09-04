/*
 * Copyright 2004 Martin Fuchs
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
 // ntobjfs.cpp
 //
 // Martin Fuchs, 31.01.2004
 //


#include <precomp.h>

#include "regfs.h"


void RegDirectory::read_directory(int scan_flags)
{
	CONTEXT("RegDirectory::read_directory()");

	Entry* first_entry = NULL;
	int level = _level + 1;

	TCHAR buffer[MAX_PATH];

	_tcscpy(buffer, (LPCTSTR)_path);
	LPTSTR pname = buffer + _tcslen(buffer);
	int plen = MAX_PATH - _tcslen(buffer);

	HKEY hkey;

	if (!RegOpenKeyEx(_hKeyRoot, *buffer=='\\'?buffer+1:buffer, 0, STANDARD_RIGHTS_READ|KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS, &hkey)) {
		if (pname[-1] != '\\')
			*pname++ = '\\', plen--;

		TCHAR name[MAX_PATH], class_name[MAX_PATH];
		WIN32_FIND_DATA w32fd;
		Entry* last = NULL;
		RegEntry* entry;

		for(int idx=0; ; ++idx) {
			memset(&w32fd, 0, sizeof(WIN32_FIND_DATA));

			DWORD name_len = MAX_PATH;
			DWORD class_len = MAX_PATH;

			if (RegEnumKeyEx(hkey, idx, name, &name_len, 0, class_name, &class_len, &w32fd.ftLastWriteTime))
				break;

			w32fd.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			_tcsncpy(w32fd.cFileName, name, name_len);

			_tcscpy_s(pname, plen, name);
			entry = new RegDirectory(this, buffer, _hKeyRoot);

			memcpy(&entry->_data, &w32fd, sizeof(WIN32_FIND_DATA));

			if (class_len)
				entry->_type_name = _tcsdup(String(class_name, class_len));

			if (!first_entry)
				first_entry = entry;

			if (last)
				last->_next = entry;

			entry->_level = level;

			last = entry;
		}
/*
		TCHAR value[MAX_PATH];
		LONG value_len = sizeof(value);

		if (!RegQueryValue(hkey, NULL, value, &value_len) && value_len>1) {
			memset(&w32fd, 0, sizeof(WIN32_FIND_DATA));

			lstrcpy(w32fd.cFileName, TEXT("(Default)"));

			entry = new RegEntry(this);

			memcpy(&entry->_data, &w32fd, sizeof(WIN32_FIND_DATA));

			entry->_content = _tcsdup(value);

			if (!first_entry)
				first_entry = entry;

			if (last)
				last->_next = entry;

			entry->_level = level;

			last = entry;
		}
*/
		DWORD type;
		for(int idx=0; ; ++idx) {
			DWORD name_len = MAX_PATH;

			if (RegEnumValue(hkey, idx, name, &name_len, 0, &type, NULL, NULL))
				break;

			memset(&w32fd, 0, sizeof(WIN32_FIND_DATA));

			if (name[0])
				lstrcpy(w32fd.cFileName, name);
			else
				lstrcpy(w32fd.cFileName, TEXT("(Default)"));

			entry = new RegEntry(this);

			memcpy(&entry->_data, &w32fd, sizeof(WIN32_FIND_DATA));

			switch(type) {
			  case REG_NONE:						entry->_type_name = _tcsdup(TEXT("REG_NONE"));						break;
			  case REG_SZ:							entry->_type_name = _tcsdup(TEXT("REG_SZ"));						break;
			  case REG_EXPAND_SZ:					entry->_type_name = _tcsdup(TEXT("REG_EXPAND_SZ"));					break;
			  case REG_BINARY:						entry->_type_name = _tcsdup(TEXT("REG_BINARY"));					break;
			  case REG_DWORD:						entry->_type_name = _tcsdup(TEXT("REG_DWORD"));						break;
			  case REG_DWORD_BIG_ENDIAN:			entry->_type_name = _tcsdup(TEXT("REG_DWORD_BIG_ENDIAN"));			break;
			  case REG_LINK:						entry->_type_name = _tcsdup(TEXT("REG_LINK"));						break;
			  case REG_MULTI_SZ:					entry->_type_name = _tcsdup(TEXT("REG_MULTI_SZ"));					break;
			  case REG_RESOURCE_LIST:				entry->_type_name = _tcsdup(TEXT("REG_RESOURCE_LIST"));				break;
			  case REG_FULL_RESOURCE_DESCRIPTOR:	entry->_type_name = _tcsdup(TEXT("REG_FULL_RESOURCE_DESCRIPTOR"));	break;
			  case REG_RESOURCE_REQUIREMENTS_LIST:	entry->_type_name = _tcsdup(TEXT("REG_RESOURCE_REQUIREMENTS_LIST"));break;
			  case REG_QWORD:						entry->_type_name = _tcsdup(TEXT("REG_QWORD"));						break;
			}

			 ///@todo This can also be done in the RegEnumValue() call if we dynamically adjust the return buffer size.
			TCHAR value[MAX_PATH];
			DWORD value_len = sizeof(value);

			if (!RegQueryValueEx(hkey, name, NULL, NULL, (LPBYTE)value, &value_len)) {
				if (type==REG_SZ || type==REG_EXPAND_SZ || type==REG_LINK)
					entry->_content = _tcsdup(value);
				else if (type == REG_DWORD) {
					TCHAR b[32];
					_stprintf(b, TEXT("%ld"), *(DWORD*)&value);
					entry->_content = _tcsdup(b);
				}
			}

			if (!first_entry)
				first_entry = entry;

			if (last)
				last->_next = entry;

			entry->_level = level;

			last = entry;
		}

		if (last)
			last->_next = NULL;

		RegCloseKey(hkey);
	}

	_down = first_entry;
	_scanned = true;
}


const void* RegDirectory::get_next_path_component(const void* p) const
{
	LPCTSTR s = (LPCTSTR) p;

	while(*s && *s!=TEXT('\\'))
		++s;

	while(*s==TEXT('\\'))
		++s;

	if (!*s)
		return NULL;

	return s;
}


Entry* RegDirectory::find_entry(const void* p)
{
	LPCTSTR name = (LPCTSTR)p;

	for(Entry*entry=_down; entry; entry=entry->_next) {
		LPCTSTR p = name;
		LPCTSTR q = entry->_data.cFileName;

		do {
			if (!*p || *p==TEXT('\\') || *p==TEXT('/'))
				return entry;
		} while(tolower(*p++) == tolower(*q++));

		p = name;
		q = entry->_data.cAlternateFileName;

		do {
			if (!*p || *p==TEXT('\\') || *p==TEXT('/'))
				return entry;
		} while(tolower(*p++) == tolower(*q++));
	}

	return NULL;
}


 // get full path of specified registry entry
bool RegEntry::get_path(PTSTR path, size_t path_count) const
{
	return get_path_base ( path, path_count, ET_REGISTRY );
}

BOOL RegEntry::launch_entry(HWND hwnd, UINT nCmdShow)
{
	return FALSE;
}


RegDirectory::RegDirectory(Entry* parent, LPCTSTR path, HKEY hKeyRoot)
 :	RegEntry(parent),
	_hKeyRoot(hKeyRoot)
{
	_path = _tcsdup(path);

	memset(&_data, 0, sizeof(WIN32_FIND_DATA));
	_data.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
}


void RegistryRoot::read_directory(int scan_flags)
{
	Entry *entry, *last, *first_entry;
	int level = _level + 1;

	_data.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

	entry = new RegDirectory(this, TEXT("\\"), HKEY_CURRENT_USER);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_CURRENT_USER"));
	entry->_level = level;

	first_entry = entry;
	last = entry;

	entry = new RegDirectory(this, TEXT("\\"), HKEY_LOCAL_MACHINE);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_LOCAL_MACHINE"));
	entry->_level = level;

	last->_next = entry;
	last = entry;

	entry = new RegDirectory(this, TEXT("\\"), HKEY_CLASSES_ROOT);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_CLASSES_ROOT"));
	entry->_level = level;

	last->_next = entry;
	last = entry;

	entry = new RegDirectory(this, TEXT("\\"), HKEY_USERS);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_USERS"));
	entry->_level = level;

	last->_next = entry;
	last = entry;
/*
	entry = new RegDirectory(this, TEXT("\\"), HKEY_PERFORMANCE_DATA);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_PERFORMANCE_DATA"));
	entry->_level = level;

	last->_next = entry;
	last = entry;
*/
	entry = new RegDirectory(this, TEXT("\\"), HKEY_CURRENT_CONFIG);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_CURRENT_CONFIG"));
	entry->_level = level;

	last->_next = entry;
	last = entry;
/*
	entry = new RegDirectory(this, TEXT("\\"), HKEY_DYN_DATA);
	_tcscpy(entry->_data.cFileName, TEXT("HKEY_DYN_DATA"));
	entry->_level = level;

	last->_next = entry;
	last = entry;
*/
	last->_next = NULL;

	_down = first_entry;
	_scanned = true;
}
