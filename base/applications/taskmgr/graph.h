/*
 *  Odyssey Task Manager
 *
 *  graph.h
 *
 *  Copyright (C) 1999 - 2001  Brian Palmer  <brianp@odyssey.org>
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define BRIGHT_GREEN	RGB(0, 255, 0)
#define MEDIUM_GREEN	RGB(0, 190, 0)
#define DARK_GREEN	RGB(0, 130, 0)
#define RED		RGB(255, 0, 0)

extern	WNDPROC		OldGraphWndProc;

INT_PTR CALLBACK	Graph_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


#ifdef __cplusplus
};
#endif
