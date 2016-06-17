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

#ifndef __JET_MODULE_H__
#define __JET_MODULE_H__

#include <core/sched.h>
#include <core/partition.h>
#include <core/partition_arinc.h>
#include <core/error.h>
#include <arch.h>

/* Configuration of the module. */
struct jet_module_conf
{
#ifdef POK_NEEDS_MONITOR
    // Monitor partition, NULL if not used
    pok_partition_t* partition_monitor;
#endif
#ifdef POK_NEEDS_GDB
    // GDB partition, NULL if not used
    pok_partition_t* partition_gdb;
#endif
    // Array of schedule slots.
    const pok_sched_slot_t* module_sched;
    // Number of schedule slots in 'module_sched'
    uint8_t module_sched_n;
    // Major time frame.
    pok_time_t major_time_frame;
    // Array of ARINC partitions.
    pok_partition_arinc_t* partitions_arinc;
    // Number of ARINC partitions.
    uint8_t partitions_arinc_n;
    // Array of queuing channels (*intra*-module)
    pok_channel_queuing_t* channels_queuing;
    // Number of queuing channels (*intra*-module)
    uint8_t channels_queuing_n;
    // Array of sampling channels (*intra*-module)
    pok_channel_sampling_t* channels_sampling;
    // Number of sampling channels (*intra*-module)
    uint8_t channels_sampling_n;
    // Module-level error selector.
    pok_error_level_selector_t* hm_module_selector;
    // Module HM table.
    pok_error_module_action_table_t* hm_module_table;

    // Pointer to array of spaces for partitions. TODO: This should be arch-specific part
    struct pok_space* spaces;
    uint8_t spaces_n;

};

// Setup module according to configuration.
void jet_module_setup(const struct jet_module_conf* conf);

#endif /* __JET_MODULE_H__ */
