
#include "ARINC_RECEIVER_gen.h"

#define C_NAME "ARINC_RECEIVER: "
ret_t arinc_receive_message(ARINC_RECEIVER *self, char *payload, size_t payload_size)
{
    printf(C_NAME"%s got message\n", self->state.tmp_name);
    return EOK;
}

