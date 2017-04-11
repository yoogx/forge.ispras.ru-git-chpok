/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/afdx/config.yaml).
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
#include "REDUNDANCY_MANAGER_gen.h"



    static ret_t __wrapper_redundancy_manager_receive_packet_net_a(self_t *arg0, const uint8_t * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
    {
        return redundancy_manager_receive_packet_net_a((REDUNDANCY_MANAGER*) arg0, arg1, arg2, arg3);
    }

    static ret_t __wrapper_redundancy_manager_receive_packet_net_b(self_t *arg0, const uint8_t * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
    {
        return redundancy_manager_receive_packet_net_b((REDUNDANCY_MANAGER*) arg0, arg1, arg2, arg3);
    }



      ret_t REDUNDANCY_MANAGER_call_portC_handle(REDUNDANCY_MANAGER *self, const uint8_t * arg1, size_t arg2)
      {
         if (self->out.portC.ops == NULL) {
             printf("WRONG CONFIG: out port portC of component REDUNDANCY_MANAGER was not initialized\n");
             //fatal_error?
         }
         return self->out.portC.ops->handle(self->out.portC.owner, arg1, arg2);
      }


void __REDUNDANCY_MANAGER_init__(REDUNDANCY_MANAGER *self)
{
            self->in.portA.ops.handle = __wrapper_redundancy_manager_receive_packet_net_a;
            self->in.portB.ops.handle = __wrapper_redundancy_manager_receive_packet_net_b;

        redundancy_manager_init(self);
}

void __REDUNDANCY_MANAGER_activity__(REDUNDANCY_MANAGER *self)
{
        (void) self; //suppress warning
}
