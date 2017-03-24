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

#include <asp/space.h>
#include <assert.h>

//TODO
struct jet_fp_store
{
    uint64_t fp_regs[32];
    uint64_t fp_fpscr;
};

struct jet_fp_store* ja_alloc_fp_store(void)
{
    struct jet_fp_store* res = ja_mem_alloc_aligned(sizeof(*res), 4);
    return res;
}
