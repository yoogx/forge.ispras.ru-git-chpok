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

#ifndef __LIBJET_ARM_SYSCALL_H__
#define __LIBJET_ARM_SYSCALL_H__


#define LIBJET_ARCH_DECLARE_SYSCALL 1

//copy of pok_syscall_args_t from kernel core/syscall.h
typedef struct
{
    uint32_t nargs;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t arg4;
    uint32_t arg5;
} pok_syscall_args_t;

// next is copy of x86 code

pok_ret_t lja_do_syscall(pok_syscall_id_t syscall_id, pok_syscall_args_t *args);

#define lja_syscall0(sid) \
    lja_do_syscall(sid,&((pok_syscall_args_t){0,0,0,0,0,0}))

#define lja_syscall1(sid,arg1) \
    lja_do_syscall(sid,&((pok_syscall_args_t){1,arg1,0,0,0,0}))

#define lja_syscall2(sid,arg1,arg2) \
    lja_do_syscall(sid,&((pok_syscall_args_t){2,arg1,arg2,0,0,0}))

#define lja_syscall3(sid,arg1,arg2,arg3) \
    lja_do_syscall(sid,&((pok_syscall_args_t){3,arg1,arg2,arg3,0,0}))

#define lja_syscall4(sid,arg1,arg2,arg3,arg4) \
    lja_do_syscall(sid,&((pok_syscall_args_t){4,arg1,arg2,arg3,arg4,0}))

#define lja_syscall5(sid,arg1,arg2,arg3,arg4,arg5) \
    lja_do_syscall(sid,&((pok_syscall_args_t){5,arg1,arg2,arg3,arg4,arg5}))

#endif
