/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/X/config.yaml).
 */
#include <lib/common.h>
#include "x1_gen.h"


    static void __wrapper_x_send(self_t *arg0
        ,void * arg1
        ,size_t arg2
    )
    {
        return x_send((X1*) arg0, arg1, arg2);
    }

    static void __wrapper_x_flush(self_t *arg0
    )
    {
        return x_flush((X1*) arg0);
    }


void __X1_init__(X1 *self)
{
            self->in.portC.ops.send = __wrapper_x_send;
            self->in.portC.ops.flush = __wrapper_x_flush;

        x_init(self);
}
