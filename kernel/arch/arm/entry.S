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
#include "memlayout.h"
#include "regs.h"

.macro print_hello
    //print "JetOS"
    /* write to UART phys addr. Without initialization.
     * Hope it is init correctly by bootloader.
     */
    ldr r4, =0x2020040
    ldr r3, ='J'
    str r3, [r4]
    ldr r3, ='e'
    str r3, [r4]
    ldr r3, ='t'
    str r3, [r4]
    ldr r3, ='O'
    str r3, [r4]
    ldr r3, ='S'
    str r3, [r4]
    ldr r3, ='\n'
    str r3, [r4]
.endm

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

    // Setup the stack.
    ldr sp, =jet_entry_stack

    //TODO: init IRQ sp
    msr cpsr_c, 1<<7 | 1<<6 | 0x12 // switch to IRQ mode
    ldr sp, =0x10050000
    msr cpsr_c, 1<<7 | 1<<6 | 0x13 // return to SVC mode

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
