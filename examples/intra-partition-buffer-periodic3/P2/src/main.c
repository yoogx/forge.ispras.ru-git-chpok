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

static BUFFER_ID_TYPE global_buffer_id;

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    int i = 0;
    while (1) {
        SEND_BUFFER(global_buffer_id, (MESSAGE_ADDR_TYPE) &i, sizeof(i), 0, &ret);
        if (ret != NO_ERROR) {
            printf("P2: couldn't send to the buffer: %d\n", (int) ret);
            break;
        } 
        i++;
        PERIODIC_WAIT(&ret);
    }
    STOP_SELF();
}

static void second_process(void)
{
    RETURN_CODE_TYPE ret;
    while (1) {
        int i;
        MESSAGE_SIZE_TYPE len;

        RECEIVE_BUFFER(global_buffer_id, INFINITE_TIME_VALUE, (MESSAGE_ADDR_TYPE) &i, &len, &ret);

        if (ret != NO_ERROR) {
            printf("P2: couldn'd receive from the buffer: %d\n", (int) ret);
            break;
        }
        printf("P2: received message %d\n", i);
    }
    STOP_SELF();
}

 

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    BUFFER_ID_TYPE id;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE periodic_process_attrs = {
        .PERIOD = 100000000UL,
        .TIME_CAPACITY = 25000000UL,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    // create buffer
    CREATE_BUFFER("foo", sizeof(int), 10, FIFO, &id, &ret);
    if (ret != NO_ERROR) {
        printf("error creating a buffer: %d\n", (int) ret);
        return 1;
    } else {
        printf("buffer successfully created\n");
    }
    global_buffer_id = id;

    // create process 1
    periodic_process_attrs.ENTRY_POINT = first_process;
    strncpy(periodic_process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&periodic_process_attrs, &pid, &ret);
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
