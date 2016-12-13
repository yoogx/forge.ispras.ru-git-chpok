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
*
* Created by ....
*/



#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <arinc653/time.h>
#include <arinc653/partition.h>

#include <utils.h>

#include "afdx.h"

#include <net/byteorder.h>

#include <mem.h>

// include generated files
#include "AFDX_QUEUE_ENQUEUER_gen.h"

#define C_NAME  "AFDX_QUEUE_ENQUEUER: "

#define SIZE_OF_HEADER              42
#define UDP_H_LENGTH                8

#define NETWORK_CARD_A              0
#define NETWORK_CARD_B              1

#define MAC_INTERFACE_ID_A  0x1    // (001 b) for the network A
#define MAC_INTERFACE_ID_B  0x2    // (010 b) for the network B
#define MAC_INTERFACE_ID_2  0

void fill_afdx_interface_id (frame_data_t *p, int net_card)
{
    if (net_card == 0)
    p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_A << 5 | MAC_INTERFACE_ID_2);
    else
    p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_B << 5 | MAC_INTERFACE_ID_2);
}


void afdx_queue_init(AFDX_QUEUE_ENQUEUER *self)
{
    SYSTEM_TIME_TYPE system_time;
    RETURN_CODE_TYPE ret;

    /*
     * Initialize the memory for the queue,
     * head, tail, the number of elements
     * and empty srate
     */
    self->state.buffer = smalloc(self->state.packet_count * sizeof(afdx_buffer));
    self->state.head = 0;
    self->state.tail = 0;
    self->state.count = 0;

    // only for TEST!
    GET_TIME(&system_time, &ret);
    self->state.last_time = system_time;
}


void afdx_queue_enqueuer_activity(AFDX_QUEUE_ENQUEUER *self)
{
    SYSTEM_TIME_TYPE system_time;
    RETURN_CODE_TYPE return_code;
    char    data_buffer[MAX_AFDX_FRAME_SIZE];
    frame_data_t    *afdx_frame = (frame_data_t *) data_buffer;

    // only for TEST!
    GET_TIME(&system_time, &return_code);


    if (system_time <= (self->state.last_time + self->state.BAG))
    {
        self->state.last_time += self->state.BAG;

        /* 
         * Take information from queue:
         * 1. copy from buffer[head].data size from buffer[head].size
         * 2. send packet
         * 3. Increase the head by 1
         * 4. decreace count by 1
         */
        if (self->state.count > 0)
        {
            memcpy(afdx_frame, &(self->state.buffer[self->state.head].data), self->state.buffer[self->state.head].size);
            printf("QUEUE activity get message: %s\n", afdx_frame->afdx_payload);
            /*
             * Take packet and write information of interface_id and send to the cards
             */
             //send to 1 card
            {
                printf("Sending to the card A \n");
                fill_afdx_interface_id(afdx_frame, NETWORK_CARD_A);

                AFDX_QUEUE_ENQUEUER_call_portNetA_send(self,
                        data_buffer,
                        self->state.buffer[self->state.head].size,
                        self->state.buffer[self->state.head].prepend_overhead,
                        self->state.buffer[self->state.head].append_overhead
                        );

                AFDX_QUEUE_ENQUEUER_call_portNetA_flush(self);
            }

            //send to 2 card
            {
                printf("Sending to the card B\n");
                fill_afdx_interface_id(afdx_frame, NETWORK_CARD_B);

                AFDX_QUEUE_ENQUEUER_call_portNetB_send(self,
                        data_buffer,
                        self->state.buffer[self->state.head].size,
                        self->state.buffer[self->state.head].prepend_overhead,
                        self->state.buffer[self->state.head].append_overhead
                        );

                AFDX_QUEUE_ENQUEUER_call_portNetB_flush(self);
            }
            
            /*
             * Chenge QUEUE state
             */
            self->state.head++;
            if (self->state.head == self->state.packet_count)
                self->state.head = 0;

            self->state.count--;

        }
    }
}



ret_t afdx_enqueuer_implementation(
        AFDX_QUEUE_ENQUEUER *self,
        char * afdx_frame,
        const size_t frame_size,
        const size_t prepend_overhead,
        const size_t append_overhead
        )
{

    memcpy(&(self->state.buffer[self->state.tail].data), afdx_frame, frame_size);
    self->state.buffer[self->state.tail].size = frame_size;
    self->state.buffer[self->state.tail].prepend_overhead = prepend_overhead;
    self->state.buffer[self->state.tail].append_overhead = append_overhead;

    self->state.tail++;
    self->state.count++;

    if ((self->state.tail == self->state.packet_count) && (self->state.count < self->state.packet_count)) {
        self->state.tail = 0;
    }
    if ( ((self->state.tail == self->state.packet_count) && (self->state.count >= self->state.packet_count)) ||
         ((self->state.tail == self->state.head)  && (self->state.count >= self->state.packet_count)) )  {

        printf("ERROR QUEUE if FULL \n");
        printf("Partition P2 will be stopped\n");
        //kill partition
        RETURN_CODE_TYPE ret;
        SET_PARTITION_MODE(IDLE, &ret);
        return -1;
    }

    return 0;
}

ret_t afdx_enqueuer_flush(AFDX_QUEUE_ENQUEUER *self)
{
    return EOK;
}
