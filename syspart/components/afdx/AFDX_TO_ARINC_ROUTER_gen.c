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
#include "AFDX_TO_ARINC_ROUTER_gen.h"



    static ret_t __wrapper_afdx_to_arinc_router_receive(self_t *arg0, const uint8_t * arg1, size_t arg2)
    {
        return afdx_to_arinc_router_receive((AFDX_TO_ARINC_ROUTER*) arg0, arg1, arg2);
    }



      ret_t AFDX_TO_ARINC_ROUTER_call_portArray_handle_by_index(int idx, AFDX_TO_ARINC_ROUTER *self, const uint8_t * arg1, size_t arg2)
      {
         if (self->out.portArray[idx].ops == NULL) {
             printf("WRONG CONFIG: out port portArray of component AFDX_TO_ARINC_ROUTER was not initialized\n");
             //fatal_error?
         }
         return self->out.portArray[idx].ops->handle(self->out.portArray[idx].owner, arg1, arg2);
      }


void __AFDX_TO_ARINC_ROUTER_init__(AFDX_TO_ARINC_ROUTER *self)
{
            self->in.portC.ops.handle = __wrapper_afdx_to_arinc_router_receive;

}

void __AFDX_TO_ARINC_ROUTER_activity__(AFDX_TO_ARINC_ROUTER *self)
{
        (void) self; //suppress warning
}
