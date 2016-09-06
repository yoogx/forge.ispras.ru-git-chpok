#include <lib/common.h>
#include "x_gen.h"


    static void __wrapper_x_send(self_t *arg0
        ,void * arg1
        ,size_t arg2
    )
    {
        return x_send((X*) arg0, arg1, arg2);
    }

    static void __wrapper_x_flush(self_t *arg0
    )
    {
        return x_flush((X*) arg0);
    }


void __X_init__(X *self)
{
            self->in.portC.ops.send = __wrapper_x_send;
            self->in.portC.ops.flush = __wrapper_x_flush;

        x_init(self);
}
