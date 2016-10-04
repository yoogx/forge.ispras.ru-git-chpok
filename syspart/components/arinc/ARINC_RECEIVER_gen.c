/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
 */
#include <lib/common.h>
#include "ARINC_RECEIVER_gen.h"



    static ret_t __wrapper_arinc_receive_message(self_t *arg0, char * arg1, size_t arg2)
    {
        return arinc_receive_message((ARINC_RECEIVER*) arg0, arg1, arg2);
    }





void __ARINC_RECEIVER_init__(ARINC_RECEIVER *self)
{
            self->in.portA.ops.send = __wrapper_arinc_receive_message;

        arinc_receiver_init(self);
}

void __ARINC_RECEIVER_activity__(ARINC_RECEIVER *self)
{
}
