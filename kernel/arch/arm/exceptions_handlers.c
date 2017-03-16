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

#include <core/syscall.h>
#include <core/uaccess.h>
#include "regs.h"

struct interrupt_context {
    uint32_t fp;
    uint32_t svc_lr;
    uint32_t adj;
    uint32_t r12;
    uint32_t r3;
    uint32_t r2;
    uint32_t r1;
    uint32_t r0;
    uint32_t saved_psr;
    uint32_t saved_pc;
};

pok_ret_t syscall_handler(
        pok_syscall_id_t syscall_id,
        pok_syscall_args_t* __user args)
{
    pok_syscall_args_t* syscall_args_ptr = jet_user_to_kernel_typed(args);

    if(syscall_args_ptr == NULL) {
        return POK_ERRNO_EINVAL;
    } else {
        pok_syscall_args_t  syscall_args_kernel; //syscall args in kernel address space
        memcpy(&syscall_args_kernel, syscall_args_ptr, sizeof(pok_syscall_args_t));
        return pok_core_syscall (syscall_id, &syscall_args_kernel);
    }
}

void prefetch_abort_handler(struct interrupt_context *ictx)
{
    void *fault_addres = (void *)ifar_get();
    int is_user = ictx->saved_psr & CPSR_MODE_SYS;

#ifdef POK_NEEDS_DEBUG
    printf("%s code at %p address tried to execute code on %p address\n",
            is_user? "USER" : "KERNEL",
            (void *) ictx->saved_pc,
            fault_addres);
    printf("IFSR = %lx\n", ifsr_get());
#endif

    pok_raise_error(POK_ERROR_ID_MEMORY_VIOLATION, is_user, fault_addres);
}

void data_abort_handler(struct interrupt_context *ictx)
{
    void *fault_addres = (void *)dfar_get();
    int is_user = ictx->saved_psr & CPSR_MODE_SYS;

#ifdef POK_NEEDS_DEBUG
    printf("%s code at %p address tried to access data on %p address\n",
            is_user? "USER" : "KERNEL",
            (void *) ictx->saved_pc,
            fault_addres);
    printf("DFSR = %lx\n", dfsr_get());
#endif

    pok_raise_error(POK_ERROR_ID_MEMORY_VIOLATION, is_user, fault_addres);
}

void fiq_handler()
{
    //not supported
    assert(0);
}

void undefined_instruction_handler()
{
    printf("bad\n");
    assert(0);
}
