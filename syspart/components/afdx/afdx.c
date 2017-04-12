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
*
*=======================================================================
*
*             That file describes the frame structure
*
* The following file is a part of the AFDX project. Any modification should
* made according to the AFDX standard.
*
*
*/

#include "afdx.h"
#include <stdio.h>
/*
 * This function check size of afdx frame_size
 * it should be at least not less than 60 bytes
 * (42 bytes - header, 17 - payload, 1 - sequence number)
 * and less than the maximum packet length (1518)
 */
ret_t check_afdx_frame_size(size_t afdx_frame_size) {
    if (afdx_frame_size < sizeof(afdx_frame_t) + MIN_AFDX_PAYLOAD_SIZE + 1 ||
        afdx_frame_size > MAX_AFDX_FRAME_SIZE) {
        /*
         * Check that payload is suitable
         */
            printf("The payload is bad\n");
            return EINVAL;
    } else {
        return EOK;
    }
}
