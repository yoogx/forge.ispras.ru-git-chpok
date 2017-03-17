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

// Supervision Call (syscall)
// in syscall we don't need to save/restor volatile registers,
// because svc is synchronous operation
jet_excp_svc_asm:
    // lr is already right

    srsfd sp!, #CPSR_MODE_SVC // save lr and SPSR on our stack

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

    rfefd sp! //from stack load saved lr and SPSR  to pc and CPSR

/* SVC is syncronous exception. We don't do SVC call in kernel, thus we
 * can't get nested svc exception. Therefore from any other exception we
 * switch to SVC mode and save everyting on stack there. This allows to
 * have same handler for nested and not-nested exceptions.
 */
.macro common_handling handler
    srsfd sp!, #CPSR_MODE_SVC // save lr (pc of exception) and SPSR on SVC stack

    /* Switch to SVC mode. If this is a nested exception, then
     * SVC mode's CPSR has a IRQ interrupt enabled. Thus we should
     * disable interrupt while switching to SVC mode. CPSID instruction
     * does it for us.
     */
    cpsid i, #CPSR_MODE_SVC

    push {r0-r3, r12} //push volatile regs

    push {lr} //lr is non-volatile and banked, but we corrupt it in blx. Save it

    push {fp}
    mov fp, #0 //for gdb backtrace

    mov r0, sp // mov sp to r0. This is an interrupt context. Stack alignment
               // won't go to interrupt contxt

    //make sp to 8-byte aligned. Store adjustment in stack
    and r3, sp, #0x7
    sub sp, sp, r3 //sp = sp - sp&7
    push {r3}

    /* r0 is interrupt_context */
    ldr r3, =\handler
    blx r3

    // restore original (possible not 8-byte aligned) sp value
    pop {r3}
    add sp, sp, r3

    pop {fp}

    pop {lr}

    pop {r0-r3, r12} //restore volatile regs

    rfefd sp! //from stack load saved lr and SPSR  to pc and CPSR
.endm

// Prefetch Abort
jet_excp_prefetch_abort_asm:
    sub lr, lr, #4 //prefetch abort xcp should return to lr-4 addr
    common_handling handler=prefetch_abort_handler

// Data Abort
jet_excp_data_abort_asm:
    sub lr, lr, #8 //data abort excp should return to lr-8 addr
    common_handling handler=data_abort_handler

// IRQ interrupt
jet_excp_irq_asm:
    sub lr, lr, #4 //IRQ excp should return to lr-4 addr
    common_handling handler=irq_handle

// FIQ interrupt
jet_excp_fiq_asm:
    sub lr, lr, #4 //FIQ excp should return to lr-4 addr
    common_handling handler=fiq_handler

// Undefined instruction
jet_excp_undef_asm:
    // lr is already right
    common_handling handler=undefined_instruction_handler


.section ".vector_table", "a"
/* 'ldr pc, =<lable>' is pseudo instruction. GCC substitutes it with something like
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
