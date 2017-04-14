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
#include "AFDX_QUEUE_ENQUEUER_gen.h"



    static ret_t __wrapper_afdx_enqueuer_implementation(self_t *arg0, char * arg1, size_t arg2, size_t arg3, size_t arg4)
    {
        return afdx_enqueuer_implementation((AFDX_QUEUE_ENQUEUER*) arg0, arg1, arg2, arg3, arg4);
    }

    static ret_t __wrapper_afdx_enqueuer_flush(self_t *arg0)
    {
        return afdx_enqueuer_flush((AFDX_QUEUE_ENQUEUER*) arg0);
    }



      ret_t _AFDX_QUEUE_ENQUEUER_call_portNetA_send_impl(AFDX_QUEUE_ENQUEUER *self, char * arg1, size_t arg2, size_t arg3, size_t arg4)
      {
         if (self->out.portNetA.ops == NULL) {
             printf("WRONG CONFIG: out port portNetA of component AFDX_QUEUE_ENQUEUER was not initialized\n");
             //fatal_error?
         }
         return self->out.portNetA.ops->send(self->out.portNetA.owner, arg1, arg2, arg3, arg4);
      }
      ret_t AFDX_QUEUE_ENQUEUER_call_portNetA_send(AFDX_QUEUE_ENQUEUER *self, char * arg1, size_t arg2, size_t arg3, size_t arg4)
      __attribute__ ((weak, alias ("_AFDX_QUEUE_ENQUEUER_call_portNetA_send_impl")));
      ret_t _AFDX_QUEUE_ENQUEUER_call_portNetA_flush_impl(AFDX_QUEUE_ENQUEUER *self)
      {
         if (self->out.portNetA.ops == NULL) {
             printf("WRONG CONFIG: out port portNetA of component AFDX_QUEUE_ENQUEUER was not initialized\n");
             //fatal_error?
         }
         return self->out.portNetA.ops->flush(self->out.portNetA.owner);
      }
      ret_t AFDX_QUEUE_ENQUEUER_call_portNetA_flush(AFDX_QUEUE_ENQUEUER *self)
      __attribute__ ((weak, alias ("_AFDX_QUEUE_ENQUEUER_call_portNetA_flush_impl")));
      ret_t _AFDX_QUEUE_ENQUEUER_call_portNetB_send_impl(AFDX_QUEUE_ENQUEUER *self, char * arg1, size_t arg2, size_t arg3, size_t arg4)
      {
         if (self->out.portNetB.ops == NULL) {
             printf("WRONG CONFIG: out port portNetB of component AFDX_QUEUE_ENQUEUER was not initialized\n");
             //fatal_error?
         }
         return self->out.portNetB.ops->send(self->out.portNetB.owner, arg1, arg2, arg3, arg4);
      }
      ret_t AFDX_QUEUE_ENQUEUER_call_portNetB_send(AFDX_QUEUE_ENQUEUER *self, char * arg1, size_t arg2, size_t arg3, size_t arg4)
      __attribute__ ((weak, alias ("_AFDX_QUEUE_ENQUEUER_call_portNetB_send_impl")));
      ret_t _AFDX_QUEUE_ENQUEUER_call_portNetB_flush_impl(AFDX_QUEUE_ENQUEUER *self)
      {
         if (self->out.portNetB.ops == NULL) {
             printf("WRONG CONFIG: out port portNetB of component AFDX_QUEUE_ENQUEUER was not initialized\n");
             //fatal_error?
         }
         return self->out.portNetB.ops->flush(self->out.portNetB.owner);
      }
      ret_t AFDX_QUEUE_ENQUEUER_call_portNetB_flush(AFDX_QUEUE_ENQUEUER *self)
      __attribute__ ((weak, alias ("_AFDX_QUEUE_ENQUEUER_call_portNetB_flush_impl")));


void __AFDX_QUEUE_ENQUEUER_init__(AFDX_QUEUE_ENQUEUER *self)
{
            self->in.portB.ops.send = __wrapper_afdx_enqueuer_implementation;
            self->in.portB.ops.flush = __wrapper_afdx_enqueuer_flush;

        afdx_queue_init(self);
}

void __AFDX_QUEUE_ENQUEUER_activity__(AFDX_QUEUE_ENQUEUER *self)
{
        (void) self; //suppress warning
}
