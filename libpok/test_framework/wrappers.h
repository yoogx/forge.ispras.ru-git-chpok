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

/////////////////////////// BUFFERS ///////////////////////////

pok_ret_t pok_buffer_create_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_send_wrapper   (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_receive_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_get_id_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_buffer_status_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

/////////////////////////// BLACKBOARDS ///////////////////////

pok_ret_t pok_blackboard_create_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_read_wrapper   (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_display_wrapper(void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_clear_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_id_wrapper     (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_blackboard_status_wrapper (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);

/////////////////////////// SEMAPHORES ////////////////////////

pok_ret_t pok_semaphore_create_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_semaphore_wait_wrapper    (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_semaphore_id_wrapper      (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_semaphore_status_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);


/////////////////////////// EVENTS ////////////////////////////

pok_ret_t pok_event_create_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_event_wait_wrapper    (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_event_id_wrapper      (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);
pok_ret_t pok_event_status_wrapper  (void* param, int pos, uint8_t pre_created_id, const char* pre_created_name);



/*
 *  TODO: add prototypes for all wrappers
 * 
 * buffers      +
 * blackboards  +
 * semaphores   +
 * events       +
 * 
*/
