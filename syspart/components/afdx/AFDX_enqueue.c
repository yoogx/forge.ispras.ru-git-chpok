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
#include <utils.h>

#include "afdx.h"

#include <net/byteorder.h>

#include <mem.h>

// include generated files
#include "AFDX_QUEUE_ENQUEUER_gen.h"

#define C_NAME  "AFDX_QUEUE_ENQUEUER: "

// подумать куда перенести
#define SIZE_OF_HEADER              42
#define UDP_H_LENGTH                8

#define NETWORK_CARD_A              0
#define NETWORK_CARD_B              1

#define MAC_INTERFACE_ID_A  0x1    // (001 b) for the network A
#define MAC_INTERFACE_ID_B  0x2    // (010 b) for the network B
#define MAC_INTERFACE_ID_2  0

// information for queue
#define PACKET_COUNT    10

//-----DEBUG INFO-------------
//~ #define PRINT_IMPL
//~ #define PRINT_QUEUE
//~ #define PRINT_ACT
//----------------------------

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
    self->state.buffer = smalloc(PACKET_COUNT * sizeof(afdx_buffer));
    self->state.head = 0;
    self->state.tail = 0;
    self->state.count = 0;
    self->state.empty = TRUE;

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

    if ((system_time - self->state.last_time) <=  self->state.BAG)
        return;
    else
    {
        self->state.last_time = system_time;

        /* 
         * Take information from queue:
         * 1. copy from buffer[head].data size from buffer[head].size
         * 2. send packet
         * 3. Increase the head by 1
         * 4. decreace count by 1
         * 
         */
        if (self->state.empty == FALSE)
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
                        0,
                        0
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
                        0,
                        0
                        );

                AFDX_QUEUE_ENQUEUER_call_portNetB_flush(self);
            }
            
            /*
             * Chenge QUEUE state
             */
            self->state.head++;
            if (self->state.head == PACKET_COUNT)
                self->state.head = 0;

            self->state.count--;
            if (self->state.count == 0)
                self->state.empty = TRUE;
            //printf("AM = %d\n", self->state.count);
            return;
        }
        return;
    }
}



ret_t afdx_enqueuer_implementation(
        AFDX_QUEUE_ENQUEUER *self,
        char * afdx_frame,
        size_t frame_size
        )
{
    memcpy(&(self->state.buffer[self->state.tail].data), afdx_frame, frame_size);
    self->state.buffer[self->state.tail].size = frame_size;
    self->state.tail++;
    self->state.count++;
    self->state.empty = FALSE;

    if ((self->state.tail == PACKET_COUNT) && (self->state.count < PACKET_COUNT)) {
        self->state.tail = 0;
    }
    if (self->state.count >= PACKET_COUNT) {
        printf("ERROR QUEUE if FULL \n");
        
        //kill partition
        *(int*) 0x2000c = 1;
        return -1;
    }

 #ifdef PRINT_QUEUE
        printf(C_NAME"afdx_enqueuer_implementation \n");
        printf("empty = FALSE\n");
        printf("tail = %ld\n", self->state.tail);
        printf("head = %ld\n", self->state.head);

        char afdx_message_1[MAX_AFDX_FRAME_SIZE];
        frame_data_t *afdx_frame_1 = (frame_data_t *)afdx_frame;
        memcpy(afdx_message_1, &(self->state.queue[head_temp + SIZE_OF_HEADER]), (afdx_frame_1->udp_header.udp_length - UDP_H_LENGTH));

        printf(C_NAME" %s\n", afdx_message_1);
        printf("====\n");

#endif

 #ifdef PRINT_IMPL
        char afdx_message[MAX_AFDX_FRAME_SIZE];
        frame_data_t *afdx_frame_2 = (frame_data_t *)afdx_frame;
        strncpy(afdx_message, afdx_frame_2->afdx_payload, afdx_frame_2->udp_header.udp_length - UDP_H_LENGTH);

        printf(C_NAME" %s\n", afdx_message);
#endif

    return 0;
}
