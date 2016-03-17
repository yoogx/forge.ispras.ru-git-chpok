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
pok_channel_queuing_t pok_channels_queuing[1] = {
    {
        .max_message_size = 64,
    
        .max_nb_message_receive = 5,
        .max_nb_message_send = 10,
    }
};

uint8_t pok_channels_queuing_n = 1;

/****************** Setup sampling channels ***************************/
pok_channel_sampling_t pok_channels_sampling[1] = {
    {
        .max_message_size = 64,
    }
};

uint8_t pok_channels_sampling_n = 1;

/****************** Setup partition1 (auxiliary) **********************/
// HM partition level selector.
static const pok_error_level_selector_t partition_hm_selector_0 = {
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

// Mapping of thread-level errors information.
static const pok_thread_error_map_t partition_thread_error_info_0 = {
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

/* 
 * Pointer to partition HM table.
 * 
 * Actions which are not set in `xml` are IDLE.
 */
static const pok_error_hm_partition_t partition_hm_table_0 = {
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
static pok_thread_t partition_threads_0[12];

// Queuing ports
static pok_port_queuing_t partition_ports_queuing_0[2] = {
    {
        .name = "QP1",
        .channel = &pok_channels_queuing[0],
        .direction = POK_PORT_DIRECTION_IN,
        .partition = &pok_partitions_arinc[0],
    },
    {
        .name = "QP2",
        .channel = &pok_channels_queuing[0],
        .direction = POK_PORT_DIRECTION_OUT,
        .partition = &pok_partitions_arinc[0],
    }
};

// Sampling ports
static pok_port_sampling_t partition_ports_sampling_0[2] = {
    {
        .name = "SP1",
        .channel = &pok_channels_sampling[0],
        .direction = POK_PORT_DIRECTION_IN,
    },
    {
        .name = "SP2",
        .channel = &pok_channels_sampling[0],
        .direction = POK_PORT_DIRECTION_OUT,
    }
};

// Buffers array
static pok_buffer_t partition_buffers_0[16];

// Blackboards array
static pok_blackboard_t partition_blackboards_0[16];

// Semaphores array
static pok_semaphore_t partition_semaphores_0[16];

// Events array
static pok_event_t partition_events_0[16];


/*************** Setup partitions array *******************************/
pok_partition_arinc_t pok_partitions_arinc[1] =
{
    {
        .base_part = {
            .name = "P1",
            
            .period = 15000,
            
            .space_id = 0,
            
            .multi_partition_hm_selector = &hm_multi_partition_selector_default,
            .multi_partition_hm_table = &hm_multi_partition_table_default,
        },
        
        .size = 307200,
        .nthreads = 12,
        .threads = partition_threads_0,
        
        .main_user_stack_size = 8192,

        .ports_queuing = partition_ports_queuing_0,
        .nports_queuing = 2,

        .ports_sampling = partition_ports_sampling_0,
        .nports_sampling = 2,


        .intra_memory_size = 4096 + 4096, // Memory for intra-partition communication

        .buffers = partition_buffers_0,
        .nbuffers = 16,

        .blackboards = partition_blackboards_0,
        .nblackboards = 16,
        
        .semaphores = partition_semaphores_0,
        .nsemaphores = 16,

        .events = partition_events_0,
        .nevents = 16,

        .partition_hm_selector = &partition_hm_selector_0,
    
        .thread_error_info = &partition_thread_error_info_0,

        .partition_hm_table = &partition_hm_table_0,

        .main_user_stack_size = 8096, // TODO: This should set in config somehow.

		.partition_id = 0,
    }
};

const uint8_t pok_partitions_arinc_n = 1;

/**************************** Monitor *********************************/
pok_partition_t partition_monitor =
{
    .name = "Monitor",
    
    .period = 15000,
    
    .space_id = 0xff,
    
    .multi_partition_hm_selector = &hm_multi_partition_selector_default,
    .multi_partition_hm_table = &hm_multi_partition_table_default,
};

/************************* Setup time slots ***************************/
const pok_sched_slot_t pok_module_sched[1] = {
    {
        .duration = 15000,
        .offset = 0,
        .partition = &pok_partitions_arinc[0].base_part,
        .periodic_processing_start = TRUE,
        .id = 0
    },
};

const uint8_t pok_module_sched_n = 1;

const pok_time_t pok_config_scheduling_major_frame = 15000;

/************************ Setup address spaces ************************/
struct pok_space spaces[1]; // As many as partitions
