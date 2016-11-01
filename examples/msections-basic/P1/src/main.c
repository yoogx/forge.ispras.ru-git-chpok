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
#include <arinc653/buffer.h>
#include <arinc653/partition.h>
#include <arinc653/time.h>

#include <msection.h>

static int volatile global_val = 0;

static struct msection s;

static void first_process(void)
{
    printf("First process starts.\n");
    msection_enter(&s);
    printf("First process has been entered mutual section.\n");

    for(int i = 0; i < 5000000; i++)
        global_val ++;

    printf("First process is leaving the section.\n");
    msection_leave(&s);
    printf("First process is finishing.\n");

    STOP_SELF();
}

static void second_process(void)
{
    printf("Second process starts.\n");
    msection_enter(&s);
    printf("Second process has been entered mutual section.\n");

    printf("Second process is leaving the section.\n");
    msection_leave(&s);
    
    printf("Second process is finishing.\n");

    STOP_SELF();
}

 

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    msection_init(&s);

    // create process 1
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 created\n");
    }
    
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }

    // create process 2
    process_attrs.ENTRY_POINT = second_process;
    process_attrs.BASE_PRIORITY ++;
    strncpy(process_attrs.NAME, "process 2", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create process 2: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 2 created\n");
    }

    DELAYED_START(pid, 20000000/*20 ms*/, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 2: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 2 \"started\" (it won't actually run until operating mode becomes NORMAL and delay elapsed)\n");
    }

    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("couldn't transit to normal operating mode: %d\n", (int) ret);
    } 

    STOP_SELF();
    return 0;
}

void main(void) {
    real_main();
    STOP_SELF();
}  
