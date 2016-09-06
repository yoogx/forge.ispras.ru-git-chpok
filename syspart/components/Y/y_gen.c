#include <lib/common.h>
#include "y_gen.h"


    static void __wrapper_y_tick(self_t *arg0
    )
    {
        return y_tick((Y*) arg0);
    }


void __Y_init__(Y *self)
{
            self->in.portA.ops.tick = __wrapper_y_tick;

}
