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

#include <stdint.h>
#include <stdio.h>

#include <gcov.h>

#ifdef POK_NEEDS_GCOV

#define DEFAULT_GCOV_ENTRY_COUNT 200

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
}

void __gcov_init(struct gcov_info *info) {
    if (info == NULL) {
        printf("libpok: %s: NULL info\n", __func__);
        return;
    }

    if (gcov_part_data->num_used_gcov_entries >= DEFAULT_GCOV_ENTRY_COUNT) {
        printf("libpok: %s: gcov_info_head is full, all %zd entries used\n",
                __func__, gcov_part_data->num_used_gcov_entries);
        return;
    }

    gcov_part_data->gcov_info_head[gcov_part_data->num_used_gcov_entries++] = info;
}

#endif /* POK_NEEDS_GCOV */
