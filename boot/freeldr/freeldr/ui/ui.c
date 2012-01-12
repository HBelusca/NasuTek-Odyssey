/*
 *  FreeLoader
 *  Copyright (C) 1998-2003  Brian Palmer  <brianp@sginet.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef _M_ARM
#include <freeldr.h>
#include <debug.h>

DBG_DEFAULT_CHANNEL(UI);

ULONG	UiScreenWidth;							// Screen Width
ULONG	UiScreenHeight;							// Screen Height

UCHAR	UiStatusBarFgColor			= COLOR_BLACK;			// Status bar foreground color
UCHAR	UiStatusBarBgColor			= COLOR_CYAN;			// Status bar background color
UCHAR	UiBackdropFgColor			= COLOR_WHITE;			// Backdrop foreground color
UCHAR	UiBackdropBgColor			= COLOR_BLUE;			// Backdrop background color
UCHAR	UiBackdropFillStyle			= MEDIUM_FILL;			// Backdrop fill style
UCHAR	UiTitleBoxFgColor			= COLOR_WHITE;			// Title box foreground color
UCHAR	UiTitleBoxBgColor			= COLOR_RED;			// Title box background color
UCHAR	UiMessageBoxFgColor			= COLOR_WHITE;			// Message box foreground color
UCHAR	UiMessageBoxBgColor			= COLOR_BLUE;			// Message box background color
UCHAR	UiMenuFgColor				= COLOR_GRAY;			// Menu foreground color
UCHAR	UiMenuBgColor				= COLOR_BLACK;			// Menu background color
UCHAR	UiTextColor					= COLOR_GRAY;			// Normal text color
UCHAR	UiSelectedTextColor			= COLOR_BLACK;			// Selected text color
UCHAR	UiSelectedTextBgColor		= COLOR_GRAY;			// Selected text background color
UCHAR	UiEditBoxTextColor			= COLOR_WHITE;			// Edit box text color
UCHAR	UiEditBoxBgColor			= COLOR_BLACK;			// Edit box text background color

CHAR	UiTitleBoxTitleText[260]	= "Boot Menu";			// Title box's title text

BOOLEAN	UiUseSpecialEffects			= FALSE;				// Tells us if we should use fade effects
BOOLEAN	UiDrawTime					= FALSE;					// Tells us if we should draw the time
BOOLEAN	UiCenterMenu				= FALSE;					// Tells us if we should use a centered or left-aligned menu
BOOLEAN	UiMenuBox					= FALSE;					// Tells us if we shuld draw a box around the menu
CHAR	UiTimeText[260] = "Seconds until highlighted choice will be started automatically: ";

const CHAR	UiMonthNames[12][15] = { "January ", "February ", "March ", "April ", "May ", "June ", "July ", "August ", "September ", "October ", "November ", "December " };

UIVTBL UiVtbl =
{
	NoUiInitialize,
	NoUiUnInitialize,
	NoUiDrawBackdrop,
	NoUiFillArea,
	NoUiDrawShadow,
	NoUiDrawBox,
	NoUiDrawText,
	NoUiDrawCenteredText,
	NoUiDrawStatusText,
	NoUiUpdateDateTime,
	NoUiMessageBox,
	NoUiMessageBoxCritical,
	NoUiDrawProgressBarCenter,
	NoUiDrawProgressBar,
	NoUiEditBox,
	NoUiTextToColor,
	NoUiTextToFillStyle,
	NoUiFadeInBackdrop,
	NoUiFadeOut,
	NoUiDisplayMenu,
	NoUiDrawMenu,
};

BOOLEAN UiInitialize(BOOLEAN ShowGui)
{
	VIDEODISPLAYMODE	UiDisplayMode; // Tells us if we are in text or graphics mode
	ULONG_PTR SectionId;
	CHAR	DisplayModeText[260];
	CHAR	SettingText[260];
	ULONG	Depth;

	if (!ShowGui) {
		if (!UiVtbl.Initialize())
		{
			MachVideoSetDisplayMode(NULL, FALSE);
			return FALSE;
		}
		return TRUE;
	}

	TRACE("Initializing User Interface.\n");
	TRACE("Reading in UI settings from [Display] section.\n");

	UiDisplayMode = MachVideoSetDisplayMode(DisplayModeText, TRUE);
	MachVideoGetDisplaySize(&UiScreenWidth, &UiScreenHeight, &Depth);

	if (VideoTextMode == UiDisplayMode)
		UiVtbl = MiniTuiVtbl;

	if (!UiVtbl.Initialize())
	{
		MachVideoSetDisplayMode(NULL, FALSE);
		return FALSE;
	}

	// Draw the backdrop and fade it in if special effects are enabled
	UiFadeInBackdrop();

	TRACE("UiInitialize() returning TRUE.\n");
	return TRUE;
}

BOOLEAN SetupUiInitialize(VOID)
{
	CHAR	DisplayModeText[260];
	ULONG	Depth, Length;


	DisplayModeText[0] = '\0';

	MachVideoSetDisplayMode(DisplayModeText, TRUE);
	MachVideoGetDisplaySize(&UiScreenWidth, &UiScreenHeight, &Depth);

	UiVtbl = SetupUiVtbl;
	UiVtbl.Initialize();

	// Draw the backdrop and fade it in if special effects are enabled
	UiVtbl.FillArea(0,
			0,
			UiScreenWidth - 1,
			UiScreenHeight - 2,
			0,
			ATTR(UiBackdropFgColor, UiBackdropBgColor));

	UiDrawTime = FALSE;
	UiStatusBarBgColor = 7;

	Length = strlen("Odyssey " KERNEL_VERSION_STR " Setup");
	memset(DisplayModeText, 0xcd, Length + 2);
	DisplayModeText[Length + 2] = '\0';

	UiVtbl.DrawText(4, 1, "Odyssey " KERNEL_VERSION_STR " Setup", ATTR(COLOR_GRAY, UiBackdropBgColor));
	UiVtbl.DrawText(3, 2, DisplayModeText, ATTR(COLOR_GRAY, UiBackdropBgColor));

	TRACE("UiInitialize() returning TRUE.\n");

	return TRUE;
}

VOID UiUnInitialize(PCSTR BootText)
{
	UiDrawBackdrop();
	UiDrawStatusText("Booting...");
	UiInfoBox(BootText);

	UiVtbl.UnInitialize();
}

VOID UiDrawBackdrop(VOID)
{
	UiVtbl.DrawBackdrop();
}

VOID UiFillArea(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, CHAR FillChar, UCHAR Attr /* Color Attributes */)
{
	UiVtbl.FillArea(Left, Top, Right, Bottom, FillChar, Attr);
}

