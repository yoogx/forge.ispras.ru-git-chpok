/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
#include <lib/common.h>
#include "UDP_RECEIVER_gen.h"



    static ret_t __wrapper_udp_receive(self_t *arg0, const char * arg1, size_t arg2)
    {
        return udp_receive((UDP_RECEIVER*) arg0, arg1, arg2);
    }



      ret_t UDP_RECEIVER_call_portB_udp_message_handler(UDP_RECEIVER *self, char * arg1, size_t arg2, uint32_t arg3, uint16_t arg4)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component UDP_RECEIVER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->udp_message_handler(self->out.portB.owner, arg1, arg2, arg3, arg4);
      }


void __UDP_RECEIVER_init__(UDP_RECEIVER *self)
{
            self->in.portA.ops.handle = __wrapper_udp_receive;

}

void __UDP_RECEIVER_activity__(UDP_RECEIVER *self)
{
}
