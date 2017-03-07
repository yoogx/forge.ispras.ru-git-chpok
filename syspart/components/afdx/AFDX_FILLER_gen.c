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
#include "AFDX_FILLER_gen.h"



    static ret_t __wrapper_afdx_filler_send(self_t *arg0, char * arg1, size_t arg2, size_t arg3, size_t arg4)
    {
        return afdx_filler_send((AFDX_FILLER*) arg0, arg1, arg2, arg3, arg4);
    }

    static ret_t __wrapper_afdx_filler_flush(self_t *arg0)
    {
        return afdx_filler_flush((AFDX_FILLER*) arg0);
    }



      ret_t AFDX_FILLER_call_portB_send(AFDX_FILLER *self, char * arg1, size_t arg2, size_t arg3, size_t arg4)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component AFDX_FILLER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->send(self->out.portB.owner, arg1, arg2, arg3, arg4);
      }
      ret_t AFDX_FILLER_call_portB_flush(AFDX_FILLER *self)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component AFDX_FILLER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->flush(self->out.portB.owner);
      }


void __AFDX_FILLER_init__(AFDX_FILLER *self)
{
            self->in.portA.ops.send = __wrapper_afdx_filler_send;
            self->in.portA.ops.flush = __wrapper_afdx_filler_flush;

        afdx_filler_init(self);
}

void __AFDX_FILLER_activity__(AFDX_FILLER *self)
{
        (void) self; //suppress warning
}
