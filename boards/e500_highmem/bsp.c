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
#include "cons.h"
#include <bsp_common.h>
#include "space.h"

#include "devtree.h"

pok_bsp_t pok_bsp = {
    .ccsrbar_size = 0x1000000ULL,
    .ccsrbar_base = 0xE0000000ULL,
    .ccsrbar_base_phys = 0xFE0000000ULL,
    .dcfg_offset = 0xE0000UL,
    .serial0_regs_offset = 0x4500ULL,
    .serial1_regs_offset = 0x4600ULL,
    .timebase_freq = 400000000,
    .pci_bridge = {
        .cfg_addr = 0xe0008000,
        .cfg_data = 0xe0008004,
        .iorange =  0xe1000000
    }
};

extern char _end[];

int pok_bsp_init (void)
{
   pok_cons_init ();

   //devtree_dummy_dump();
   if ((uintptr_t) _end > 0x4000000ULL)
       pok_fatal("Kernel size is more than 64 megabytes");

   return (POK_ERRNO_OK);
}


static char *heap_end = _end;

void *pok_bsp_mem_alloc (size_t sz)
{
  char *res;

  res = (char *)(((unsigned int)heap_end + 4095) & ~4095);
  heap_end = res + sz;
  return res;
}

void * pok_bsp_alloc_partition(size_t size)
{
    static uintptr_t last_base             = 0x4000000ULL; // 64 mebibytes
    static const size_t max_partition_size = POK_PARTITION_MEMORY_SIZE; // 16 mebibytes
    uintptr_t res;
    
    // TODO it should be more flexible than this
    if (size > max_partition_size) pok_fatal("partition size is too big");

    res = last_base;
    last_base += max_partition_size;

    // TODO check that we aren't out of RAM bounds

    return (void *) res;
}

//This is only used by virtio where alignment=4096
void *pok_bsp_mem_alloc_aligned(size_t mem_size, size_t alignment) 
{
    if (alignment == 4096)
        return pok_bsp_mem_alloc(mem_size);

    pok_fatal("unimplemented!");
}

void pok_bsp_get_info(void *addr) {
    pok_bsp_t *data = addr;
    *data = pok_bsp;
}

