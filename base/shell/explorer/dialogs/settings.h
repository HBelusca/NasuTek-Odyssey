/*
 * Copyright 2004, 2004 Martin Fuchs
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
 // dialogs/settings.h
 //
 // Explorer dialogs
 //
 // Martin Fuchs, 18.01.2004
 //


void ExplorerPropertySheet(HWND hparent);


 /// "Desktopbar Settings" Property Sheet Dialog
struct DesktopSettingsDlg : public OwnerDrawParent<PropSheetPageDlg>
{
	typedef OwnerDrawParent<PropSheetPageDlg> super;

	DesktopSettingsDlg(HWND hwnd);

protected:
	ResBitmap	_bmp0;
	ResBitmap	_bmp1;
	ResBitmap	_bmp2;
	ResBitmap	_bmp3;
	ResBitmap	_bmp4;
	ResBitmap	_bmp5;
	ResBitmap	_bmp6;
	ResBitmap	_bmp7;
	ResBitmap	_bmp8;
	ResBitmap	_bmp9;
	ResBitmap	_bmp10;

	int	_alignment_cur;
	int	_alignment_tmp;

	int	_display_version_org;

	virtual int Command(int id, int code);
	virtual int Notify(int id, NMHDR* pnmh);
};


 /// "Taskbar Settings" Property Sheet Dialog
struct TaskbarSettingsDlg : public PropSheetPageDlg
{
	typedef PropSheetPageDlg super;

	TaskbarSettingsDlg(HWND hwnd);

	virtual int	Command(int id, int code);
	virtual int Notify(int id, NMHDR* pnmh);

protected:
	XMLDoc	_cfg_org;
};


 /// "Startmenu Settings" Property Sheet Dialog
struct StartmenuSettingsDlg : public PropSheetPageDlg
{
	typedef PropSheetPageDlg super;

	StartmenuSettingsDlg(HWND hwnd);

	virtual int	Command(int id, int code);
};


 /// configuration dialog to choose between MDI and SDI mode
struct MdiSdiDlg : public ResizeController<Dialog>
{
	typedef ResizeController<Dialog> super;

	MdiSdiDlg(HWND hwnd);

protected:
	virtual int	Command(int id, int code);
};
