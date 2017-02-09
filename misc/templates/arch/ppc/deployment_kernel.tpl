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

struct ja_ppc_space ja_spaces[{{conf.spaces | length}}] =
{
{%for space in conf.spaces%}
    {
        .phys_base = 0x4000000UL + {{loop.index0}} * POK_PARTITION_MEMORY_SIZE,
        .size_normal = {{space.size}},
        .size_heap = {{space.part.get_heap_size()}},
        // .size_total is calculated on initialization.
    },
{%endfor%}
};

int ja_spaces_n = {{conf.spaces | length}};

/************************ Memory mapping ************************/

struct tlb_entry jet_tlb_entries[] = {
    {%for mblock in conf.memory_blocks%}
    // {{mblock.name}}
    {%for pid, access_right in mblock.access.iteritems() %}
    {
        .virt_addr = {{"0x%x" | format(mblock.virt_addr)}},
        .phys_addr = {{"0x%x" | format(mblock.phys_addr)}},
        .size = E500MC_PGSIZE_{{mblock.str_size}},
        .permissions = MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR,
        {%if mblock.cache_policy == "IO"%}
        .cache_policy = MAS2_W | MAS2_I | MAS2_M | MAS2_G,
        {% endif %}
        .pid = {{pid}},
    },
    {%endfor%}

    {%endfor%}

};

size_t jet_tlb_entries_n = {{ conf.memory_blocks_tlb_entries_count }};
