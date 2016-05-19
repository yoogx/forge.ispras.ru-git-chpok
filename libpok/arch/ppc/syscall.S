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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */
        .text
        .globl pok_syscall0
        .globl pok_syscall1
        .globl pok_syscall2
        .globl pok_syscall3
        .globl pok_syscall4
        .globl pok_syscall5
pok_syscall0:
pok_syscall1:
pok_syscall2:
pok_syscall3:
pok_syscall4:
pok_syscall5:
        sc
        blr
