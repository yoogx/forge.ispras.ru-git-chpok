/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
#include <lib/common.h>
#include "ROUTER_gen.h"



    static ret_t __wrapper_receive_packet(self_t *arg0, const char * arg1, size_t arg2, uint32_t arg3, uint16_t arg4)
    {
        return receive_packet((ROUTER*) arg0, arg1, arg2, arg3, arg4);
    }



      ret_t ROUTER_call_portArray_handle_by_index(int idx, ROUTER *self, const char * arg1, size_t arg2)
      {
         if (self->out.portArray[idx].ops == NULL) {
             printf("WRONG CONFIG: out port portArray of component ROUTER was not initialized\n");
             //fatal_error?
         }
         return self->out.portArray[idx].ops->handle(self->out.portArray[idx].owner, arg1, arg2);
      }


void __ROUTER_init__(ROUTER *self)
{
            self->in.portA.ops.udp_message_handle = __wrapper_receive_packet;

}

void __ROUTER_activity__(ROUTER *self)
{
}
