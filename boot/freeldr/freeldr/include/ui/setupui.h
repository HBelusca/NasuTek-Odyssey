/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         FreeLoader
 * FILE:            freeldr/include/ui/setupui.h
 * PURPOSE:         Setup UI interface header
 * PROGRAMMERS:     Hervé Poussineau
 */

#pragma once

///////////////////////////////////////////////////////////////////////////////////////
//
// Textual User Interface Functions
//
///////////////////////////////////////////////////////////////////////////////////////

VOID SetupUiDrawBackdrop(VOID);
VOID SetupUiDrawStatusText(PCSTR StatusText);

extern const UIVTBL SetupUiVtbl;
