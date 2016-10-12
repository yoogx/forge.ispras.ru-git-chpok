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

struct ja_ppc_space ja_spaces[{{conf.spaces | length}}] =
{
{%for space in conf.spaces%}
    {
        .phys_base = POK_PARTITION_PHYS_MEMORY_BASE + {{loop.index0}} * POK_PARTITION_MEMORY_SIZE,
        .size_normal = {{space.size}},
    },
{%endfor%}
};

int ja_spaces_n = {{conf.spaces | length}};
