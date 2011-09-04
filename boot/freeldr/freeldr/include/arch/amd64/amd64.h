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

#ifndef __ASM__
#pragma once
#endif

// This is needed because headers define wrong one for Odyssey
#undef KIP0PCRADDRESS
#define KIP0PCRADDRESS                      0xFFFFF78000001000ULL /* FIXME!!! */

#define VA_MASK 0x0000FFFFFFFFFFFFUL

#define PtrToPfn(p) \
    ((((ULONGLONG)p) >> PAGE_SHIFT) & 0xfffffffULL)

#define VAtoPXI(va) ((((ULONG64)va) >> PXI_SHIFT) & 0x1FF)
#define VAtoPPI(va) ((((ULONG64)va) >> PPI_SHIFT) & 0x1FF)
#define VAtoPDI(va) ((((ULONG64)va) >> PDI_SHIFT) & 0x1FF)
#define VAtoPTI(va) ((((ULONG64)va) >> PTI_SHIFT) & 0x1FF)

#define HYPERSPACE_BASE             0xfffff70000000000ULL
#define HAL_BASE                    0xffffffff80000000ULL
#define APIC_BASE                   0xFFFFFFFFFFFE0000ULL

#define APIC_PHYS_BASE              0xfee00000

#define NUM_PAGES_KERNEL

#ifndef ASM

VOID FrLdrSetupGdtIdt(VOID);
#define MachInit PcMachInit
#endif

/* EOF */
