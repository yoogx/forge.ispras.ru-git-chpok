/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
 */
#include <lib/common.h>
#include "arinc_sender_gen.h"


    static void __wrapper_x_send(self_t *arg0, void * arg1, size_t arg2)
    {
        return x_send((ARINC_SENDER*) arg0, arg1, arg2);
    }

    static void __wrapper_x_flush(self_t *arg0)
    {
        return x_flush((ARINC_SENDER*) arg0);
    }


void __ARINC_SENDER_init__(ARINC_SENDER *self)
{
            self->in.portC.ops.send = __wrapper_x_send;
            self->in.portC.ops.flush = __wrapper_x_flush;

        arinc_sender_init(self);
}
