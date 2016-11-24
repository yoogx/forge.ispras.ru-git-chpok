#include "DTSEC_NET_DEV_gen.h"


int fm_eth_send(struct fm_eth *fm_eth, void *buf, int len);
void dtsec_init(DTSEC_NET_DEV *self);

ret_t dtsec_send_frame(DTSEC_NET_DEV *self, char *buffer, size_t size, size_t max_back_step)
{
    printf("DTSEC %s\n", __func__);

    fm_eth_send(self->state.dev_state.current_fm, buffer, size);
    return 0;
}

ret_t dtsec_flush_send(DTSEC_NET_DEV *self)
{
    //Empty
    return EOK;
}


void dtsec_component_init(DTSEC_NET_DEV *self)
{
    dtsec_init(self);
    printf("DTSEC init\n");
}
