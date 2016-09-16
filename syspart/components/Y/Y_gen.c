/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/Y/config.yaml).
 */
#include <lib/common.h>
#include "Y_gen.h"



    static void __wrapper_y_tick(self_t *arg0)
    {
        return y_tick((Y*) arg0);
    }



      void Y_call_portB_send(Y *self, void * arg1, size_t arg2)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component Y was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->send(self->out.portB.owner, arg1, arg2);
      }
      void Y_call_portB_flush(Y *self)
      {
         if (self->out.portB.ops == NULL) {
             printf("WRONG CONFIG: out port portB of component Y was not initialized\n");
             //fatal_error?
         }
         return self->out.portB.ops->flush(self->out.portB.owner);
      }


void __Y_init__(Y *self)
{
            self->in.portA.ops.tick = __wrapper_y_tick;

}

void __Y_activity__(Y *self)
{
}
