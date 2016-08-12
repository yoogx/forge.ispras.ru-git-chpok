#include <core/memblocks.h>
#include <libc.h>
#include <core/partition.h>
pok_ret_t pok_memory_block_get_status(
        const char* name,
        jet_memory_block_status_t* __user status)
{
    int current_pid = current_partition->space_id + 1;
    printf("%s %s\n", __func__, name);
    printf("partition space_id = %d\n", current_pid);

    status->addr = 0xed000000;
    status->size = 65536;
    status->mode = JET_MEMORY_BLOCK_READ_WRITE;
    return POK_ERRNO_OK;
}

