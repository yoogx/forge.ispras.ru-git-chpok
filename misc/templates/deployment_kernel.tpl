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

#include <config.h>
#include <types.h>
#include <core/error.h>
#include <core/error_arinc.h>
#include <core/partition_arinc.h>
#include <core/partition.h>
#include <core/memblocks.h>

/*********************** HM module tables *****************************/
/*
 * HM module selector.
 */
{% set module_hm_table = conf.module_hm_table %}
pok_error_level_selector_t pok_hm_module_selector = {
    .levels = {
{%for error_id in module_hm_table.error_ids%}
        {{module_hm_table.level_selector_total(error_id)}}, /* POK_ERROR_ID_{{error_id}} */
{%endfor%}
    }
};

/*
 * HM module table.
 */
pok_error_module_action_table_t pok_hm_module_table = {
    .actions = {
{%for system_state in module_hm_table.system_states%}
    /* POK_SYSTEM_STATE_{{system_state}} */
    {
{%for error_id in module_hm_table.error_ids%}
        POK_ERROR_MODULE_ACTION_{{module_hm_table.recovery_action(system_state, error_id)}}, /* POK_ERROR_ID_{{error_id}}*/
{%endfor%}
    },
{%endfor%}
    }
};

{%macro connection_partition(connection)%}
{%if connection %}
&pok_partitions_arinc[{{connection.partition_index}}].base_part
{%-else%}
NULL
{%-endif%}
{%-endmacro%}

/**************** Setup queuing channels ****************************/
pok_channel_queuing_t pok_channels_queuing[{{ conf.channels_queueing | length }}] = {
    {%for channel_queueing in conf.channels_queueing%}
    {
        .max_message_size = {{channel_queueing.max_message_size}},

        .recv = {
            .max_nb_message = {{channel_queueing.max_nb_message_receive}},
            .part = {{connection_partition(channel_queueing.dst)}},
        },
        .send = {
            .max_nb_message = {{channel_queueing.max_nb_message_send}},
            .part = {{connection_partition(channel_queueing.src)}},
        },

        // Currently hardcoded.
        .overflow_strategy = JET_CHANNEL_QUEUING_SENDER_BLOCK,
    },
    {%endfor%}
};

uint8_t pok_channels_queuing_n = {{ conf.channels_queueing | length }};

/****************** Setup sampling channels ***************************/
pok_channel_sampling_t pok_channels_sampling[{{ conf.channels_sampling | length }}] = {
    {%for channel_sampling in conf.channels_sampling%}
    {
        .max_message_size = {{channel_sampling.max_message_size}},
    },
    {%endfor%}
};

uint8_t pok_channels_sampling_n = {{ conf.channels_sampling | length }};

{%for part in conf.partitions%}

{%set pmd = memory_definitions.partitions[loop.index0]%}

/****************** Setup partition{{loop.index0}} (auxiliary) **********************/
{%set partition_hm_table = part.partition_hm_table%}
// HM partition level selector.
static const pok_error_level_selector_t partition_hm_selector_{{loop.index0}} = {
    .levels = {
{%for error_id in partition_hm_table.error_ids %}
        {{partition_hm_table.level_selector_total(error_id)}}, /*POK_ERROR_ID_{{error_id}}*/
{%endfor%}
    }
};
// Mapping of process-level errors information.
static const pok_thread_error_map_t partition_thread_error_info_{{loop.index0}} = {
    .map = {

{%for error_id in partition_hm_table.error_ids %}
{%set action = partition_hm_table.actions['USER'][error_id]%}
{% if action.error_code %}
        {POK_ERROR_KIND_{{action.error_code}}, "{{action.error_description}}"},        /* POK_ERROR_ID_{{error_id}} */
{%else%}
        {POK_ERROR_KIND_INVALID, NULL}, /*POK_ERROR_ID_{{error_id}}*/
{%endif%}
{%endfor%}
    }
};

/*
 * Pointer to partition HM table.
 */
static const pok_error_hm_partition_t partition_hm_table_{{loop.index0}} = {
    .actions = {
{%for s in partition_hm_table.system_states %}
    /* POK_SYSTEM_STATE_{{s}} */
    {
{%for error_id in partition_hm_table.error_ids %}
        POK_ERROR_ACTION_{{partition_hm_table.recovery_action(s, error_id, 'IDLE')}}, /* POK_ERROR_ID_{{error_id}} */
{%endfor%}
    },
{%endfor%}
    }
};

// Threads array
static pok_thread_t partition_threads_{{loop.index0}}[{{part.num_threads_total}}];

// Queuing ports
static pok_port_queuing_t partition_ports_queuing_{{loop.index0}}[{{part.ports_queueing | length}}] = {
{%for port_queueing in part.ports_queueing%}
    {
        .name = "{{port_queueing.name}}",
        .channel = {%if port_queueing.queueing_channel_index is not none%}&pok_channels_queuing[{{port_queueing.queueing_channel_index}}]{%else%}NULL{%endif%},
        .direction = {%if port_queueing.direction == 'OUT'%}POK_PORT_DIRECTION_OUT{%else%}POK_PORT_DIRECTION_IN{%endif%},
    },
{%endfor%}
};

// Sampling ports
static pok_port_sampling_t partition_ports_sampling_{{loop.index0}}[{{part.ports_sampling | length}}] = {
{%for port_sampling in part.ports_sampling%}
    {
        .name = "{{port_sampling.name}}",
        .channel = {%if port_sampling.sampling_channel_index is not none%}&pok_channels_sampling[{{port_sampling.sampling_channel_index}}]{%else%}NULL{%endif%},
        .direction = {%if port_sampling.direction == 'OUT'%}POK_PORT_DIRECTION_OUT{%else%}POK_PORT_DIRECTION_IN{%endif%},
    },
{%endfor%}
};

