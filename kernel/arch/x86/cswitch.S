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

    .text

/* Restore context from current stack. */
CONTEXT_RESTORE:
    pop %edi
    pop %esi
    pop %ebx
    popf
    pop %ebp
    ret

.globl ja_context_switch
ja_context_switch:
    push %ebp
    mov %esp, %ebp
    /* [ebp+8]: old_sp_p, [ebp+12]: new_sp */
    pushf
    push %ebx
    push %esi
    push %edi
    /* Store current stack to old_sp */
    mov 8(%ebp), %eax
    mov %esp, 0(%eax)
    /* Switch stacks. */
    mov 12(%ebp), %esp
    
    jmp CONTEXT_RESTORE
    .size ja_context_switch, . - ja_context_switch

.globl ja_context_jump
ja_context_jump:
    /* [esp + 4]: new_sp */
    mov 4(%esp), %esp
    
    jmp CONTEXT_RESTORE
    .size ja_context_jump, . - ja_context_jump


/*
 * Wrapper around the first function in the kernel thread.
 *
 * This function is shown first in the backchain, while returning to
 * it should never occures.
 *
 * %esp is assumed pointed to the beginning of the stack,
 * %ebx contains address of the function to be called,
 * %ebp should be 0,
 * other general registers are unspecified.
 */
.globl ja_kernel_thread
ja_kernel_thread:
    call *%ebx
    // TODO: assert "no return"
    .size ja_kernel_thread, . - ja_kernel_thread


.globl ja_context_restart
ja_context_restart:
    /* [esp + 4]: stack */
    /* [esp + 8]: entry */
    // Store all stack parameters into registers.
    mov 4(%esp), %eax
    mov 8(%esp), %ebx // %ebx is used in ja_kernel_thread
    // Now we can modify stack
    mov  %eax, %esp
    mov 0, %ebp // Mark current frame as first.
    jmp ja_kernel_thread
    .size ja_context_restart, . - ja_context_restart

.globl ja_context_restart_and_save
ja_context_restart_and_save:
    /* [esp + 4]: stack */
    /* [esp + 8]: entry */
    /* [esp + 12]: new_sp_p */
    // Fill pointer to new context
    mov 4(%esp), %eax
    sub 4, %eax
    mov 12(%esp), %ebx
    mov %eax, 0(%ebx) // *new_sp_p = stack - 4
    jmp ja_context_restart // Just jump to common restart procedure.
