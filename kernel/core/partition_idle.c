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

// Implementation for idle partition.

#include <core/partition.h>
#include <common.h>
#include <asp/arch.h>

static void partition_idle_thread(void)
{
    ja_inf_loop();
}

static void partition_idle_process_error(pok_system_state_t partition_state,
        pok_error_id_t error_id,
        uint8_t state_byte_preempt_local,
        void* failed_address)
{
    pok_fatal("Error in idle partition");
}


static const struct pok_partition_operations partition_idle_operations =
{
    .start = partition_idle_thread,
    .process_partition_error = partition_idle_process_error
};

pok_partition_t partition_idle =
{
    .name = "Idle",

    .partition_event_max = 0,

    .period = 0,
    .space_id = 0,

    .part_sched_ops = &partition_sched_ops_kernel,
    .part_ops = &partition_idle_operations,

    .multi_partition_hm_selector = &pok_hm_multi_partition_selector_default,
    .multi_partition_hm_table = &pok_hm_multi_partition_table_default,
};
