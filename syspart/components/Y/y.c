#include "Y_gen.h"
#include <lib/common.h>

void y_tick(Y *self)
{
    printf("Y<y=%d>.tick()\n", self->state.y);

    CALL_PORT_FUNCTION(self, portB, send, NULL, 101);

    CALL_PORT_FUNCTION(self, portB, flush);

}


void y_init(Y *self)
{
    printf("Y<y=%d> init()\n", self->state.y);
}