VOID UiDrawShadow(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom)
{
	UiVtbl.DrawShadow(Left, Top, Right, Bottom);
}

VOID UiDrawBox(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, UCHAR VertStyle, UCHAR HorzStyle, BOOLEAN Fill, BOOLEAN Shadow, UCHAR Attr)
{
	UiVtbl.DrawBox(Left, Top, Right, Bottom, VertStyle, HorzStyle, Fill, Shadow, Attr);
}

VOID UiDrawText(ULONG X, ULONG Y, PCSTR Text, UCHAR Attr)
{
	UiVtbl.DrawText(X, Y, Text, Attr);
}

VOID UiDrawCenteredText(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, PCSTR TextString, UCHAR Attr)
{
	UiVtbl.DrawCenteredText(Left, Top, Right, Bottom, TextString, Attr);
}

VOID UiDrawStatusText(PCSTR StatusText)
{
	UiVtbl.DrawStatusText(StatusText);
}

VOID UiUpdateDateTime(VOID)
{
	UiVtbl.UpdateDateTime();
}

VOID UiInfoBox(PCSTR MessageText)
{
	ULONG		TextLength;
	ULONG		BoxWidth;
	ULONG		BoxHeight;
	ULONG		LineBreakCount;
	ULONG		Index;
	ULONG		LastIndex;
	ULONG		Left;
	ULONG		Top;
	ULONG		Right;
	ULONG		Bottom;

	TextLength = strlen(MessageText);

	// Count the new lines and the box width
	LineBreakCount = 0;
	BoxWidth = 0;
	LastIndex = 0;
	for (Index=0; Index<TextLength; Index++)
	{
		if (MessageText[Index] == '\n')
		{
			LastIndex = Index;
			LineBreakCount++;
		}
		else
		{
			if ((Index - LastIndex) > BoxWidth)
			{
				BoxWidth = (Index - LastIndex);
			}
		}
	}

	// Calc the box width & height
	BoxWidth += 6;
	BoxHeight = LineBreakCount + 4;

	// Calc the box coordinates
	Left = (UiScreenWidth / 2) - (BoxWidth / 2);
	Top =(UiScreenHeight / 2) - (BoxHeight / 2);
	Right = (UiScreenWidth / 2) + (BoxWidth / 2);
	Bottom = (UiScreenHeight / 2) + (BoxHeight / 2);

	// Draw the box
	UiDrawBox(Left,
			  Top,
			  Right,
			  Bottom,
			  VERT,
			  HORZ,
			  TRUE,
			  TRUE,
			  ATTR(UiMenuFgColor, UiMenuBgColor)
			  );

	// Draw the text
	UiDrawCenteredText(Left, Top, Right, Bottom, MessageText, ATTR(UiTextColor, UiMenuBgColor));
}

