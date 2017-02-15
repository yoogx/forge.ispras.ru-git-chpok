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
    //~ RETURN_CODE_TYPE ret;
    //~ int k = 0;
    //~ uint32_t rval;
    //~ while ( 1 == 1){
        //~ for (int i = 0; i < 100000000; i++){
            //~ k++;
        //~ }
 //~ 
        //~ asm volatile("move %0, $sp" : "=r" (rval));
        //~ printf("First Part!\n");//             sp = 0x%x\n", rval);
        //~ rval = 0;
        //~ TIMED_WAIT(1000LL, &ret);
    //~ }
    RETURN_CODE_TYPE ret;
    int i = 0;
    while (1) {
        printf("Ready for send\n");
        SEND_BUFFER(global_buffer_id, (MESSAGE_ADDR_TYPE) &i, sizeof(i), 0, &ret);
        if (ret != NO_ERROR) {
            printf("couldn't send to the buffer: %d\n", (int) ret);
            break;
        } 
        i++;
        TIMED_WAIT(10000LL, &ret);
    }
}

static void second_process(void)
{
    //~ RETURN_CODE_TYPE ret;
    //~ int k = 0;
    //~ uint32_t rval;
    //~ while ( 1 == 1){
        //~ for (int i = 0; i < 100000000; i++){
            //~ k++;
        //~ }
        //~ asm volatile("move %0, $sp" : "=r" (rval));
        //~ printf("Second Part!\n");// sp = 0x%x\n", rval);
        //~ rval = 0;
        //~ TIMED_WAIT(1000LL, &ret);
    //~ }
    RETURN_CODE_TYPE ret;
    while (1) {
        printf("Ready for receive\n");
        int i;
        MESSAGE_SIZE_TYPE len;

        RECEIVE_BUFFER(global_buffer_id, INFINITE_TIME_VALUE, (MESSAGE_ADDR_TYPE) &i, &len, &ret);

        if (ret != NO_ERROR) {
            printf("couldn'd receive from the buffer: %d\n", (int) ret);
            break;
        }
        printf("received message %d\n", i);
    }
    STOP_SELF();
}

 

static int real_main(void)
{
    RETURN_CODE_TYPE ret;
    BUFFER_ID_TYPE id;
    PROCESS_ID_TYPE pid;
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
    printf("- I'm alive! - said Part 1\n"); 
    //~ int k = 0;
    //~ while ( 1 == 1){
        //~ for (int i = 0; i < 100000000; i++){
            //~ k++;
        //~ }
        //~ printf("I'm still alive!\n");
        //~ printf("    - I'm still alive! - said Part 1\n"); 
    //~ }
    real_main();
    STOP_SELF();
}  
