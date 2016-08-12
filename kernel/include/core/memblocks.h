#ifndef __JET_KERNEL_CORE_MEMBLOCKS_H__
#define __JET_KERNEL_CORE_MEMBLOCKS_H__

#include <uapi/memblock_types.h>
#include <common.h>
#include <errno.h>

pok_ret_t pok_memory_block_get_status(
        const char* name,
        jet_memory_block_status_t* __user status);
#endif
