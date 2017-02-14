#include "DTSEC_NET_DEV_gen.h"


int fm_eth_send(struct fm_eth *fm_eth, void *buf, int len);
void dtsec_init(DTSEC_NET_DEV *self);
int fm_eth_recv(DTSEC_NET_DEV *self);

ret_t dtsec_send_frame(DTSEC_NET_DEV *self, char *buffer, size_t size, size_t max_back_step)
{
#ifdef __PPC__
    printf("DTSEC %s\n", __func__);

    fm_eth_send(self->state.dev_state.current_fm, buffer, size);
#endif
    return 0;
}

ret_t dtsec_flush_send(DTSEC_NET_DEV *self)
{
    //Empty
    return EOK;
}


void dtsec_component_init(DTSEC_NET_DEV *self)
{
    printf("DTSEC initializing ...\n");
#ifdef __PPC__
    dtsec_init(self);
    printf("DTSEC initialized\n");
#else
    printf("ERROR: DTSEC doesn't support archectures other than PPC\n");
#endif
}

void dtsec_receive_activity(DTSEC_NET_DEV *self)
{
#ifdef __PPC__
    fm_eth_recv(self);
#endif
}
