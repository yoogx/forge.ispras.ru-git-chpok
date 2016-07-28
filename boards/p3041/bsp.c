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
 *
 * This file also incorporates work covered by POK License.
 * Copyright (c) 2007-2009 POK team
 */

#include <config.h>

#include <errno.h>
#include <arch.h>
#include <core/debug.h>
#include <bsp_common.h>
#include "space.h"
#include <cons.h>

#include "devtree.h"

pok_bsp_t pok_bsp = {
    .ccsrbar_size = 0x1000000ULL,
    .ccsrbar_base = 0x0FE000000ULL,
    .ccsrbar_base_phys = 0x0FE000000ULL,
    .dcfg_offset = 0xE0000ULL,
    .serial0_regs_offset = 0x11C500ULL,
    .serial1_regs_offset = 0x11C600ULL,
    /* U-Boot claims that CCB freq (equal to platform clock freq) = 606.061 MHz
     * Timebase freq = platform_clock/32 */
    .timebase_freq = 18939406,
    .pci_bridge = {
        .cfg_addr = 0xfe200000,
        .cfg_data = 0xfe200004,
        .iorange =  0xf8000000
    }
};

int pok_bsp_init (void)
{
   pok_cons_init ();

   //devtree_dummy_dump();

   return (POK_ERRNO_OK);
}

void pok_bsp_get_info(void *addr) {
    pok_bsp_t *data = addr;
    *data = pok_bsp;
}

