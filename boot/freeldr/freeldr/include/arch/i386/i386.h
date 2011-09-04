/*
 *  FreeLoader
 *
 *  Copyright (C) 2003  Eric Kohl
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

#pragma once

// This is needed because headers define wrong one for Odyssey
#undef KIP0PCRADDRESS
#define KIP0PCRADDRESS                      0xffdff000

void i386DivideByZero(void);
void i386DebugException(void);
void i386NMIException(void);
void i386Breakpoint(void);
void i386Overflow(void);
void i386BoundException(void);
void i386InvalidOpcode(void);
void i386FPUNotAvailable(void);
void i386DoubleFault(void);
void i386CoprocessorSegment(void);
void i386InvalidTSS(void);
void i386SegmentNotPresent(void);
void i386StackException(void);
void i386GeneralProtectionFault(void);
void i386PageFault(void);
void i386CoprocessorError(void);
void i386AlignmentCheck(void);
void i386MachineCheck(void);

/* EOF */
