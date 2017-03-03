/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/mac/config.yaml).
 */
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <lib/common.h>
#include "MAC_RECEIVER_gen.h"



    static ret_t __wrapper_mac_receive(self_t *arg0, const char * arg1, size_t arg2)
    {
        return mac_receive((MAC_RECEIVER*) arg0, arg1, arg2);
    }



      ret_t MAC_RECEIVER_call_port_UDP_handle(MAC_RECEIVER *self, const char * arg1, size_t arg2)
      {
         if (self->out.port_UDP.ops == NULL) {
             printf("WRONG CONFIG: out port port_UDP of component MAC_RECEIVER was not initialized\n");
             //fatal_error?
         }
         return self->out.port_UDP.ops->handle(self->out.port_UDP.owner, arg1, arg2);
      }
      ret_t MAC_RECEIVER_call_port_ARP_handle(MAC_RECEIVER *self, const char * arg1, size_t arg2)
      {
         if (self->out.port_ARP.ops == NULL) {
             printf("WRONG CONFIG: out port port_ARP of component MAC_RECEIVER was not initialized\n");
             //fatal_error?
         }
         return self->out.port_ARP.ops->handle(self->out.port_ARP.owner, arg1, arg2);
      }


void __MAC_RECEIVER_init__(MAC_RECEIVER *self)
{
            self->in.portA.ops.handle = __wrapper_mac_receive;

}

void __MAC_RECEIVER_activity__(MAC_RECEIVER *self)
{
        (void) self; //suppress warning
}
