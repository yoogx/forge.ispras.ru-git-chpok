#include "X_gen.h"
#include <lib/common.h>

void x_send(X *self, void *buf, size_t len)
{
    /*...*/
    printf("X<x=%d>.send(%p, %zx)\n", self->state.x, buf, len);

}

void x_flush(X *self)
{
    /*...*/
    printf("X<x=%d>.flush()\n", self->state.x);
}

void x_init(X *self)
{
    printf("X<x=%d> init()\n", self->state.x);
}