VOID UiMessageBox(PCSTR MessageText)
{
	UiVtbl.MessageBox(MessageText);
}

VOID UiMessageBoxCritical(PCSTR MessageText)
{
	UiVtbl.MessageBoxCritical(MessageText);
}

UCHAR UiTextToColor(PCSTR ColorText)
{
	return UiVtbl.TextToColor(ColorText);
}

UCHAR UiTextToFillStyle(PCSTR FillStyleText)
{
	return UiVtbl.TextToFillStyle(FillStyleText);
}

VOID UiDrawProgressBarCenter(ULONG Position, ULONG Range, PCHAR ProgressText)
{
	UiVtbl.DrawProgressBarCenter(Position, Range, ProgressText);
}

VOID UiDrawProgressBar(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, ULONG Position, ULONG Range, PCHAR ProgressText)
{
	UiVtbl.DrawProgressBar(Left, Top, Right, Bottom, Position, Range, ProgressText);
}

VOID UiShowMessageBoxesInSection(PCSTR SectionName)
{
#if 0
	ULONG		Idx;
	CHAR	SettingName[80];
	CHAR	SettingValue[80];
	PCHAR	MessageBoxText;
	ULONG		MessageBoxTextSize;
	ULONG_PTR	SectionId;

	if (!IniOpenSection(SectionName, &SectionId))
	{
		return;
	}

	//
	// Find all the message box settings and run them
	//
	for (Idx=0; Idx<IniGetNumSectionItems(SectionId); Idx++)
	{
		IniReadSettingByNumber(SectionId, Idx, SettingName, sizeof(SettingName), SettingValue, sizeof(SettingValue));

		if (_stricmp(SettingName, "MessageBox") == 0)
		{
			// Get the real length of the MessageBox text
			MessageBoxTextSize = IniGetSectionSettingValueSize(SectionId, Idx);

			//if (MessageBoxTextSize > 0)
			{
				// Allocate enough memory to hold the text
				MessageBoxText = MmHeapAlloc(MessageBoxTextSize);

				if (MessageBoxText)
				{
					// Get the MessageBox text
					IniReadSettingByNumber(SectionId, Idx, SettingName, sizeof(SettingName), MessageBoxText, MessageBoxTextSize);

					// Fix it up
					UiEscapeString(MessageBoxText);

					// Display it
					UiMessageBox(MessageBoxText);

					// Free the memory
					MmHeapFree(MessageBoxText);
				}
			}
		}
	}
#endif
}

VOID UiEscapeString(PCHAR String)
{
	ULONG		Idx;

	for (Idx=0; Idx<strlen(String); Idx++)
	{
		// Escape the new line characters
		if (String[Idx] == '\\' && String[Idx+1] == 'n')
		{
			// Escape the character
			String[Idx] = '\n';

			// Move the rest of the string up
			strcpy(&String[Idx+1], &String[Idx+2]);
		}
	}
}

VOID UiTruncateStringEllipsis(PCHAR StringText, ULONG MaxChars)
{
	if (strlen(StringText) > MaxChars)
	{
		strcpy(&StringText[MaxChars - 3], "...");
	}
}

BOOLEAN UiDisplayMenu(PCSTR MenuItemList[], ULONG MenuItemCount, ULONG DefaultMenuItem, LONG MenuTimeOut, ULONG* SelectedMenuItem, BOOLEAN CanEscape, UiMenuKeyPressFilterCallback KeyPressFilter)
{
	return UiVtbl.DisplayMenu(MenuItemList, MenuItemCount, DefaultMenuItem, MenuTimeOut, SelectedMenuItem, CanEscape, KeyPressFilter);
}

VOID UiFadeInBackdrop(VOID)
{
	UiVtbl.FadeInBackdrop();
}

VOID UiFadeOut(VOID)
{
	UiVtbl.FadeOut();
}

BOOLEAN UiEditBox(PCSTR MessageText, PCHAR EditTextBuffer, ULONG Length)
{
	return UiVtbl.EditBox(MessageText, EditTextBuffer, Length);
}
#endif
