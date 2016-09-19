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

#include <stddef.h>

#define DEFINE(sym, val) asm volatile("\n-> " #sym " %0 " #val "\n" : : "i" (val))
#define OFFSETOF(s, m) \
    DEFINE(OFFSETOF_##s##_##m, offsetof(struct s, m));

#define SIZEOF_STRUCT(s) \
    DEFINE(SIZEOF_##s, sizeof(struct s));

#define AS_IS(str) asm volatile("\n->#" str)
