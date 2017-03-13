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

#include "regs.h"

// Undefined instruction
jet_excp_undef_asm:
    b .

// Supervision Call (syscall)
// in syscall we don't need to save/restor volatile registers,
// because svc is synchronous operation
jet_excp_svc_asm:
    // lr is already right

    srsfd sp!, #CPSR_MODE_SVC // save lr and SPSR on our stack

    //push {r0-r3, r12} //push volatile regs

    //make sp to 8-byte aligned. Store adjustment in stack
    and r2, sp, #0x7
    sub sp, sp, r2 //sp = sp - sp&7
    push {r2}

    push {lr} // lr in non-volatile, but we corrupt it in blx

    /* r0 is syscall_id
       r1 is args */
    ldr r3, =syscall_handler
    blx r3

    pop {lr}

    // restore original (possible not 8-byte aligned) sp value
    pop {r2}
    add sp, sp, r2

    //pop {r0-r3, r12} //restore volatile regs

    rfefd sp! //from stack load saved lr and SPSR  to pc and CPSR

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
    sub lr, lr, #4 //IRQ excp should return to lr-4 addr
    srsfd sp!, #CPSR_MODE_SVC // save lr and SPSR on SVC stack

    /* Switch to SVC mode. If this is a nested interrupt, then
     * SVC mode's CPSR has a IRQ interrupt enabled. Thus we should
     * disable interrupt while switching to SVC mode. CPSID instruction
     * does it for us.
     */
    cpsid i, #CPSR_MODE_SVC

    push {r0-r3, r12} //push volatile regs

    //make sp to 8-byte aligned. Store adjustment in stack
    and r1, sp, #0x7
    sub sp, sp, r1 //sp = sp - sp&7
    push {r1}

    push {lr} // lr in non-volatile, but we corrupt it in blx

    ldr r3, =irq_handle
    blx r3

    pop {lr}

    // restore original (possible not 8-byte aligned) sp value
    pop {r1}
    add sp, sp, r1

    pop {r0-r3, r12} //restore volatile regs

    rfefd sp! //from stack load saved lr and SPSR  to pc and CPSR

// FIQ interrupt
jet_excp_fiq_asm:
    b .

.section ".vector_table", "a"
/* 'ldr pc, =<lable>' is pseudo instruction. GCC substitutes is with something like
 * 'ldr pc, [pc, #offset]' and put at the end of this section (.vector_table) instruction '.long <lable>'
 */
.globl vector_table
vector_table:
    b . // Not used in normal mode (reset in secuter mode)
    ldr pc, =jet_excp_undef_asm // Undefined instruction
    ldr pc, =jet_excp_svc_asm // Supervision Call
    ldr pc, =jet_excp_prefetch_abort_asm // Prefetch Abort
    ldr pc, =jet_excp_data_abort_asm // Data Abort
    b . // Not used
    ldr pc, =jet_excp_irq_asm // IRQ interrupt
    ldr pc, =jet_excp_fiq_asm // FIQ interrupt
