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

#include <kernel_shared_data.h>
#include <init_arinc.h>
#include "smalloc_internal.h"
#include <memblocks.h>
#include <stdlib.h>

int main(void);

struct jet_kernel_shared_data* kshd;

int __pok_partition_start (void)
{
   jet_memory_block_status_t kshd_status;

   if(jet_memory_block_get_status(".KSHD", &kshd_status) != POK_ERRNO_OK) {
       // Without kshd many things are not worked, even printf.
       abort();
   }

   kshd = (void*)kshd_status.addr;

   // Setup user-only fields of kernel shared data.
   kshd->main_thread_id = kshd->current_thread_id;
   kshd->error_thread_id = JET_THREAD_ID_NONE;

   smalloc_init();

   libjet_arinc_init();

   main(); /* main loop from user */
   return (0);
}
