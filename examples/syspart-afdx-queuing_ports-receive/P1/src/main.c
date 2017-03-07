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
#define PRINT
/* ============================== */

#define SECOND 1000000000LL

#define MAX_AFDX_FRAME_SIZE    1518
#define MAX_AFDX_PAYLOAD_SIZE 64
#define MAX_NB_MESSAGE 10

// Two modes Sampling and Queuing -
//~ #define SAMPLING_MODE
#define QUEUING_MODE
//---------------------------------

 #ifdef QUEUING_MODE
QUEUING_PORT_ID_TYPE QP1, QP2;
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
    RETURN_CODE_TYPE ret;

    TIMED_WAIT(1 * SECOND, &ret);

    while (1) {
        #ifdef SAMPLING_MODE
            VALIDITY_TYPE validity;
            MESSAGE_SIZE_TYPE len;
            int x = 0;

            READ_SAMPLING_MESSAGE(SP2, (MESSAGE_ADDR_TYPE) &afdx_payload, &len, &validity, &ret);
            x = 1;
            if (ret == NO_ERROR) {
                if (x == 1){
                    afdx_payload[len] = '\0';
                    printf("ARINC GET MESSAGE: %s\n", afdx_payload);
                    printf("=================================================\n");
                    x = 0;
                }
            }
        #endif

        #ifdef QUEUING_MODE

            MESSAGE_SIZE_TYPE len;

            RECEIVE_QUEUING_MESSAGE(QP2, INFINITE_TIME_VALUE, (MESSAGE_ADDR_TYPE) &afdx_payload, &len, &ret);

            if (ret == NO_ERROR) {
                afdx_payload[len] = '\0';
                printf("ARINC GET MESSAGE: %s\n", afdx_payload);
                printf("=================================================\n");
            }
        #endif
        TIMED_WAIT(2 * SECOND, &ret);
    }
    check_ret(ret);
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

    if (ret != NO_ERROR) {
        printf("P1_couldn't create port SP2, ret %d\n", (int) ret);
    } else {
        printf("P1_SP2 = %d\n", (int) SP2);
    }
#endif

 #ifdef QUEUING_MODE
    // CREATING QP port 1
    CREATE_QUEUING_PORT("QP1", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, SOURCE , FIFO, &QP1, &ret);
    if (ret != NO_ERROR) {
        printf("P1_couldn't create port QP1, ret %d\n", (int) ret);
    } else {
        printf("created P1_QP1 = %d\n", (int) QP1);
    }
    // CREATING QP port 2
    CREATE_QUEUING_PORT("QP2", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, DESTINATION, FIFO, &QP2, &ret);
    if (ret != NO_ERROR) {
        printf("P1_couldn't create port QP2, ret %d\n", (int) ret);
    } else {
        printf("created P1_QP2 = %d\n", (int) QP2);
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
