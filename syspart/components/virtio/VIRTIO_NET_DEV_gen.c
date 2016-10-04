/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/virtio/config.yaml).
 */
#include <lib/common.h>
#include "VIRTIO_NET_DEV_gen.h"



    static ret_t __wrapper_send_frame(self_t *arg0, char * arg1, size_t arg2, size_t arg3)
    {
        return send_frame((VIRTIO_NET_DEV*) arg0, arg1, arg2, arg3);
    }

    static ret_t __wrapper_flush_send(self_t *arg0)
    {
        return flush_send((VIRTIO_NET_DEV*) arg0);
    }



      ret_t VIRTIO_NET_DEV_call_portB_handle(VIRTIO_NET_DEV *self, const char * arg1, size_t arg2)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component VIRTIO_NET_DEV was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->handle(self->out.portB.owner, arg1, arg2);
      }


void __VIRTIO_NET_DEV_init__(VIRTIO_NET_DEV *self)
{
            self->in.portA.ops.send = __wrapper_send_frame;
            self->in.portA.ops.flush = __wrapper_flush_send;

        virtio_init(self);
}

void __VIRTIO_NET_DEV_activity__(VIRTIO_NET_DEV *self)
{
        virtio_receive_activity(self);
}
