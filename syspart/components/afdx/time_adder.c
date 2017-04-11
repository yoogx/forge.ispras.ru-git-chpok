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
#include <arinc653/time.h>

#include "AFDX_TIME_ADDER_gen.h"

#define C_NAME "AFDX_TIME_ADDER: "

ret_t time_adder_send(AFDX_TIME_ADDER *self,
        const uint8_t * afdx_packet,
        const size_t afdx_packet_size
        )
{
    RETURN_CODE_TYPE return_code;
    SYSTEM_TIME_TYPE arrival_time;

    GET_TIME(&arrival_time, &return_code);

    if (return_code != NO_ERROR)
        printf(C_NAME"Error in GET_TIME\n");

    ret_t res = AFDX_TIME_ADDER_call_portA_handle(self,
                                            afdx_packet,
                                            afdx_packet_size,
                                            arrival_time);

    if (res != EOK)
        printf(C_NAME"Error in send\n");
    return res;
}
