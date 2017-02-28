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

#include <arch/deployment.h>
#include <arch/mmu_ext.h>

struct tlb_entry tlb_entries[] = {
{%for entry in entries%}
    {
        .virt_addr = {{"0x%x"|format(entry.vaddr)}},
        .phys_addr = {{"0x%x"|format(entry.paddr)}},
        .half_size = MIPS_PGSIZE_{{entry.size_enum}},
        .permissions = {{entry.permissions}},
        .cache_policy = {{entry.cache_policy}},
        .pid = {{entry.pid}},
    },
{%endfor%}
};

int tlb_entries_n = {{entries | length}};
