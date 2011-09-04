/* $Id: vis.h 53467 2011-08-27 12:38:23Z gadamopoulos $
 *
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey Win32k subsystem
 * PURPOSE:          Visibility computations interface definition
 * FILE:             include/win32k/vis.h
 * PROGRAMMER:       Ge van Geldorp (ge@gse.nl)
 *
 */

#pragma once

HRGN FASTCALL VIS_ComputeVisibleRegion(PWND Window, BOOLEAN ClientArea, BOOLEAN ClipChildren, BOOLEAN ClipSiblings);
VOID FASTCALL co_VIS_WindowLayoutChanged(PWND Window, HRGN UncoveredRgn);

/* EOF */
