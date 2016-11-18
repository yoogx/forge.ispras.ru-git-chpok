#include <core/memblocks.h>
#include <libc.h>
#include <core/partition.h>
#include <core/memblocks_config.h>
pok_ret_t pok_memory_block_get_status(
        const char* name,
        jet_memory_block_status_t* __user status)
{
    int i;
    int current_pid = current_partition->space_id;

    for (i = 0; i < jet_memory_blocks_n; i++) {
        if (strncmp(jet_memory_blocks[i].name, name, MAX_NAME_LENGTH) == 0)
            break;
    }
    if (i == jet_memory_blocks_n)
        return POK_ERRNO_EINVAL;

    if (jet_memory_blocks[i].pid_to_rights[current_pid] == 0)
        return POK_ERRNO_EPERM;

    status->addr = jet_memory_blocks[i].virt_addr;
    status->size = jet_memory_blocks[i].size;
    if (jet_memory_blocks[i].pid_to_rights[current_pid] == MB_CONFIG_READ_WRITE)
        status->mode = JET_MEMORY_BLOCK_READ_WRITE;
    else
        status->mode = JET_MEMORY_BLOCK_READ;
    return POK_ERRNO_OK;
}

