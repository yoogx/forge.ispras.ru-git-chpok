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
*                   AFDX End-System configuration
*
* The following file is a part of the AFDX project. Any modification
* should made according to the AFDX standard.
*
*
* Created by ....
*/

#ifndef __AFDX_ES_CONFIG_H_
#define __AFDX_ES_CONFIG_H_

#include <afdx/AFDX_ES.h>
#include <stdlib.h>
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

#define SECOND                     1000000000LL

#define DOMAIN_ID        0xA        //The 0000 and 1111 are forbidden values.
                                    //Will be specified for each hosting equipment

#define SIDE_ID            0x4      //The 000 and 111 are forbidden values.
                                    //Will be specified for each hosting equipment

#define LOCATION_ID        0x5      //The 00000 and 11111 are forbidden values.
                                    //Will be specified for each hosting equipment


/********************************************/
/*
 * This structure  describes ports types
 * The packet may be sampling or queuing
 */
typedef enum
{
    QUEUING,
    SAMPLING,
} ARINC_PORT_TYPE;

#endif
