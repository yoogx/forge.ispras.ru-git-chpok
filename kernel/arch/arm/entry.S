/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */
#include <arch/memlayout.h>
#include "regs.h"
#include "cpu.h"

reset:
    bl reset

.macro putc symbl
    ldr r0, =\symbl
    put_r0
.endm

.macro put_r0
    /* write symbl to UART phys addr. Without initialization.
     * Hope it is init correctly by bootloader.
     */
    ldr r4, =0x3f201000
    ldr r5, =0x3f201018
__repeat2_\@:
    ldr r6, [r5]
    tst r6, #0x20
    bne __repeat2_\@

    mov r7, r0
    str r7, [r4]
.endm

.macro print_hello
    //print "JetOS"
    putc 'J'
    putc 'e'
    putc 't'
    putc 'O'
    putc 'S'
    putc ' '
    //// print mode
    //mrs r0, cpsr
    //and r0, r0, #0xf
    //add r0, #'0'
    //put_r0
    putc '\n'
    putc '\r'
.endm

#ifdef ARMv7_VIRT_EXTENSION
switch_from_hyp_mode:
    ldr r1, =PHYS(__entry_cont)
    msr ELR_hyp, r1
    mov r1, #KERNEL_ENTRY_MODE
    msr SPSR_hyp, r1
    eret
#endif

/*
 * Entry point for the kernel.
 * r0 -> 0x00000000
 * r1 -> machine type
 * r2 -> boot date (ATAGS or dtb)
 * preserve these registers as argument for kernel_main
 *
 */
.globl jet_start
jet_start = PHYS(__entry)
__entry:
    print_hello

    /*
     * switch to KERNEL_ENTRY_MODE. This is a little tricky if current mode is
     * hypervisor mode. Hyp mode works only on ARM with virt extension.
     */
    mrs r0, cpsr
    and r1, r0, #CPSR_MODE_MASK
    teq r1, #CPSR_MODE_HYP // test for HYP mode

#ifdef ARMv7_VIRT_EXTENSION
    ldreq r0, =PHYS(switch_from_hyp_mode) // in HYP mode
    bxeq r0                               // in HYP mode
#endif

    movne r1, #KERNEL_ENTRY_MODE // not in HYP mode
    msrne cpsr_all, r1           // not in HYP mode

__entry_cont:
    print_hello

    // Setup the stack.
    ldr sp, =jet_entry_stack

    // TODO reset regs?
    mov fp, #0

    // Start MMU enabling
    // Set all Domains to Client
    ldr r1, =0x55555555
    mcr p15, 0, r1, c3, c0, 0   // Write Domain Access Control Register

    // Write 0 to TTBCR
    ldr r1, =0
    mcr p15, 0, r1, c2, c0, 2

    // Write l1_table addr to TTBR0
    ldr r1, =PHYS(entry_l1_table)
    mcr p15, 0, r1, c2, c0, 0

    // Enable MMU
    mrc p15, 0, r1, c1, c0, 0 //read SCTLR to r1
    orr r1, r1, #SCTLR_M //enable bit 0 in r1
    mcr p15, 0, r1, c1, c0, 0 //write r1 to SCTLR

    // MMU is enabled but pc still has low addressess
    // Jump UP
    ldr pc, =relocated //long jump
relocated:
    // finish MMU enabling

    // Call arch_init
    ldr r3, =jet_arch_init
    blx r3

    // Call jet_boot. Should not return
    ldr r3, =jet_boot
    blx r3

    // Halt if return
halt:
    wfe
    b halt
