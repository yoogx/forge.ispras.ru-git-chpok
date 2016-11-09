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

/* ============================== */
//~ #define PRINT
/* ============================== */

//~ #include <afdx/AFDX_ES.h>
//~ #include <afdx/AFDX_ES_config.h>

#define SECOND 1000000000LL

#define MAX_AFDX_FRAME_SIZE    114
#define MAX_AFDX_PAYLOAD_SIZE 64
#define MAX_NB_MESSAGE 10

QUEUING_PORT_ID_TYPE QP1, QP2;


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

    while (1) {
            if (i == 0){
                strcpy(afdx_payload, "Hello!");
            #ifdef PRINT
                printf("P1_test message: %s\n", afdx_payload);
            #endif
                SEND_QUEUING_MESSAGE(QP1, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, 0, &ret);
            }
            if (i == 1){
                strcpy(afdx_payload, "How are you?");
            #ifdef PRINT
                printf("P1_test message: %s\n", afdx_payload);
            #endif
                SEND_QUEUING_MESSAGE(QP2, (MESSAGE_ADDR_TYPE) &afdx_payload, strlen(afdx_payload) + 1, 0, &ret);
            }

            check_ret(ret);
            i = 1 -i;

            TIMED_WAIT(SECOND / 3, &ret);
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

    //CREATING QP port 1
    CREATE_QUEUING_PORT("QP1", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, SOURCE , FIFO, &QP1, &ret);
    printf("P1_QP1 = %d\n", (int) QP1);
    if (ret != NO_ERROR) {
        printf("P2_couldn't create port QP1, ret %d\n", (int) ret);
    }
    //CREATING QP port 3
    CREATE_QUEUING_PORT("QP3", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, SOURCE , FIFO, &QP2, &ret);
    printf("P1_QP3 = %d\n", (int) QP2);
    if (ret != NO_ERROR) {
        printf("P2_couldn't create port QP3, ret %d\n", (int) ret);
    }  


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
