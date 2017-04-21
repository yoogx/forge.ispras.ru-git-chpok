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
#include <arinc653/process.h>
#include <stdio.h>
#include <conftree.h>

struct jet_kernel_shared_data* kshd;
jet_pt_tree_t part_config_tree;

void main(void);

/*
 * By default, main() function is called and expected to not return.
 *
 * It is possible to redefine this function to another wrapper around main.
 */
void __attribute__ ((weak)) __main(void)
{
    main();

    printf("ERROR: Main function returned.\n");

    STOP_SELF();
}

static void thread_print_message_after_return(void)
{
    PROCESS_STATUS_TYPE process_status;
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE process_id;

    GET_MY_ID(&process_id, &ret);

    if(ret == NO_ERROR) {
        // Normal thread, can request status for it.
        GET_PROCESS_STATUS(process_id, &process_status, &ret);
        printf("WARNING: Function for process '%s' returns. Forgot about STOP_SELF()?\n", process_status.ATTRIBUTES.NAME);
    }
    else {
        // Error thread.
        printf("WARNING: Error handler function returns. Forgot about STOP_SELF()?\n");
    }
}

/* Default wrapper around thread entry point. */
static void default_thread_entry_wrapper(void)
{
    // Call actual thread entry.
    struct jet_thread_shared_data* tshd = &kshd->tshd[kshd->current_thread_id];

    tshd->thread_entry_point();

    thread_print_message_after_return();

    STOP_SELF();
}



int __pok_partition_start (void)
{
   jet_memory_block_status_t kshd_status;

   if(jet_memory_block_get_status(".KSHD", &kshd_status) != EOK) {
       // Without kshd many things are not worked, even printf.
       abort();
   }

   kshd = (void*)kshd_status.addr;

   // Setup user-only fields of kernel shared data.
   kshd->main_thread_id = kshd->current_thread_id;
   kshd->error_thread_id = JET_THREAD_ID_NONE;

   jet_memory_block_status_t config_status;

   if(jet_memory_block_get_status(".CONFIG_TREE", &config_status) != EOK) {
       printf("ERROR: No '.CONFIG_TREE' memory block which contains partition's configuration tree");
       abort();
   }

   part_config_tree = (const void*)config_status.addr;

   if(jet_pt_check_magic(part_config_tree)) {
       printf("ERROR: Invalid format of the partition's configuration tree");
       abort();
   }

   smalloc_init();

   libjet_arinc_init();

   kshd->thread_entry_wrapper = &default_thread_entry_wrapper;

   __main(); /* main loop from user */

   return 0; // Unreachable
}
