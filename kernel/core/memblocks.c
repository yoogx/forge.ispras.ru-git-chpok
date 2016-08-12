#include <core/memblocks.h>
#include <libc.h>
pok_ret_t pok_memory_block_get_status(
        const char* name,
        jet_memory_block_status_t* __user status)
{
    printf("%s %s\n", __func__, name);

    status->addr = 0xed000000;
    status->size = 65536;
    status->mode = JET_MEMORY_BLOCK_READ_WRITE;
    return POK_ERRNO_OK;
}

