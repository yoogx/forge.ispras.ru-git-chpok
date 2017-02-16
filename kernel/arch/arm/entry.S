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

// Entry point for the kernel.
// r0 -> 0x00000000
// r1 -> machine type
// r2 -> boot date (ATAGS or dtb)
// preserve these registers as argument for kernel_main
.globl jet_start
jet_start:
        //print "JetOS"
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

        // Setup the stack.
        ldr sp, =jet_entry_stack


        // Set all Domains to Client
        ldr r1, =0x55555555
        mcr p15, 0, r1, c3, c0, 0   // Write Domain Access Control Register

        // Call arch_init
        ldr r3, =jet_arch_init
        blx r3

        // Call jet_boot. Should not return
        ldr r3, =jet_boot
        blx r3

        // halt
halt:
        wfe
        b halt
