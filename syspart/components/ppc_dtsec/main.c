#include "DTSEC_NET_DEV_gen.h"

ret_t dtsec_send_frame(DTSEC_NET_DEV *self, char *buffer, size_t size, size_t max_back_step)
{
    printf("DTSEC %s\n", __func__);
    return 0;
}

ret_t dtsec_flush_send(DTSEC_NET_DEV *self)
{
    printf("DTSEC %s\n", __func__);
    return 0;
}



void dtsec_init(DTSEC_NET_DEV *self)
{
    printf("DTSEC init\n");
}
