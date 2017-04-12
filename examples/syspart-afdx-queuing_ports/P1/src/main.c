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

/* Uncomment this if you want to print some debug information */
#define PRINT

#define SECOND 1000000000LL

#define MAX_AFDX_FRAME_SIZE    1518
#define MAX_AFDX_PAYLOAD_SIZE 64
#define MAX_NB_MESSAGE 10

 /*
  * Two modes Sampling and Queuing
  * Uncomment mode, which you want to use
  */
//~ #define SAMPLING_MODE
#define QUEUING_MODE
//---------------------------------

 #ifdef QUEUING_MODE
QUEUING_PORT_ID_TYPE QP1, QP2, QP3;
#endif

#ifdef SAMPLING_MODE
SAMPLING_PORT_ID_TYPE SP1, SP2;
#endif


static void check_ret(RETURN_CODE_TYPE ret)
{
    if (ret != NO_ERROR) {
        printf("check_ret_error: %d\n", (int) ret);
    }
}


static void first_process(void)
{
    // send messages in bursts of 10 (maximum queued amount)
    char    afdx_payload[MAX_AFDX_PAYLOAD_SIZE];
    int i = 0;
    RETURN_CODE_TYPE ret;

    TIMED_WAIT(1 * SECOND, &ret);

    while (1) {
            if (i == 0){
                strcpy(afdx_payload, "Hello!");
            #ifdef PRINT
                printf("P1_test message: %s\n", afdx_payload);
            #endif

            #ifdef SAMPLING_MODE
                WRITE_SAMPLING_MESSAGE(SP1, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, &ret);
            #endif

            #ifdef QUEUING_MODE
                SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, 0, &ret);
                SEND_QUEUING_MESSAGE(QP3, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, 0, &ret);
            #endif
            }
            if (i == 1){
                strcpy(afdx_payload, "How are you?");
            #ifdef PRINT
                printf("P1_test message: %s\n", afdx_payload);
            #endif

            #ifdef SAMPLING_MODE
                WRITE_SAMPLING_MESSAGE(SP1, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, &ret);
            #endif

            #ifdef QUEUING_MODE
                SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, 0, &ret);
                SEND_QUEUING_MESSAGE(QP3, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, 0, &ret);
            #endif
            }

            check_ret(ret);
            i = 1 -i;

            //printf("P1_main, first_process: Here!\n");
            TIMED_WAIT(5 * SECOND, &ret);
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
        printf("P1_couldn't create process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("P1_process 1 created\n");
    }

    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("P1_couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("P1_process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }

#ifdef SAMPLING_MODE
        CREATE_SAMPLING_PORT("SP1", MAX_AFDX_PAYLOAD_SIZE, SOURCE, SECOND, &SP1, &ret);
    if (ret != NO_ERROR) {
        printf("P1_couldn't create port SP1, ret %d\n", (int) ret);
    } else {
        printf("P1_SP1 = %d\n", (int) SP1);
    }

        CREATE_SAMPLING_PORT("SP2", MAX_AFDX_PAYLOAD_SIZE, DESTINATION, SECOND, &SP2, &ret);
#endif

 #ifdef QUEUING_MODE
    // CREATING QP port 1
    CREATE_QUEUING_PORT("QP1", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, SOURCE , FIFO, &QP1, &ret);

    if (ret != NO_ERROR) {
        printf("P1_couldn't create port QP1, ret %d\n", (int) ret);
    } else {
        printf("P1_created QP1 = %d\n", (int) QP1);
    }

    // CREATING QP port 2
    CREATE_QUEUING_PORT("QP2", 64, 10, DESTINATION, FIFO, &QP2, &ret);

    if (ret != NO_ERROR) {
        printf("P1_couldn't create port QP2, ret %d\n", (int) ret);
    } else {
        printf("P1_created QP2 = %d\n", (int) QP2);
    }
    // CREATING QP port 3
    CREATE_QUEUING_PORT("QP3", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, SOURCE , FIFO, &QP3, &ret);

    if (ret != NO_ERROR) {
        printf("P1_couldn't create port QP3, ret %d\n", (int) ret);
    } else {
        printf("P1_created QP3 = %d\n", (int) QP3);
    }
#endif

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
