/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         FreeLoader
 * FILE:            freeldr/ui/minitui.c
 * PURPOSE:         Mini Text UI interface
 * PROGRAMMERS:     Brian Palmer <brianp@sginet.com>
 *                  Herv� Poussineau
 */
#ifndef _M_ARM
#include <freeldr.h>

VOID MiniTuiDrawBackdrop(VOID)
{
	//
	// Fill in a black background
	//
	TuiFillArea(0,
	            0,
	            UiScreenWidth - 1,
	            UiScreenHeight - 1,
	            0,
	            0);
	
	//
	// Update the screen buffer
	//
	VideoCopyOffScreenBufferToVRAM();
}

VOID MiniTuiDrawStatusText(PCSTR StatusText)
{
	//
	// Minimal UI doesn't have a status bar
	//
}

VOID MiniTuiDrawProgressBarCenter(ULONG Position, ULONG Range, PCHAR ProgressText)
{
	ULONG		Left, Top, Right, Bottom;
	ULONG		Width = 80; // Allow for 50 "bars"
	ULONG		Height = 2;
	ULONG           i;

	Width = 80;
	Left = 0;
	Right = Left + Width;
	Top = UiScreenHeight - Height - 4;
	Bottom = Top + Height + 1;

	for (i=0; i<Width; i++)
	{
		TuiDrawText(Left+i, Top+3, "\xDD", ATTR(UiTextColor, UiMenuBgColor));
		//TuiDrawText(Left+2+i, Top+2, "\xDB", ATTR(COLOR_WHITE, COLOR_GRAY));
	}

	MiniTuiDrawProgressBar(Left, Top, Right, Bottom, Position, Range, ProgressText);
}
VOID MiniTuiF8Notice()
{
	ULONG		Left, Top, Right, Bottom;
	ULONG		Width = 80; // Allow for 50 "bars"
	ULONG		Height = 2;
	ULONG           i;

	Width = 80;
	Left = 0;
	Right = Left + Width;
	Top = UiScreenHeight - Height - 4;
	Bottom = Top + Height + 1;

	for (i=0; i<Width; i++)
	{
		TuiDrawText(Left+i, Top+3, "\xDD", ATTR(UiTextColor, UiMenuBgColor));
	}

	TuiDrawText(Left, Bottom+2, "For troubleshooting and advanced startup options for Odyssey, press F8.", ATTR(UiTextColor, UiMenuBgColor));
	
	MiniTuiDrawProgressBar(Left, Top, Right, Bottom, 0, 100, "Starting Odyssey...");
}

VOID MiniTuiDrawProgressBar(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, ULONG Position, ULONG Range, PCHAR ProgressText)
{
	ULONG		i;
	ULONG		ProgressBarWidth = 80;

	// First make sure the progress bar text fits
	UiTruncateStringEllipsis(ProgressText, ProgressBarWidth - 4);

	if (Position > Range)
	{
		Position = Range;
	}

	//
	//  Draw the "Loading..." text
	//
	TuiDrawCenteredText(Left, Top+3, Right, Top+3, ProgressText, ATTR(7, 0));

	// Draw the percent complete

	for (i=0; i<(Position*ProgressBarWidth)/Range; i++)
	{
	  TuiDrawText(Left+i, Top+3, "\xDB", ATTR(UiTextColor, UiMenuBgColor));
	  //TuiDrawText(Left+2+i, Top+2, "\xDD", ATTR(COLOR_WHITE, COLOR_GRAY));
	}

	TuiUpdateDateTime();
	VideoCopyOffScreenBufferToVRAM();
}

VOID
MiniTuiDrawMenu(PUI_MENU_INFO MenuInfo)
{
    ULONG i;

    //
    // Draw the backdrop
    //
    UiDrawBackdrop();

    //
    // No GUI status bar text, just minimal text. first to tell the user to
    // choose.
    //
    UiVtbl.DrawText(0,
                    MenuInfo->Top - 2,
                    "Please select the operating system to start:",
                    ATTR(UiMenuFgColor, UiMenuBgColor));

    //
    // Now tell him how to choose
    //
    UiVtbl.DrawText(0,
                    MenuInfo->Bottom + 1,
                    "Use the up and down arrow keys to move the highlight to "
                    "your choice.",
                    ATTR(UiMenuFgColor, UiMenuBgColor));
    UiVtbl.DrawText(0,
                    MenuInfo->Bottom + 2,
                    "Press ENTER to choose.",
                    ATTR(UiMenuFgColor, UiMenuBgColor));

    //
    // And offer F8 options
    //
    UiVtbl.DrawText(0,
                    UiScreenHeight - 4,
                    "For troubleshooting and advanced startup options for "
                    "Odyssey, press F8.",
                    ATTR(UiMenuFgColor, UiMenuBgColor));

    //
    // Draw the menu box
    //
    TuiDrawMenuBox(MenuInfo);

    //
    // Draw each line of the menu
    //
    for (i = 0; i < MenuInfo->MenuItemCount; i++) TuiDrawMenuItem(MenuInfo, i);
    VideoCopyOffScreenBufferToVRAM();
}

const UIVTBL MiniTuiVtbl =
{
	TuiInitialize,
	TuiUnInitialize,
	MiniTuiDrawBackdrop,
	TuiFillArea,
	TuiDrawShadow,
	TuiDrawBox,
	TuiDrawText,
	TuiDrawCenteredText,
	MiniTuiDrawStatusText,
	TuiUpdateDateTime,
	TuiMessageBox,
	TuiMessageBoxCritical,
	MiniTuiDrawProgressBarCenter,
	MiniTuiDrawProgressBar,
	TuiEditBox,
	TuiTextToColor,
	TuiTextToFillStyle,
	MiniTuiDrawBackdrop, /* no FadeIn */ 
	TuiFadeOut,
	TuiDisplayMenu,
	MiniTuiDrawMenu,
	MiniTuiF8Notice,
};
#endif
