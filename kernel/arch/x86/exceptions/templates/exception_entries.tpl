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

#include <config.h>
#include <asm_offsets_interrupt_context.h>

    .text

#ifdef POK_NEEDS_GDB
#define SAVE_FRAME call save_frame
#else
#define SAVE_FRAME
#endif /* POK_NEEDS_GDB */

.macro INTERRUPT_PROLOGUE_ERROR
    pusha
    push %ds
    push %es
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %esp, %eax
    add $(SIZEOF_jet_interrupt_context), %eax
    mov %eax, %ebp // %ebp points to the first stack frame
    push %esp // Interrupt frame is the only parameter to the followed functions.
    SAVE_FRAME
.endm

.macro INTERRUPT_PROLOGUE
    subl $4, %esp // As if error is on stack
    INTERRUPT_PROLOGUE_ERROR // Fallback to the common code
.endm



INTERRUPT_EPILOGUE:
    call update_tss
    addl $4, %esp
    pop %es
    pop %ds
    popa
    addl $4, %esp
    iret

{% for exception in exceptions %}

    .global exception_{{exception.id}}
    .type exception_{{exception.id}} ,@function
exception_{{exception.id}}:
{% if exception.set_error %}
    INTERRUPT_PROLOGUE_ERROR
{%else%}
    INTERRUPT_PROLOGUE
{%endif%}
    call exception_{{exception.id}}_handler
    jmp INTERRUPT_EPILOGUE
{% endfor %}
