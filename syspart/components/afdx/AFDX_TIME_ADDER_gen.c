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
#include "AFDX_TIME_ADDER_gen.h"



    static ret_t __wrapper_time_adder_send(self_t *arg0, const char * arg1, size_t arg2)
    {
        return time_adder_send((AFDX_TIME_ADDER*) arg0, arg1, arg2);
    }



      ret_t AFDX_TIME_ADDER_call_portA_handle(AFDX_TIME_ADDER *self, const char * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
      {
         if (self->out.portA.ops == NULL) {
             printf("WRONG CONFIG: out port portA of component AFDX_TIME_ADDER was not initialized\n");
             //fatal_error?
         }
         return self->out.portA.ops->handle(self->out.portA.owner, arg1, arg2, arg3);
      }


void __AFDX_TIME_ADDER_init__(AFDX_TIME_ADDER *self)
{
            self->in.portB.ops.handle = __wrapper_time_adder_send;

        afdx_time_adder_init(self);
}

void __AFDX_TIME_ADDER_activity__(AFDX_TIME_ADDER *self)
{
        (void) self; //suppress warning
}
