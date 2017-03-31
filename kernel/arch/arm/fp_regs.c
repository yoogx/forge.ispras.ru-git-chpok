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

#ifdef JET_ARM_CONFIG_FPU
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

#else
// floating points are not supported

struct jet_fp_store;

struct jet_fp_store* ja_alloc_fp_store(void)
{
    // there is an assert othervise
    return (void *)1;
}

void ja_fp_save(struct jet_fp_store* fp_store)
{
    (void) fp_store;
}
void ja_fp_restore(struct jet_fp_store* fp_store)
{
    (void) fp_store;
}
void ja_fp_init(void) {}

void floating_point_enable(void) {}
#endif //JET_ARM_CONFIG_FPU

