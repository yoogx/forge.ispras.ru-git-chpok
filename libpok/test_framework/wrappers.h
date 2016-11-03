#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>

//#include <errno.h>
#include <types.h>
//#include "../../kernel/include/core/time.h"
//#include "../../kernel/include/core/syscall.h"
#include "../../kernel/include/uapi/errno.h"
//#include "../../kernel/include/uapi/syscall_types.h"
//#include "../../kernel/include/uapi/thread_types.h"


//#include "../../kernel/include/core/thread.h"                  // core/delayed_event.h: No such file or directory #include <core/delayed_event.h>
//#include "../../kernel/include/uapi/syscall_map_arinc.h"       // many errors

//#include "../../libpok/include/uapi/syscall_map_arinc.h"         // undefined reference to 'pok_syscall4'
#include "../../libpok/include/core/syscall.h"
//#include "../../libpok/include/uapi/syscall_types.h" // TODO: uncomment

#include "../../kernel/include/uapi/types.h"
#include "../../kernel/include/uapi/thread_types.h"
#include "../../kernel/include/uapi/partition_types.h"
#include "../../kernel/include/uapi/partition_arinc_types.h"
#include "../../kernel/include/uapi/port_types.h"
#include "../../kernel/include/uapi/buffer_types.h"
#include "../../kernel/include/uapi/blackboard_types.h"
#include "../../kernel/include/uapi/semaphore_types.h"
#include "../../kernel/include/uapi/event_types.h"
#include "../../kernel/include/uapi/error_arinc_types.h"

// TODO: debug and find from which file all of the syscalls signatures come

// TODO: for messages, size should match message (or not, depends on what error we try to catch)

// TODO: split 'thread' section on 'process', 'partition' and 'time management'

/*
 * For all 1-parametrized wrappers
 * 
 * pos - position of tested param
 * 
 * other _n params are autocompleted by a wrapper
 * 
 * usefull files to write this type of wrappers:
 * - libpok/arinc653/<name>.c // for example, buffer.c for buffer syscalls
 * - kernel/include/uapi/<name>_types.h // for example, buffer_types.h for buffer syscalls
 * - kernel/include/uapi/types.h
 * - kernel/include/uapi/syscall_map_arinc.h
 * - libpok/include/uapi/syscall_map_arinc.h
 * - libpok/include/arinc653/types.h
 * - kernel/core/intra_arinc.c
 **/
 

/////////////////////////// THREAD (PROCESS) ///////////////////////////

pok_ret_t pok_thread_create_wrapper                     (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_thread_sleep_wrapper                      (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_thread_sleep_until_wrapper                (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_thread_suspend_wrapper                    (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_sched_get_current_wrapper                 (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_thread_get_status_wrapper                 (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_thread_delayed_start_wrapper              (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_sched_replenish_wrapper                   (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_current_partition_get_status_wrapper      (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_current_partition_inc_lock_level_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_current_partition_dec_lock_level_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);


/////////////////////////// BUFFER /////////////////////////////////////

pok_ret_t pok_buffer_create_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_send_wrapper   (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_receive_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_get_id_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_status_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

/////////////////////////// BLACKBOARD /////////////////////////////////

pok_ret_t pok_blackboard_create_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_read_wrapper   (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_display_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_clear_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_id_wrapper     (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_status_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

/////////////////////////// SEMAPHORE //////////////////////////////////

pok_ret_t pok_semaphore_create_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_semaphore_wait_wrapper    (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_semaphore_id_wrapper      (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_semaphore_status_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);


/////////////////////////// EVENT //////////////////////////////////////

pok_ret_t pok_event_create_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_event_wait_wrapper    (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_event_id_wrapper      (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_event_status_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);


/////////////////////////// HEALTH MONITORING //////////////////////////

pok_ret_t pok_error_thread_create_wrapper           (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_error_raise_application_error_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_error_get_wrapper                     (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

// TODO: add other types of entities
typedef enum {
	BLACKBOARD      = 0,
	BUFFER          = 1,
    SEMAPHORE       = 2,
    EVENT           = 3,
    STARTED_PROCESS = 4
} ENTITY_TYPE;


/*
 *  TODO: add prototypes for all wrappers
 * 
 * buffers      +
 * blackboards  +
 * semaphores   +
 * events       +
 * 
*/


pok_ret_t buffer_write_wrapper(void* param, uint8_t pre_created_id);
pok_ret_t buffer_read_wrapper(uint8_t pre_created_id);

pok_ret_t blackboard_write_wrapper(void* param, uint8_t pre_created_id);
pok_ret_t blackboard_read_wrapper(uint8_t pre_created_id);
