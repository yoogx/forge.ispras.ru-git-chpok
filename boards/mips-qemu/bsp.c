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

    .ccsrbar_size = 0x1000000ULL, //?
    .ccsrbar_base = 0x09FF00000ULL,
    .ccsrbar_base_phys = 0x09FF00000ULL,
    .dcfg_offset = 0xE0000ULL, //?
    .serial0_regs_offset = 0x70000ULL,
    .serial1_regs_offset = 0x70008ULL,
    .timebase_freq = 400000000, //?
    .pci_bridge = {
        .cfg_addr = 0xe0008000, //?
        .cfg_data = 0xe0008004, //?
        .iorange =  0xe1000000  //?
    }///FIXIT for RS-232C controller
};

extern char _end[];

void ja_bsp_init (void)
{
   jet_console_init_all ();

   //devtree_dummy_dump();
   if ((uintptr_t) _end > 0x84000000ULL)    //Kernel starts from 0x80000000
       pok_fatal("Kernel size is more than 64 megabytes");
}
