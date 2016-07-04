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

#include <libc/gcov.h>
#include <core/syscall.h>

#include <stdio.h>

#ifdef POK_NEEDS_GCOV

#define DEFAULT_GCOV_ENTRY_COUNT 200

static struct gcov_info *gcov_info_head[DEFAULT_GCOV_ENTRY_COUNT];
static size_t num_used_gcov_entries = 0;

void pok_gcov_init(void) {
    extern uint32_t __PARTITION_CTOR_START__, __PARTITION_CTOR_END__; // linker defined symbols
    uint32_t start = (uint32_t)(&__PARTITION_CTOR_START__ + 1);
    uint32_t end = (uint32_t)(&__PARTITION_CTOR_END__ - 1);
    printf("start 0x%lx, end 0x%lx\n", start, end);
    void (**p)(void);

    while(start < end) {
        p = (void(**)(void))start; // get function pointer
        (*p)(); // call constructor
        start += sizeof(p);
    }
    pok_syscall2(POK_SYSCALL_GCOV_INIT, (uint32_t) gcov_info_head, num_used_gcov_entries);
}

void __gcov_init(struct gcov_info *info) {
    if (info == NULL) {
        printf("%s: NULL info\n", __func__);
        return;
    }
    //printf("%s filename '%s'\n", __func__, info->filename);

    if (num_used_gcov_entries >= DEFAULT_GCOV_ENTRY_COUNT) {
        printf("%s: gcov_info_head is full, all %zd entries used\n",
                __func__, num_used_gcov_entries);
        return;
    }

    gcov_info_head[num_used_gcov_entries++] = info;
}

#endif /* POK_NEEDS_GCOV */
