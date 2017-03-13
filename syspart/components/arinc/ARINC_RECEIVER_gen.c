/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arinc/config.yaml).
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
#include "ARINC_RECEIVER_gen.h"



    static ret_t __wrapper_arinc_receive_message(self_t *arg0, const char * arg1, size_t arg2)
    {
        return arinc_receive_message((ARINC_RECEIVER*) arg0, arg1, arg2);
    }





void __ARINC_RECEIVER_init__(ARINC_RECEIVER *self)
{
            self->in.portA.ops.handle = __wrapper_arinc_receive_message;

        arinc_receiver_init(self);
}

void __ARINC_RECEIVER_activity__(ARINC_RECEIVER *self)
{
        (void) self; //suppress warning
}
