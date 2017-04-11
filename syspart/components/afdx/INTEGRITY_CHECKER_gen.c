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
#include "INTEGRITY_CHECKER_gen.h"



    static ret_t __wrapper_integrity_checker_receive_packet(self_t *arg0, const uint8_t * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
    {
        return integrity_checker_receive_packet((INTEGRITY_CHECKER*) arg0, arg1, arg2, arg3);
    }



      ret_t INTEGRITY_CHECKER_call_portB_handle(INTEGRITY_CHECKER *self, const uint8_t * arg1, size_t arg2, SYSTEM_TIME_TYPE arg3)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component INTEGRITY_CHECKER was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->handle(self->out.portB.owner, arg1, arg2, arg3);
      }


void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER *self)
{
            self->in.portA.ops.handle = __wrapper_integrity_checker_receive_packet;

        integrity_checker_init(self);
}

void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER *self)
{
        (void) self; //suppress warning
}
