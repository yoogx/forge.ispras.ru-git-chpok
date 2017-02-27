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
//~ #include "afdx.h"  // TO DELETE

#include "AFDX_TIME_ADDER_gen.h"

#define C_NAME "AFDX_TIME_ADDER: "

void afdx_time_adder_init(AFDX_TIME_ADDER *self)
{
    self->state.arrival_time = 0;
    printf(C_NAME" initialized successfully\n");
}

ret_t time_adder_send(AFDX_TIME_ADDER *self,
        const char * afdx_frame,
        const size_t frame_size
        )
{
    RETURN_CODE_TYPE return_code;

    GET_TIME(&self->state.arrival_time, &return_code);

    //~ uint8_t sn =  afdx_frame[frame_size - 1];
    //~ printf(C_NAME"get message: SN=%u\n", sn);
    //~ printf(C_NAME"get message: %s\n", afdx_frame + 42);

    if (return_code != NO_ERROR)
        printf(C_NAME"Error in GET_TIME\n");

    ret_t res = AFDX_TIME_ADDER_call_portA_handle(self,
                                            afdx_frame,
                                            frame_size,
                                            self->state.arrival_time);

    if (res != EOK)
        printf(C_NAME"Error in send\n");
    return EOK;
}
