#ifndef _STATE_H_
#define _STATE_H_

#include "fm.h"

struct send_state{
    unsigned send_last_seen; //Truly points to first nonseen element
    //struct send_callback send_callbacks[TX_BD_RING_SIZE];
};

struct init_buffers {
    char tx_buffer_pseudo_malloc[sizeof(struct fm_port_bd) * TX_BD_RING_SIZE];
    char rx_ring_pseudo_malloc  [sizeof(struct fm_port_bd) * RX_BD_RING_SIZE];
    char rx_pool_pseudo_malloc  [MAX_RXBUF_LEN * RX_BD_RING_SIZE];
};

struct dev_state{
    struct fm_eth *current_fm;
    struct send_state send_state;
    struct init_buffers init_buffers;
    uint8_t macaddr[6];
};

#endif
