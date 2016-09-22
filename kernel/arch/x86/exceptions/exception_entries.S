/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (kernel/arch/x86/exceptions/exceptions.yaml).
 */
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
    mov $0, %ebp // Mark current frame as first
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


    .global exception_DIVIDE_ERROR
    .type exception_DIVIDE_ERROR ,@function
exception_DIVIDE_ERROR:
    INTERRUPT_PROLOGUE
    call exception_DIVIDE_ERROR_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_DEBUG
    .type exception_DEBUG ,@function
exception_DEBUG:
    INTERRUPT_PROLOGUE
    call exception_DEBUG_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_NMI
    .type exception_NMI ,@function
exception_NMI:
    INTERRUPT_PROLOGUE
    call exception_NMI_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_BREAKPOINT
    .type exception_BREAKPOINT ,@function
exception_BREAKPOINT:
    INTERRUPT_PROLOGUE
    call exception_BREAKPOINT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_OVERFLOW
    .type exception_OVERFLOW ,@function
exception_OVERFLOW:
    INTERRUPT_PROLOGUE
    call exception_OVERFLOW_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_INVALIDOPCODE
    .type exception_INVALIDOPCODE ,@function
exception_INVALIDOPCODE:
    INTERRUPT_PROLOGUE
    call exception_INVALIDOPCODE_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_NOMATH_COPROC
    .type exception_NOMATH_COPROC ,@function
exception_NOMATH_COPROC:
    INTERRUPT_PROLOGUE
    call exception_NOMATH_COPROC_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_DOUBLEFAULT
    .type exception_DOUBLEFAULT ,@function
exception_DOUBLEFAULT:
    INTERRUPT_PROLOGUE
    call exception_DOUBLEFAULT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_COPSEG_OVERRUN
    .type exception_COPSEG_OVERRUN ,@function
exception_COPSEG_OVERRUN:
    INTERRUPT_PROLOGUE
    call exception_COPSEG_OVERRUN_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_INVALID_TSS
    .type exception_INVALID_TSS ,@function
exception_INVALID_TSS:
    INTERRUPT_PROLOGUE_ERROR
    call exception_INVALID_TSS_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_SEGMENT_NOT_PRESENT
    .type exception_SEGMENT_NOT_PRESENT ,@function
exception_SEGMENT_NOT_PRESENT:
    INTERRUPT_PROLOGUE_ERROR
    call exception_SEGMENT_NOT_PRESENT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_STACKSEG_FAULT
    .type exception_STACKSEG_FAULT ,@function
exception_STACKSEG_FAULT:
    INTERRUPT_PROLOGUE_ERROR
    call exception_STACKSEG_FAULT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_GENERAL_PROTECTION
    .type exception_GENERAL_PROTECTION ,@function
exception_GENERAL_PROTECTION:
    INTERRUPT_PROLOGUE_ERROR
    call exception_GENERAL_PROTECTION_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_PAGEFAULT
    .type exception_PAGEFAULT ,@function
exception_PAGEFAULT:
    INTERRUPT_PROLOGUE_ERROR
    call exception_PAGEFAULT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_FPU_FAULT
    .type exception_FPU_FAULT ,@function
exception_FPU_FAULT:
    INTERRUPT_PROLOGUE
    call exception_FPU_FAULT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_ALIGNEMENT_CHECK
    .type exception_ALIGNEMENT_CHECK ,@function
exception_ALIGNEMENT_CHECK:
    INTERRUPT_PROLOGUE_ERROR
    call exception_ALIGNEMENT_CHECK_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_MACHINE_CHECK
    .type exception_MACHINE_CHECK ,@function
exception_MACHINE_CHECK:
    INTERRUPT_PROLOGUE
    call exception_MACHINE_CHECK_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_SIMD_FAULT
    .type exception_SIMD_FAULT ,@function
exception_SIMD_FAULT:
    INTERRUPT_PROLOGUE
    call exception_SIMD_FAULT_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_SYSCALL
    .type exception_SYSCALL ,@function
exception_SYSCALL:
    INTERRUPT_PROLOGUE
    call exception_SYSCALL_handler
    jmp INTERRUPT_EPILOGUE

    .global exception_TIMER
    .type exception_TIMER ,@function
exception_TIMER:
    INTERRUPT_PROLOGUE
    call exception_TIMER_handler
    jmp INTERRUPT_EPILOGUE
