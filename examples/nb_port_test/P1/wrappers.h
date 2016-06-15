#include <stdio.h>
#include <string.h>
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>

//#include <errno.h>
#include <types.h>
//#include "../../../kernel/include/core/time.h"
#include "../../../kernel/include/core/syscall.h"
#include "../../../kernel/include/core/thread.h"
#include "../../../kernel/include/uapi/errno.h"

pok_ret_t pok_syscall_thread_create_wrapper(void* param, int pos);
