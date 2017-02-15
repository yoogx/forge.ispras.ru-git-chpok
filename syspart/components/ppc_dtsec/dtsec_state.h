#ifndef _STATE_H_
#define _STATE_H_

#include "fm.h"

struct dev_state{
    struct fm_eth *current_fm;
    void *send_buffer;
    uint32_t send_buffer_size;
    uint8_t macaddr[6];
};

#endif
