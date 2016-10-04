/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/mac/config.yaml).
 */
#include <lib/common.h>
#include "MAC_RECEIVER_gen.h"



    static ret_t __wrapper_mac_receive(self_t *arg0, char * arg1, size_t arg2)
    {
        return mac_receive((MAC_RECEIVER*) arg0, arg1, arg2);
    }



      ret_t MAC_RECEIVER_call_port_UDP_send(MAC_RECEIVER *self, char * arg1, size_t arg2)
      {
         if (self->out.port_UDP.ops == NULL) {
             printf("WRONG CONFIG: out port port_UDP of component MAC_RECEIVER was not initialized\n");
             //fatal_error?
         }
         return self->out.port_UDP.ops->send(self->out.port_UDP.owner, arg1, arg2);
      }
      ret_t MAC_RECEIVER_call_port_ARP_send(MAC_RECEIVER *self, char * arg1, size_t arg2)
      {
         if (self->out.port_ARP.ops == NULL) {
             printf("WRONG CONFIG: out port port_ARP of component MAC_RECEIVER was not initialized\n");
             //fatal_error?
         }
         return self->out.port_ARP.ops->send(self->out.port_ARP.owner, arg1, arg2);
      }


void __MAC_RECEIVER_init__(MAC_RECEIVER *self)
{
            self->in.portA.ops.send = __wrapper_mac_receive;

}

void __MAC_RECEIVER_activity__(MAC_RECEIVER *self)
{
}
