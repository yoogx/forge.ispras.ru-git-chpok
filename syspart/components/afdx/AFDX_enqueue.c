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
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <process.h>

#include <arinc653/time.h>
#include <arinc653/partition.h>

#include <utils.h>

#include "afdx.h"

#include <net/byteorder.h>

#include <smalloc.h>

#include <msection.h>

/* include generated files */
#include "AFDX_QUEUE_ENQUEUER_gen.h"

#define C_NAME  "AFDX_QUEUE_ENQUEUER: "

#define SIZE_OF_HEADER              42
#define UDP_H_LENGTH                8

#define NETWORK_CARD_A              0
#define NETWORK_CARD_B              1

#define MAC_INTERFACE_ID_A  0x1    // (001 b) for the network A
#define MAC_INTERFACE_ID_B  0x2    // (010 b) for the network B
#define MAC_INTERFACE_ID_2  0

static struct msection sec;

void fill_afdx_interface_id (afdx_frame_t *p, int net_card)
{
    if (net_card == NETWORK_CARD_A)
        p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_A << 5 | MAC_INTERFACE_ID_2);
    else
        p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_B << 5 | MAC_INTERFACE_ID_2);
}

/*
 * Take packet and write information of interface_id and send to the cards
 */
void send_packet(AFDX_QUEUE_ENQUEUER *self, afdx_frame_t *afdx_frame, int net_card)
{
    fill_afdx_interface_id(afdx_frame, net_card);

    if (net_card == NETWORK_CARD_A) {
        ret_t res = AFDX_QUEUE_ENQUEUER_call_portNetA_send(self,
                        self->state.buffer[self->state.head].data,
                        self->state.buffer[self->state.head].size,
                        self->state.buffer[self->state.head].prepend_max_size,
                        self->state.buffer[self->state.head].append_max_size
                        );
        if (res != EOK)
            printf(C_NAME"Error in send to card A\n");

        res = AFDX_QUEUE_ENQUEUER_call_portNetA_flush(self);

        if (res != EOK)
            printf(C_NAME"Error in flush to card A\n");

    } else {

        ret_t res = AFDX_QUEUE_ENQUEUER_call_portNetB_send(self,
                        self->state.buffer[self->state.head].data,
                        self->state.buffer[self->state.head].size,
                        self->state.buffer[self->state.head].prepend_max_size,
                        self->state.buffer[self->state.head].append_max_size
                        );
        if (res != EOK)
            printf(C_NAME"Error in send to card B\n");

        res = AFDX_QUEUE_ENQUEUER_call_portNetB_flush(self);

        if (res != EOK)
            printf(C_NAME"Error in flush to card B\n");
    }
}

static void afdx_queue_enqueuer_activity(void)
{
    AFDX_QUEUE_ENQUEUER *self;
    RETURN_CODE_TYPE ret;

    printf("in afdx_queue_enqueuer_activity\n");
    self = (AFDX_QUEUE_ENQUEUER *)jet_get_my_data();

    while (1) {
        if (self->state.cur_queue_size != 0) {

            afdx_frame_t    *afdx_frame = (afdx_frame_t *) self->state.buffer[self->state.head].data;

            printf("%s \n", self->instance_name);
            printf("QUEUE activity get message: %s\n", afdx_frame->afdx_payload);

             /* send to 1 card */
            {
                printf("Sending to the card A \n");
                send_packet(self, afdx_frame, NETWORK_CARD_A);
            }

            /* send to 2 card */
            {
                printf("Sending to the card B \n");
                send_packet(self, afdx_frame, NETWORK_CARD_B);
            }

            /* Change QUEUE state */
            self->state.head++;

            if (self->state.head == self->state.max_queue_size)
                self->state.head = 0;

            msection_enter(&sec);     //process entered mutual section
            self->state.cur_queue_size--;
            msection_leave(&sec);     // process leaved the section
        } else {
            printf("%s \n", self->instance_name);
            printf("there are no messages, I'm at the END afdx_queue_enqueuer_activity \n");
        }
        PERIODIC_WAIT(&ret);
        printf("==========================\n");
    }
}

void afdx_queue_init(AFDX_QUEUE_ENQUEUER *self)
{
    printf("IN afdx_queue_init\n");
    RETURN_CODE_TYPE ret;

    /*
     * Initialize msection
     * to work with cur_queue_size
     */
    msection_init(&sec);

    /*
     * Initialize the memory for the queue,
     * head, tail, the number of elements
     * and empty srate
     */
    self->state.buffer = smalloc(self->state.max_queue_size * sizeof(afdx_buffer));
    self->state.head = 0;
    self->state.tail = 0;
    self->state.cur_queue_size = 0;

    /* Create process */
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = self->state.BAG,
        .TIME_CAPACITY = self->state.BAG,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MAX_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };

    /*
     * Providing different process names
     */
    process_attrs.ENTRY_POINT = afdx_queue_enqueuer_activity;
    /*
     * We hope, that instance names are unique being truncated to sizeof PROCESS_NAME_TYPE
     */
    strncpy(process_attrs.NAME, self->instance_name, sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't create %s: %d\n", process_attrs.NAME, (int) ret);
        return;
    } else {
        printf("process %s created\n", process_attrs.NAME);
    }

    jet_set_process_data(pid, self);

    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("couldn't start process %s: %d\n", process_attrs.NAME, (int) ret);
        return;
    } else {
        printf("process %s \"started\" (it won't actually run until operating mode becomes NORMAL)\n", process_attrs.NAME);
    }

}

ret_t afdx_enqueuer_implementation(
        AFDX_QUEUE_ENQUEUER *self,
        char * afdx_frame,
        const size_t frame_size,
        const size_t prepend_max_size,
        const size_t append_max_size
        )
{
    printf("%s I'm in implementation \n", self->instance_name);
    if (self->state.cur_queue_size >= self->state.max_queue_size) {

        printf("ERROR QUEUE if FULL \n");
        //~ printf("Partition P2 will be stopped\n");
        //~ //kill partition
        //~ RETURN_CODE_TYPE ret;
        //~ SET_PARTITION_MODE(IDLE, &ret);
        //~ return -1;

        return EINVAL; // maybe EAGAIN?? NOT_AVAILABLE
    }
    /* Queuing */
    memcpy(&(self->state.buffer[self->state.tail].data), afdx_frame, frame_size);
    self->state.buffer[self->state.tail].size = frame_size;
    self->state.buffer[self->state.tail].prepend_max_size = prepend_max_size;
    self->state.buffer[self->state.tail].append_max_size = append_max_size;

    self->state.tail++;
    //~ printf("STATE QUEUE %d \n", self->state.cur_queue_size);

    if (self->state.tail == self->state.max_queue_size) {
        self->state.tail = 0;
    }
    msection_enter(&sec); //process entered mutual section
    self->state.cur_queue_size++;
    msection_leave(&sec); //process leaved the section

    return EOK;
}

ret_t afdx_enqueuer_flush(AFDX_QUEUE_ENQUEUER *self)
{
    AFDX_QUEUE_ENQUEUER_call_portNetA_flush(self);
    AFDX_QUEUE_ENQUEUER_call_portNetB_flush(self);
    return EOK;
}