// Memory blocks
static const struct memory_block memory_blocks_{{loop.index0}}[{{pmd.memory_blocks | length}}] = {
{%for mbd in pmd.memory_blocks%}
	{
		.name = "{{mbd.name}}",
		.size = {{mbd.size}},
		.maccess = 0{%if 'R' in mbd.access%} | MEMORY_BLOCK_ACCESS_READ{%endif%}{%if 'W' in mbd.access%} | MEMORY_BLOCK_ACCESS_WRITE{%endif%}{%if 'X' in mbd.access%} | MEMORY_BLOCK_ACCESS_EXEC{%endif%},
		.vaddr = {{mbd.vaddr}},
		.is_contiguous = {%if mbd.is_contiguous%}TRUE{%else%}FALSE{%endif%},
		.paddr = {%if mbd.is_contiguous%}{{mbd.paddr}}{%else%}0{%endif%},
		.is_shared = {%if mbd.is_shared%}TRUE{%else%}FALSE{%endif%},
		.kaddr = {{mbd.kaddr}},
	},
{%endfor%}
};

{%endfor%}{#partitions loop#}

/*************** Setup partitions array *******************************/
pok_partition_arinc_t pok_partitions_arinc[{{conf.partitions | length}}] = {
{%for part in conf.partitions%}
{%set pmd = memory_definitions.partitions[loop.index0]%}
    {
        .base_part = {
            .name = "{{part.name}}",

            // Allocate 1 event slot per queuing port plus 2 slots for timer.
            .partition_event_max = {{part.ports_queueing | length}} + 2,

            .period = {{part.period}},
            .duration = {{part.duration}},
            .partition_id = {{part.part_id}},

            .space_id = {{part.space_id}},

            .multi_partition_hm_selector = &pok_hm_multi_partition_selector_default,
            .multi_partition_hm_table = &pok_hm_multi_partition_table_default,

            .memory_blocks = memory_blocks_{{loop.index0}},
            .nmemory_blocks = {{pmd.memory_blocks | length}},
        },

		.elf_id = {{part.index}},

        .nthreads = {{part.num_threads_total}},
        .threads = partition_threads_{{loop.index0}},

        .main_user_stack_size = 8192, {# TODO: This should be set in config somehow. #}

        .heap_size = {{part.heap_size}},

        .ports_queuing = partition_ports_queuing_{{loop.index0}},
        .nports_queuing = {{part.ports_queueing | length}},

        .ports_sampling = partition_ports_sampling_{{loop.index0}},
        .nports_sampling = {{part.ports_sampling | length}}, {#TODO: ports#}

        .partition_hm_selector = &partition_hm_selector_{{loop.index0}},

        .thread_error_info = &partition_thread_error_info_{{loop.index0}},

        .partition_hm_table = &partition_hm_table_{{loop.index0}},
    },
{%endfor%}{#partitions loop#}
};

const uint8_t pok_partitions_arinc_n = {{conf.partitions | length}};

#ifdef POK_NEEDS_MONITOR
/**************************** Monitor *********************************/
pok_partition_t partition_monitor =
{
    .name = "Monitor",

    .partition_event_max = 0,

    .period = {{conf.major_frame}}, {#TODO: Where it is stored in conf?#}

    .space_id = 0,

    .multi_partition_hm_selector = &pok_hm_multi_partition_selector_default,
    .multi_partition_hm_table = &pok_hm_multi_partition_table_default,
};
#endif /* POK_NEEDS_MONITOR*/
#ifdef POK_NEEDS_GDB
/******************************* GDB **********************************/
pok_partition_t partition_gdb =
{
    .name = "GDB",

    .partition_event_max = 0,

    .period = {{conf.major_frame}}, {#TODO: Where it is stored in conf?#}

    .space_id = 0,

    .multi_partition_hm_selector = &pok_hm_multi_partition_selector_default,
    .multi_partition_hm_table = &pok_hm_multi_partition_table_default,
};
#endif /* POK_NEEDS_GDB*/

/************************* Setup time slots ***************************/
const pok_sched_slot_t pok_module_sched[{{conf.slots | length}}] = {
{%for slot in conf.slots%}
    {
        .duration = {{slot.duration}},
        .offset = {{slot.offset}},
    {%if slot.slot_type == 'PARTITION' %}
        .partition = &pok_partitions_arinc[{{slot.partition_index}}].base_part,
        .periodic_processing_start = {%if slot.periodic_processing_start%}TRUE{%else%}FALSE{%endif%},
    {%elif slot.slot_type == 'MONITOR' %}
#ifdef POK_NEEDS_MONITOR
        .partition = &partition_monitor,
#else /* POK_NEEDS_MONITOR */
        .partition = &partition_idle,
#endif /* POK_NEEDS_MONITOR */
        .periodic_processing_start = FALSE,
    {%elif slot.slot_type == 'GDB' %}
#ifdef POK_NEEDS_GDB
        .partition = &partition_gdb,
#else /* POK_NEEDS_GDB */
        .partition = &partition_idle,
#endif /* POK_NEEDS_GDB */
        .periodic_processing_start = FALSE,
    {%endif%}
        .id = {{loop.index0}}
    },
{%endfor%}
};

const uint8_t pok_module_sched_n = {{conf.slots | length}};

const pok_time_t pok_config_scheduling_major_frame = {{conf.major_frame}};
