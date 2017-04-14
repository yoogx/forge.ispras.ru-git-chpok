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

#include <stdio.h>
#include <string.h>

#include "AFDX_STOP_gen.h"

#define C_NAME "AFDX_STOP: "

ret_t afdx_stop_func(AFDX_STOP *self,
        const uint8_t * afdx_frame,
        const size_t frame_size
        )
{
    int a;
    a = 1;
    self->state.st = a;
    (void) frame_size;
    (void ) afdx_frame;
    return EOK;
}
