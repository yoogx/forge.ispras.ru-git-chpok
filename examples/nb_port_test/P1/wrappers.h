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
#include "../../../kernel/include/uapi/thread_types.h"


//#include "../../../kernel/include/core/thread.h"                  // core/delayed_event.h: No such file or directory #include <core/delayed_event.h>
//#include "../../../kernel/include/uapi/syscall_map_arinc.h"       // many errors

//#include "../../../libpok/include/uapi/syscall_map_arinc.h"         // undefined reference to 'pok_syscall4'
#include "../../../libpok/include/core/syscall.h"
#include "../../../libpok/include/uapi/syscall_types.h"

pok_ret_t pok_syscall_thread_create_wrapper(void* param, int pos);
