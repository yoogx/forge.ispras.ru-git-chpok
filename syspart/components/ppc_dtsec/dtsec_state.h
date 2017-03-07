#ifndef _STATE_H_
#define _STATE_H_

#include "fm.h"

#include <memblocks.h>
#include <smalloc.h>

struct dev_state{
    struct fm_eth *current_fm;
    void *send_buffer;
    uint32_t send_buffer_size;
    uint8_t macaddr[6];
    jet_memory_block_status_t heap_mb;
    struct jet_sallocator allocator;
};

#endif
