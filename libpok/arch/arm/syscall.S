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
 */
        .text
        .globl lja_syscall0
        .globl lja_syscall1
        .globl lja_syscall2
        .globl lja_syscall3
        .globl lja_syscall4
        .globl lja_syscall5
lja_syscall0:
lja_syscall1:
lja_syscall2:
lja_syscall3:
lja_syscall4:
lja_syscall5:
        svc #0
