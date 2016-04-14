#include <types.h>
#include <core/error.h>
#include <core/error_arinc.h>
#include <core/partition_arinc.h>
#include <core/partition.h>

/*********************** HM module tables *****************************/
/*
 * Default HM module selector.
 * 
 * For common errors id select partition only in partition-related
 * states (0x70).
 * 
 * Note, that some errors are handled by the module even in
 * partition-related states (0).
 */

pok_error_level_selector_t pok_hm_module_selector = {
    .levels = {
        0,      /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
        0x70,   /* POK_ERROR_ID_ILLEGAL_REQUEST */
        0x70,   /* POK_ERROR_ID_APPLICATION_ERROR */
        0,      /* POK_ERROR_ID_PARTLOAD_ERROR */
        0x70,   /* POK_ERROR_ID_NUMERIC_ERROR */
        0x70,   /* POK_ERROR_ID_MEMORY_VIOLATION */
        0x70,   /* POK_ERROR_ID_DEADLINE_MISSED */
        0,      /* POK_ERROR_ID_HARDWARE_FAULT */
        0,      /* POK_ERROR_ID_POWER_FAIL */
        0x70,   /* POK_ERROR_ID_STACK_OVERFLOW */
        0,      /* POK_ERROR_ID_PROCINIT_ERROR */
        0,      /* POK_ERROR_ID_NOMEMORY_PROCDATA */
        0,      /* POK_ERROR_ID_ASSERT */
        0,      /* POK_ERROR_ID_CONFIG_ERROR */
        0,      /* POK_ERROR_ID_CHECK_POOL */
        0,      /* POK_ERROR_ID_UNHANDLED_INT */
    }
};

/*
 * Default HM module table.
 * 
 * SHUTDOWN for all errors.
 */
pok_error_module_action_table_t pok_hm_module_table = {
    .actions = {
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_INIT_PARTOS */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_INIT_PARTUSER */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_INTERRUPT_HANDLER */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_OS_MOD */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_OS_PART */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_ERROR_HANDLER */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_USER */
    }
};

/********************* HM Multi-partition tables **********************/
// Default HM multi-partition selector - all errors are partition-level.
pok_error_level_selector_t hm_multi_partition_selector_default = {
    .levels = {
        0x7f,   /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
        0x7f,   /* POK_ERROR_ID_ILLEGAL_REQUEST */
        0x7f,   /* POK_ERROR_ID_APPLICATION_ERROR */
        0x7f,   /* POK_ERROR_ID_PARTLOAD_ERROR */
        0x7f,   /* POK_ERROR_ID_NUMERIC_ERROR */
        0x7f,   /* POK_ERROR_ID_MEMORY_VIOLATION */
        0x7f,   /* POK_ERROR_ID_DEADLINE_MISSED */
        0x7f,   /* POK_ERROR_ID_HARDWARE_FAULT */
        0x7f,   /* POK_ERROR_ID_POWER_FAIL */
        0x7f,   /* POK_ERROR_ID_STACK_OVERFLOW */
        0x7f,   /* POK_ERROR_ID_PROCINIT_ERROR */
        0x7f,   /* POK_ERROR_ID_NOMEMORY_PROCDATA */
        0x7f,   /* POK_ERROR_ID_ASSERT */
        0x7f,   /* POK_ERROR_ID_CONFIG_ERROR */
        0x7f,   /* POK_ERROR_ID_CHECK_POOL */
        0x7f,   /* POK_ERROR_ID_UNHANDLED_INT */
    }
};

// Default HM multi-partition table - shutdown for all errors.
pok_error_module_action_table_t hm_multi_partition_table_default = {
    .actions = {
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_INIT_PARTOS */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_INIT_PARTUSER */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_INTERRUPT_HANDLER */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_OS_MOD */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_OS_PART */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_ERROR_HANDLER */
        {POK_ERROR_MODULE_ACTION_SHUTDOWN, }, /* POK_SYSTEM_STATE_USER */
    }
};

/****************** Setup queuing channels ****************************/
pok_channel_queuing_t pok_channels_queuing[{{ conf.channels_queueing | length }}] = {
    <$for channel_queueing in conf.channels_queueing$>
    {
        .max_message_size = {{channel_queueing.max_message_size}},
        .max_nb_message_send = {{channel_queueing.max_nb_message_send}},
        .max_nb_message_receive = {{channel_queueing.max_nb_message_receive}},
    },
    <$endfor$>
};

