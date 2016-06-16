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

#include <module.h>

void jet_module_setup(const struct jet_module_conf* conf)
{
#ifdef POK_NEEDS_MONITOR
    // Monitor partition, NULL if not used
    partition_monitor = conf->partition_monitor;
#endif
#ifdef POK_NEEDS_GDB
    // GDB partition, NULL if not used
    partition_gdb = conf->partition_gdb;
#endif
    // Array of schedule slots.
    pok_module_sched = conf->module_sched;
    // Number of schedule slots in 'module_sched'
    pok_module_sched_n = conf->module_sched_n;
    // Major time frame.
    pok_config_scheduling_major_frame = conf->major_time_frame;
    // Array of ARINC partitions.
    pok_partitions_arinc = conf->partitions_arinc;
    // Number of ARINC partitions.
    pok_partitions_arinc_n = conf->partitions_arinc_n;
    // Array of queuing channels (*intra*-module)
    pok_channels_queuing = conf->channels_queuing;
    // Number of queuing channels (*intra*-module)
    pok_channels_queuing_n = conf->channels_queuing_n;
    // Array of sampling channels (*intra*-module)
    pok_channels_sampling = conf->channels_sampling;
    // Number of sampling channels (*intra*-module)
    pok_channels_sampling_n = conf->channels_sampling_n;
    // Module-level error selector.
    pok_hm_module_selector = conf->hm_module_selector;
    // Module HM table.
    pok_hm_module_table = conf->hm_module_table;
}
