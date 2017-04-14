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
#include "MAC_SENDER_gen.h"



    static ret_t __wrapper_mac_send(self_t *arg0, char * arg1, size_t arg2, size_t arg3, uint8_t * arg4, enum ethertype arg5)
    {
        return mac_send((MAC_SENDER*) arg0, arg1, arg2, arg3, arg4, arg5);
    }

    static ret_t __wrapper_mac_flush(self_t *arg0)
    {
        return mac_flush((MAC_SENDER*) arg0);
    }



      ret_t MAC_SENDER_call_portB_send(MAC_SENDER *self, char * arg1, size_t arg2, size_t arg3)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component MAC_SENDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->send(self->out.portB.owner, arg1, arg2, arg3);
      }
      ret_t MAC_SENDER_call_portB_flush(MAC_SENDER *self)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component MAC_SENDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->flush(self->out.portB.owner);
      }


void __MAC_SENDER_init__(MAC_SENDER *self)
{
            self->in.portA.ops.mac_send = __wrapper_mac_send;
            self->in.portA.ops.flush = __wrapper_mac_flush;

}

void __MAC_SENDER_activity__(MAC_SENDER *self)
{
        (void) self; //suppress warning
}
