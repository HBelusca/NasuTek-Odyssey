/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         FreeLoader
 * FILE:            freeldr/ui/noui.c
 * PURPOSE:         No Text UI interface
 * PROGRAMMERS:     Hervé Poussineau
 */
#ifndef _M_ARM
#include <freeldr.h>

VOID SetupUiDrawBackdrop(VOID)
{
	CHAR	DisplayModeText[260];
	ULONG	Length;
	
	UiBackdropFgColor = COLOR_GRAY;
	UiBackdropBgColor = COLOR_BLUE;
	UiStatusBarFgColor = COLOR_BLACK;
	UiStatusBarBgColor = COLOR_GRAY;

	TuiFillArea(0,
			0,
			UiScreenWidth - 1,
			UiScreenHeight,
			0,
			ATTR(UiStatusBarFgColor, UiStatusBarBgColor));
			
	TuiFillArea(0,
			0,
			UiScreenWidth - 1,
			UiScreenHeight - 2,
			0,
			ATTR(UiBackdropFgColor, UiBackdropBgColor));
	
	Length = strlen("Odyssey " KERNEL_VERSION_STR " Setup");
	memset(DisplayModeText, 0xcd, Length + 2);
	DisplayModeText[Length + 2] = '\0';

	TuiDrawText(4, 1, "Odyssey " KERNEL_VERSION_STR " Setup", ATTR(UiBackdropFgColor, UiBackdropBgColor));
	TuiDrawText(3, 2, DisplayModeText, ATTR(UiBackdropFgColor, UiBackdropBgColor));
	
	VideoCopyOffScreenBufferToVRAM();
}

VOID SetupUiDrawStatusText(PCSTR StatusText)
{
	ULONG		Left, Top, Right, Bottom;
	ULONG		Width = 80; // Allow for 50 "bars"
	ULONG		Height = 2;
	ULONG       i;

	Width = 80;
	Left = 2;
	Right = Left + Width;
	Top = UiScreenHeight - Height - 4;
	Bottom = Top + Height + 1;

	for (i=0; i<Width; i++)
	{
		TuiDrawText(Left+i, Bottom+2, " ", ATTR(UiStatusBarFgColor, UiStatusBarBgColor));
	}
	
    TuiDrawText(Left, Bottom+2, StatusText, ATTR(UiStatusBarFgColor, UiStatusBarBgColor));
	
	VideoCopyOffScreenBufferToVRAM();
}

const UIVTBL SetupUiVtbl =
{
	TuiInitialize,
	TuiUnInitialize,
	SetupUiDrawBackdrop,
	NoUiFillArea,
	NoUiDrawShadow,
	NoUiDrawBox,
	TuiDrawText,
	TuiDrawCenteredText,
	SetupUiDrawStatusText,
	NoUiUpdateDateTime,
	NoUiMessageBox,
	NoUiMessageBoxCritical,
	NoUiDrawProgressBarCenter,
	NoUiDrawProgressBar,
	NoUiEditBox,
	TuiTextToColor,
	TuiTextToFillStyle,
	SetupUiDrawBackdrop, /* no FadeIn */ 
	NoUiFadeOut,
	NoUiDisplayMenu,
	NoUiDrawMenu,
};
#endif
