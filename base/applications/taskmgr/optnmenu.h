/*
 *  Odyssey Task Manager
 *
 *  optnmenu.h
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

/*
 * Menu item handlers for the options menu.
 */

#pragma once

#define OPTIONS_MENU_INDEX    1

void TaskManager_OnOptionsAlwaysOnTop(void);
void TaskManager_OnOptionsMinimizeOnUse(void);
void TaskManager_OnOptionsHideWhenMinimized(void);
void TaskManager_OnOptionsShow16BitTasks(void);
