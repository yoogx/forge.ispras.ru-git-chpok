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
#include <arch/mmu.h>

{%for part_pages in partitions_pages%}
struct page pages_{{loop.index}}[] = {
    {% for page in part_pages %}
    {
        .vaddr = {{"0x%x"|format(page.vaddr)}},
        .paddr = {{"0x%x"|format(page.paddr)}},
        .flags = {{page.flags if page.flags else '0'}},
        .size  = {{page.size if page.size else '0'}},
    },
    {% endfor %}
};
{%endfor%}

struct partition_pages ja_partitions_pages[] = {
    {%for part_pages in partitions_pages%}
    {
        .pages = pages_{{loop.index}},
        .len = {{part_pages | length}}
    },
    {%endfor%}
};

size_t ja_partitions_pages_nb = {{partitions_pages | length}};