uint8_t pok_channels_queuing_n = {{ conf.channels_queueing | length }};

/****************** Setup sampling channels ***************************/
pok_channel_sampling_t pok_channels_sampling[{{ conf.channels_sampling | length }}] = {
    <$for channel_sampling in conf.channels_sampling$>
    {
        .max_message_size = {{channel_sampling.max_message_size}},
    },
    <$endfor$>
};

uint8_t pok_channels_sampling_n = {{ conf.channels_sampling | length }};

<$for part in conf.partitions$>
/****************** Setup partition{{loop.index0}} (auxiliary) **********************/
<# TODO: Parse errors from input #>
// HM partition level selector.
static const pok_error_level_selector_t partition_hm_selector_{{loop.index0}} = {
    .levels = {
        0,      /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
        0x40,   /* POK_ERROR_ID_ILLEGAL_REQUEST */
        0x40,   /* POK_ERROR_ID_APPLICATION_ERROR */
        0,      /* POK_ERROR_ID_PARTLOAD_ERROR */
        0x40,   /* POK_ERROR_ID_NUMERIC_ERROR */
        0x40,   /* POK_ERROR_ID_MEMORY_VIOLATION */
        0x40,   /* POK_ERROR_ID_DEADLINE_MISSED */
        0,      /* POK_ERROR_ID_HARDWARE_FAULT */
        0,      /* POK_ERROR_ID_POWER_FAIL */
        0x40,   /* POK_ERROR_ID_STACK_OVERFLOW */
        0,      /* POK_ERROR_ID_PROCINIT_ERROR */
        0,      /* POK_ERROR_ID_NOMEMORY_PROCDATA */
        0,      /* POK_ERROR_ID_ASSERT */
        0,      /* POK_ERROR_ID_CONFIG_ERROR */
        0,      /* POK_ERROR_ID_CHECK_POOL */
        0,      /* POK_ERROR_ID_UNHANDLED_INT */
    }
};
<# TODO: Parse errors from input #>
// Mapping of thread-level errors information.
static const pok_thread_error_map_t partition_thread_error_info_{{loop.index0}} = {
    .map = {
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
        {POK_ERROR_KIND_ILLEGAL_REQUEST, "Illegal Request"},        /* POK_ERROR_ID_ILLEGAL_REQUEST */
        {POK_ERROR_KIND_APPLICATION_ERROR, "Application Error"},    /* POK_ERROR_ID_APPLICATION_ERROR */
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_PARTLOAD_ERROR */
        {POK_ERROR_KIND_NUMERIC_ERROR, "Numeric Error"},            /* POK_ERROR_ID_NUMERIC_ERROR */
        {POK_ERROR_KIND_MEMORY_VIOLATION, "Memory Violation"},      /* POK_ERROR_ID_MEMORY_VIOLATION */
        {POK_ERROR_KIND_DEADLINE_MISSED, "Deadline Missed"},        /* POK_ERROR_ID_DEADLINE_MISSED */
        {POK_ERROR_KIND_HARDWARE_FAULT, "Hardware Fault"},          /* POK_ERROR_ID_HARDWARE_FAULT */
        {POK_ERROR_KIND_POWER_FAIL, "Power Fail"},                  /* POK_ERROR_ID_POWER_FAIL */
        {POK_ERROR_KIND_STACK_OVERFLOW, "Stack Overflow"},          /* POK_ERROR_ID_STACK_OVERFLOW */
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_PROCINIT_ERROR */
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_NOMEMORY_PROCDATA */
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_ASSERT */
        {POK_ERROR_KIND_PARTITION_CONFIGURATION, "Config Error"},   /* POK_ERROR_ID_CONFIG_ERROR */
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_CHECK_POOL */
        {POK_ERROR_KIND_INVALID, NULL},                             /* POK_ERROR_ID_UNHANDLED_INT */
    }
};

<# TODO: Parse errors from input #>
/* 
 * Pointer to partition HM table.
 * 
 * Actions which are not set in `xml` are IDLE.
 */
