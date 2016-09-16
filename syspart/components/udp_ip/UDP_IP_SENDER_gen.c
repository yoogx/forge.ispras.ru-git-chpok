/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
#include <lib/common.h>
#include "UDP_IP_SENDER_gen.h"



    static ret_t __wrapper_udp_ip_send(self_t *arg0, char * arg1, size_t arg2, size_t arg3)
    {
        return udp_ip_send((UDP_IP_SENDER*) arg0, arg1, arg2, arg3);
    }

    static ret_t __wrapper_udp_ip_flush(self_t *arg0)
    {
        return udp_ip_flush((UDP_IP_SENDER*) arg0);
    }



      ret_t UDP_IP_SENDER_call_portB_mac_send(UDP_IP_SENDER *self, char * arg1, size_t arg2, size_t arg3, uint8_t * arg4, enum ethertype arg5)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component UDP_IP_SENDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->mac_send(self->out.portB.owner, arg1, arg2, arg3, arg4, arg5);
      }
      ret_t UDP_IP_SENDER_call_portB_flush(UDP_IP_SENDER *self)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component UDP_IP_SENDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->flush(self->out.portB.owner);
      }


void __UDP_IP_SENDER_init__(UDP_IP_SENDER *self)
{
            self->in.portA.ops.send = __wrapper_udp_ip_send;
            self->in.portA.ops.flush = __wrapper_udp_ip_flush;

}

void __UDP_IP_SENDER_activity__(UDP_IP_SENDER *self)
{
}
