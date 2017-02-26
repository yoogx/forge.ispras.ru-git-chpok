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

#include "cons.h"
#include "pit.h"
#include "pic.h"
#include <asp/entries.h>

#include <assert.h>

void ja_bsp_init (void)
{
   pok_pic_init ();
   pok_x86_qemu_timer_init ();
}

void pok_bsp_get_info(void *addr) {
    pok_fatal("pok_bsp_get_info unimplemented on x86");
}


