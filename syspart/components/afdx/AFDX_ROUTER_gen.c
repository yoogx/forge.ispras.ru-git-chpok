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
#include "AFDX_ROUTER_gen.h"



    static ret_t __wrapper_afdx_router_receive_packet(self_t *arg0, const char * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
    {
        return afdx_router_receive_packet((AFDX_ROUTER*) arg0, arg1, arg2, arg3);
    }



      ret_t AFDX_ROUTER_call_portArray_handle_by_index(int idx, AFDX_ROUTER *self, const char * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
      {
         if (self->out.portArray[idx].ops == NULL) {
             printf("WRONG CONFIG: out port portArray of component AFDX_ROUTER was not initialized\n");
             //fatal_error?
         }
         return self->out.portArray[idx].ops->handle(self->out.portArray[idx].owner, arg1, arg2, arg3);
      }


void __AFDX_ROUTER_init__(AFDX_ROUTER *self)
{
            self->in.portA.ops.handle = __wrapper_afdx_router_receive_packet;

}

void __AFDX_ROUTER_activity__(AFDX_ROUTER *self)
{
}
