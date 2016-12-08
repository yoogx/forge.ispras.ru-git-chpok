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

struct ja_x86_space ja_spaces[{{conf.spaces | length}}] =
{
{%for space in conf.spaces%}
    {
        //.phys_base is filled upon initialization
        .size_normal = {{space.size}},
        .size_heap = {{space.part.get_heap_size()}},
        // Currently stack size is hardcoded to 8K.
        .size_stack = {{space.part.get_needed_threads()}} * 8 * 1024
    },
{%endfor%}
};

int ja_spaces_n = {{conf.spaces | length}};
