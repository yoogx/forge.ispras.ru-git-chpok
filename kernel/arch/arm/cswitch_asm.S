/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

/* Restore context from current stack. */
CONTEXT_RESTORE:
    pop {r4-r11}
    pop {pc}

.globl ja_context_jump
ja_context_jump:
    /* r0: new_sp */
    mov sp, r0
    b CONTEXT_RESTORE


    .globl ja_context_restart
ja_context_restart:
    /* r0: stack, r1: entry */
    mov sp, r0

    /* TODO Zero registers */
    mov fp, #0

    mov pc, r1


    .globl ja_user_space_jump
ja_user_space_jump:
    /* r0 - kernel stack,
       r1 - space_id,
       r2 - entry_user,
       r3 - stack_user
       */
    mov sp, r0 // save kernel_stack in sp of SVC mode

    mov r7, #0
    push {r3, r7} //push regs for user sp, lr
    // usage of 'sp!' here leads to warning, thus we do
    // two steps: loads and sp move
    ldmfd sp, {sp, lr}^ //load user sp, lr
    sub sp, sp, #8 // size of sp+lr

    mov r8, #CPSR_MODE_USR
    push {r2, r8} //push user CPSR and user pc

    mov lr, #0
    mov fp, #0

    rfefd sp! //pop from stack to CPSR and pc
