#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>

//#include <errno.h>
#include <types.h>
//#include "../../../kernel/include/core/time.h"
//#include "../../../kernel/include/core/syscall.h"
#include "../../../kernel/include/uapi/errno.h"
//#include "../../../kernel/include/uapi/syscall_types.h"
//#include "../../../kernel/include/uapi/thread_types.h"


//#include "../../../kernel/include/core/thread.h"                  // core/delayed_event.h: No such file or directory #include <core/delayed_event.h>
//#include "../../../kernel/include/uapi/syscall_map_arinc.h"       // many errors

//#include "../../../libpok/include/uapi/syscall_map_arinc.h"         // undefined reference to 'pok_syscall4'
#include "../../../libpok/include/core/syscall.h"
//#include "../../../libpok/include/uapi/syscall_types.h" // TODO: uncomment

#include "../../../kernel/include/uapi/types.h"
#include "../../../kernel/include/uapi/thread_types.h"
#include "../../../kernel/include/uapi/partition_types.h"
#include "../../../kernel/include/uapi/partition_arinc_types.h"
#include "../../../kernel/include/uapi/port_types.h"
#include "../../../kernel/include/uapi/buffer_types.h"
#include "../../../kernel/include/uapi/blackboard_types.h"
#include "../../../kernel/include/uapi/semaphore_types.h"
#include "../../../kernel/include/uapi/event_types.h"
#include "../../../kernel/include/uapi/error_arinc_types.h"


/////////////////////////// BUFFERS ///////////////////////////

pok_ret_t pok_buffer_create_wrapper(void* param, int pos);
pok_ret_t pok_buffer_send_wrapper(void* param, int pos);
pok_ret_t pok_buffer_receive_wrapper(void* param, int pos);
pok_ret_t pok_buffer_get_id_wrapper(void* param, int pos);
pok_ret_t pok_buffer_status_wrapper(void* param, int pos);
