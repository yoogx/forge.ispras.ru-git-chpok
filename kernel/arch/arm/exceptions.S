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

// Undefined instruction
jet_excp_undef_asm:
    b .

// Supervision Call
jet_excp_svc_asm:
    b .

// Prefetch Abort
jet_excp_prefetch_abort_asm:
    b .

// Data Abort
jet_excp_data_abort_asm:
    //print something
    ldr r4, =0x2020040
    ldr r3,    ='E'
    str r3, [r4]
    ldr r3,    ='r'
    str r3, [r4]
    ldr r3,    ='r'
    str r3, [r4]
    ldr r3,    ='o'
    str r3, [r4]
    ldr r3,    ='r'
    str r3, [r4]
    ldr r3,    ='\n'
    str r3, [r4]
    b .

// IRQ interrupt
jet_excp_irq_asm:
    ldr r4, =0x2020040
    ldr r3,    ='I'
    str r3, [r4]
    ldr r3,    ='R'
    str r3, [r4]
    ldr r3,    ='Q'
    str r3, [r4]
    b .

// FIQ interrupt
jet_excp_fiq_asm:
    b .

.section ".vector_table", "a"
/* 'ldr pc, =<lable>' is pseudo instruction. GCC substitutes is with something like
 * 'ldr pc, [pc, #offset]' and put at the end of this section (.vector_table) instruction '.long <lable>'
 */
vector_table:
    b . // Not used in normal mode (reset in secuter mode)
    ldr pc, =jet_excp_undef_asm // Undefined instruction
    ldr pc, =jet_excp_svc_asm // Supervision Call
    ldr pc, =jet_excp_prefetch_abort_asm // Prefetch Abort
    ldr pc, =jet_excp_data_abort_asm // Data Abort
    b . // Not used
    ldr pc, =jet_excp_irq_asm // IRQ interrupt
    ldr pc, =jet_excp_fiq_asm // FIQ interrupt
