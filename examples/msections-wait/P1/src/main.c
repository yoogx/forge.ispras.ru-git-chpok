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
#include <kernel_shared_data.h>

#include <msection.h>

static int completion = 0;
static pok_thread_id_t waited_thread = JET_THREAD_ID_NONE;

static struct msection s;

static void first_process(void)
{
    printf("First process starts.\n");
    msection_enter(&s);
    
    completion = 1;
    if(waited_thread != JET_THREAD_ID_NONE)
    {
        printf("Notify second thread\n");
        msection_notify(&s, waited_thread);
    }
    
    msection_leave(&s);
    printf("First process is finishing.\n");

    STOP_SELF();
}

static void second_process(void)
{
    printf("Second process starts.\n");

    msection_enter(&s);
    if(completion == 0)
    {
        waited_thread = kshd->current_thread_id;
        printf ("Second process waits for completion.\n");
        msection_wait(&s, POK_TIME_INFINITY);
        printf ("Second process resumes.\n");
        waited_thread = JET_THREAD_ID_NONE;
    }
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

    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process 2: %d\n", (int) ret);
        return 1;
    } else {
        printf("process 2 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
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
