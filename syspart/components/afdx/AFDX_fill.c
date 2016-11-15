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
*                  AFDX Function that fill the frame 
*
* The following file is a part of the AFDX project. Any modification
* should made according to the AFDX standard.
*
*
* Created by ....
*/

 
 #include <stdint.h>
 #include <afdx/AFDX_ES_config.h>
 #include <afdx/AFDX_filling.h>
 #include <net/byteorder.h>
 #include <string.h>
 #include <stdio.h>

#include "AFDX_FILLER_gen.h"
#define C_NAME "AFDX_FILLER: "

void afdx_filler_init(AFDX_FILLER *self)
{
    RETURN_CODE_TYPE ret;
    printf(C_NAME"Hello\n");
}