static const pok_error_hm_partition_t partition_hm_table_{{loop.index0}} = {
    .actions = {
        {POK_ERROR_ACTION_IDLE, }, /* POK_SYSTEM_STATE_INIT_PARTOS */
        {POK_ERROR_ACTION_IDLE, }, /* POK_SYSTEM_STATE_INIT_PARTUSER */
        {POK_ERROR_ACTION_IDLE, }, /* POK_SYSTEM_STATE_INTERRUPT_HANDLER */
        {POK_ERROR_ACTION_IDLE, }, /* POK_SYSTEM_STATE_OS_MOD */
        { /* POK_SYSTEM_STATE_OS_PART */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_ILLEGAL_REQUEST */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_APPLICATION_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_PARTLOAD_ERROR */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_NUMERIC_ERROR */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_MEMORY_VIOLATION */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_DEADLINE_MISSED */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_HARDWARE_FAULT */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_POWER_FAIL */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_STACK_OVERFLOW */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_PROCINIT_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_NOMEMORY_PROCDATA */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_ASSERT */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_CONFIG_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_CHECK_POOL */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_UNHANDLED_INT */
        }, 
        { /* POK_SYSTEM_STATE_ERROR_HANDLER */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_ILLEGAL_REQUEST */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_APPLICATION_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_PARTLOAD_ERROR */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_NUMERIC_ERROR */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_MEMORY_VIOLATION */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_DEADLINE_MISSED */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_HARDWARE_FAULT */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_POWER_FAIL */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_STACK_OVERFLOW */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_PROCINIT_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_NOMEMORY_PROCDATA */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_ASSERT */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_CONFIG_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_CHECK_POOL */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_UNHANDLED_INT */
        }, 
        { /* POK_SYSTEM_STATE_USER */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_MODPOSTPROCEVENT_ELIST */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_ILLEGAL_REQUEST */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_APPLICATION_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_PARTLOAD_ERROR */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_NUMERIC_ERROR */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_MEMORY_VIOLATION */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_DEADLINE_MISSED */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_HARDWARE_FAULT */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_POWER_FAIL */
            POK_ERROR_ACTION_COLD_START,    /* POK_ERROR_ID_STACK_OVERFLOW */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_PROCINIT_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_NOMEMORY_PROCDATA */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_ASSERT */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_CONFIG_ERROR */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_CHECK_POOL */
            POK_ERROR_ACTION_IDLE,          /* POK_ERROR_ID_UNHANDLED_INT */
        }, 
    }
};

// Threads array
static pok_thread_t partition_threads_{{loop.index0}}[{{part.num_threads}} + 1 /*main thread*/ + 1 /* error thread */];

// Queuing ports
static pok_port_queuing_t partition_ports_queuing_{{loop.index0}}[{{part.ports_queueing | length}}] = {
<$for port_queueing in part.ports_queueing$>
    {
        .name = "{{port_queueing.name}}",
        .channel = &pok_channels_queuing[{{port_queueing.channel_id}}],
        .direction = <$if port_queueing.is_src()$>POK_PORT_DIRECTION_OUT<$else$>POK_PORT_DIRECTION_IN<$endif$>,
        .partition = &pok_partitions_arinc[{{loop.index0}}],
    },
<$endfor$>
};

// Sampling ports
static pok_port_sampling_t partition_ports_sampling_{{loop.index0}}[{{part.ports_sampling | length}}] = {
<$for port_sampling in part.ports_sampling$>
    {
        .name = "{{port_sampling.name}}",
        .channel = &pok_channels_sampling[{{port_sampling.channel_id}}],
        .direction = <$if port_sampling.is_src()$>POK_PORT_DIRECTION_OUT<$else$>POK_PORT_DIRECTION_IN<$endif$>,
    },
<$endfor$>
};

// Buffers array
static pok_buffer_t partition_buffers_{{loop.index0}}[{{part.num_arinc653_buffers}}];

// Blackboards array
static pok_blackboard_t partition_blackboards_{{loop.index0}}[{{part.num_arinc653_blackboards}}];

// Semaphores array
static pok_semaphore_t partition_semaphores_{{loop.index0}}[{{part.num_arinc653_semaphores}}];

// Events array
static pok_event_t partition_events_{{loop.index0}}[{{part.num_arinc653_events}}];
<$endfor$><#partitions loop#>

