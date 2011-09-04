/*
 * PROJECT:         Odyssey Boot Loader
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            boot/freeldr/arch/arm/boot.s
 * PURPOSE:         Implements the entry point for ARM machines
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

    .title "ARM FreeLDR Entry Point"
    .include "ntoskrnl/include/internal/arm/kxarm.h"
    .include "ntoskrnl/include/internal/arm/ksarm.h"
    .section .init

    NESTED_ENTRY _start
    PROLOG_END _start
    
    b ArmInit
    
    ENTRY_END _start

L_ArmInit:
    .long ArmInit
