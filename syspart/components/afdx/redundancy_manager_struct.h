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
 
 #ifndef __REDUNDANCY_MANAGER_STRUCT_H__
#define __REDUNDANCY_MANAGER_STRUCT_H__

#include <arinc653/time.h>

#define MAX_SEQUENCE_NUMBER     255
#define SUBNETWORKS_COUNT       2
#define VIRTUAL_LINKS_COUNT     2

/* This structure (redundancy_management_data_t) describes the information needed
 * for receiving ES.
 *
 * arrival_time            -in this buffer we safe information about accepted message per SN
 *                      -time of arriving
 *
 * last_accepted_seq_numb    -sequence umber of the last accepted message from both subnetworks
 * last_accepted_msg_time    -time of the last accepted message from both subnetworks
 *
 */
 typedef struct
 {
    uint8_t                   last_accepted_seq_numb;
    SYSTEM_TIME_TYPE          last_accepted_msg_time;
 } redundancy_management_data_t;

/*
 * This structure (vl_data_t) describes parameters for each Virtual Link
 * static parameters:
 * vl_id    -Virtual Link identificator
 * skew_max                    -is given by configuration per VL, shows the difference
 *                             between time of subnetworks
 * BAG      -time to send the message (128ms)
 */
typedef struct
{
    const uint16_t          vl_id;
    const SYSTEM_TIME_TYPE  skew_max;
    const SYSTEM_TIME_TYPE  BAG;
    redundancy_management_data_t    redundancy_management_data;

} vl_data_t;

#endif
