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
#include "AFDX_STOP_gen.h"



    static ret_t __wrapper_afdx_stop_func(self_t *arg0, const uint8_t * arg1, size_t arg2)
    {
        return afdx_stop_func((AFDX_STOP*) arg0, arg1, arg2);
    }





void __AFDX_STOP_init__(AFDX_STOP *self)
{
            self->in.portA.ops.handle = __wrapper_afdx_stop_func;

}

void __AFDX_STOP_activity__(AFDX_STOP *self)
{
        (void) self; //suppress warning
}
