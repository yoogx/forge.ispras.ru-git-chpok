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
#include <arinc653/logbook.h>
#include <arinc653/process.h>

void main(void) {
    printf("logbook test started\n");
    LOGBOOK_NAME_TYPE name = "logbook1";
    MESSAGE_SIZE_TYPE msgsize = 10;
    MESSAGE_RANGE_TYPE msgrange = 10;
    MESSAGE_RANGE_TYPE msgnbrange = 2;
    LOGBOOK_ID_TYPE lgid = 0;
    RETURN_CODE_TYPE retc = -1;
    
    CREATE_LOGBOOK(name, msgsize, msgrange, msgnbrange, &lgid, &retc);
    STOP_SELF();
}  
