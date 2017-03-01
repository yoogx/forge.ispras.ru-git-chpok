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
#include <arinc653/queueing.h>
#include <arinc653/sampling.h>

QUEUING_PORT_ID_TYPE QP1;
QUEUING_PORT_ID_TYPE QP2;
#define SECOND 1000000000LL

#define BIG_ARRAY_SIZE 10000
char BIG_ARRAY[BIG_ARRAY_SIZE];

static void first_process(void)
{
    RETURN_CODE_TYPE ret;
    MESSAGE_SIZE_TYPE len;

    /* Wait a little until second partition is initialized */
    TIMED_WAIT(SECOND/5, &ret);

    SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) "Hello", 6, INFINITE_TIME_VALUE, &ret);

    /*
    while (1) {
        BIG_ARRAY[0] = 0;
        RECEIVE_QUEUING_MESSAGE(QP2, 5*SECOND, (MESSAGE_ADDR_TYPE) BIG_ARRAY, &len, &ret);

        if (ret != NO_ERROR) {
            printf("haven't get any packet\n");
            continue;
        }
        printf("got packet %d. Sending back\n", BIG_ARRAY[0]);

        BIG_ARRAY[0] = (BIG_ARRAY[0] + 1) % 255;
        SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) BIG_ARRAY, len, INFINITE_TIME_VALUE, &ret);

        if (ret != NO_ERROR) {
            printf("error while sending %d\n", ret);
        }
    }
    */

    while (1) {
        RECEIVE_QUEUING_MESSAGE(QP2, SECOND, (MESSAGE_ADDR_TYPE) BIG_ARRAY, &len, &ret);
        if (ret == NO_ERROR) {
            int COUNT = BIG_ARRAY[0];
            printf("START. sending %d\n", COUNT);

            for (int i = 0; i < COUNT; i++) {
                //if (i%10 == 0)
                //    printf("P1: sending %d packet\n", i);
                BIG_ARRAY[0] = i;
                SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) BIG_ARRAY, len, INFINITE_TIME_VALUE, &ret);
                if (ret != NO_ERROR) {
                    printf("error while sending %d\n", ret);
                }
            }

            //for (int i = 0; i < COUNT; i++) {
            //    MESSAGE_SIZE_TYPE len;
            //    BIG_ARRAY[0] = 0;
            //    RECEIVE_QUEUING_MESSAGE(QP2, SECOND, (MESSAGE_ADDR_TYPE) BIG_ARRAY, &len, &ret);

            //    if (ret != NO_ERROR || BIG_ARRAY[0] == i + 1) {
            //        printf("lost packet :(\n");
            //    }
            //}
            printf("STOP\n");
        }
    }
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


    // create ports
    CREATE_QUEUING_PORT("QP1", 4096, 150, SOURCE,      FIFO, &QP1, &ret);
    if (ret != NO_ERROR) printf("P1: cannot create QP1\n");
    CREATE_QUEUING_PORT("QP2", 4096, 150, DESTINATION, FIFO, &QP2, &ret);
    if (ret != NO_ERROR) printf("P1: cannot create QP2\n");


    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    printf("going to NORMAL mode...\n");
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
