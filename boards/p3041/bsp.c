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

#include <core/debug.h>
#include <bsp/bsp.h>
#include <asp/entries.h>

pok_bsp_t pok_bsp = {
    .ccsrbar_size = 0x1000000ULL,
    .ccsrbar_base = 0xFE000000ULL,
    .ccsrbar_base_phys = 0xFE000000Ull,
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

extern char _end[];

void ja_bsp_init (void)
{
   jet_console_init_all ();
}