/*************** Setup partitions array *******************************/
pok_partition_arinc_t pok_partitions_arinc[{{conf.partitions | length}}] = {
<$for part in conf.partitions$>
    {
        .base_part = {
            .name = "{{part.name}}",
            
            .period = {{conf.major_frame}}, <#TODO: Where it is stored in conf?#>
            
            .space_id = {{loop.index0}},
            
            .multi_partition_hm_selector = &hm_multi_partition_selector_default,
            .multi_partition_hm_table = &hm_multi_partition_table_default,
        },
        
        .size = {{part.size}},
        .nthreads = {{part.num_threads}} + 1 /*main thread*/ + 1 /* error thread */,
        .threads = partition_threads_{{loop.index0}},
        
        .main_user_stack_size = 8192, <# TODO: This should be set in config somehow. #>

        .ports_queuing = partition_ports_queuing_{{loop.index0}},
        .nports_queuing = {{part.ports_queueing | length}},

        .ports_sampling = partition_ports_sampling_{{loop.index0}},
        .nports_sampling = {{part.ports_sampling | length}}, <#TODO: ports#>


        .intra_memory_size = {{part.buffer_data_size}} + {{part.blackboard_data_size}}, // Memory for intra-partition communication

        .buffers = partition_buffers_{{loop.index0}},
        .nbuffers = {{part.num_arinc653_buffers}},

        .blackboards = partition_blackboards_{{loop.index0}},
        .nblackboards = {{part.num_arinc653_blackboards}},
        
        .semaphores = partition_semaphores_{{loop.index0}},
        .nsemaphores = {{part.num_arinc653_semaphores}},

        .events = partition_events_{{loop.index0}},
        .nevents = {{part.num_arinc653_events}},

        .partition_hm_selector = &partition_hm_selector_{{loop.index0}},
    
        .thread_error_info = &partition_thread_error_info_{{loop.index0}},

        .partition_hm_table = &partition_hm_table_{{loop.index0}},

		.partition_id = {{loop.index0}},
    },
<$endfor$><#partitions loop#>
};

const uint8_t pok_partitions_arinc_n = {{conf.partitions | length}};

/**************************** Monitor *********************************/
pok_partition_t partition_monitor =
{
    .name = "Monitor",
    
    .period = {{conf.major_frame}}, <#TODO: Where it is stored in conf?#>
    
    .space_id = 0xff,
    
    .multi_partition_hm_selector = &hm_multi_partition_selector_default,
    .multi_partition_hm_table = &hm_multi_partition_table_default,
};

/******************************* GDB **********************************/
pok_partition_t partition_gdb =
{
    .name = "GDB",

    .period = {{conf.major_frame}}, <#TODO: Where it is stored in conf?#>

    .space_id = 0xff,

    .multi_partition_hm_selector = &hm_multi_partition_selector_default,
    .multi_partition_hm_table = &hm_multi_partition_table_default,
};


/************************* Setup time slots ***************************/
const pok_sched_slot_t pok_module_sched[{{conf.slots | length}}] = {
<$for slot in conf.slots$>
    {
        .duration = {{slot.duration}},
        .offset = 0, <#TODO: precalculate somehow#>
        <$-if slot.get_kind_constant() == 'POK_SLOT_PARTITION' $>

        .partition = &pok_partitions_arinc[{{slot.partition.part_index}}].base_part,
        .periodic_processing_start = <$if slot.periodic_processing_start$>TRUE<$else$>FALSE<$endif$>,
        <$elif slot.get_kind_constant() == 'POK_SLOT_MONITOR' $>

        .partition = &partition_monitor,
        .periodic_processing_start = FALSE,
        <$elif slot.get_kind_constant() == 'POK_SLOT_GDB' $>

        .partition = &partition_gdb,
        .periodic_processing_start = FALSE,
        <$endif-$>
        .id = {{loop.index0}}
    },
<$endfor$>
};

const uint8_t pok_module_sched_n = {{conf.slots | length}};

const pok_time_t pok_config_scheduling_major_frame = {{conf.major_frame}};

/************************ Setup address spaces ************************/
struct pok_space spaces[{{conf.partitions | length}}]; // As many as partitions
