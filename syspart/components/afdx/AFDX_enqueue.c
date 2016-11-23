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

#include <afdx/AFDX_ES_config.h>
#include <afdx/AFDX_filling.h>

#include <net/byteorder.h>

// include generated files
#include "AFDX_QUEUE_ENQUEUER_gen.h"

#define C_NAME  "AFDX_QUEUE_ENQUEUER: "

// подумать куда перенести
#define MAX_AFDX_FRAME_SIZE         114
#define SIZE_OF_HEADER              42

#define NETWORK_CARD_A              0
#define NETWORK_CARD_B              1

#define MAC_INTERFACE_ID_A  0x1    // (001 b) for the network A
#define MAC_INTERFACE_ID_B  0x2    // (010 b) for the network B

#define MAC_INTERFACE_ID_2  0

SYSTEM_TIME_TYPE last_time;
uint32_t head;                  /* head of queue */
uint32_t tail;                  /* tail of queue */
uint32_t amount_of_elements;    /* amount of elements in the queue */
uint32_t size_of_element[20];   /* size of each element in the queue */
pok_bool_t empty;               /* the queue is empty: TRUE/FALSE */

void fill_afdx_interface_id (frame_data_t *p, int net_card)
{
    if (net_card == 0)
    p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_A << 5 | MAC_INTERFACE_ID_2);
    else
    p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_B << 5 | MAC_INTERFACE_ID_2);
}


void afdx_queue_init(AFDX_QUEUE_ENQUEUER *self)
{
    //~ RETURN_CODE_TYPE ret;
    last_time = 0;
    head = 0;
    tail = 0;
    amount_of_elements = -1;
    empty = TRUE;
}


void afdx_queue_activity(AFDX_QUEUE_ENQUEUER *self)
{
    SYSTEM_TIME_TYPE system_time;
    RETURN_CODE_TYPE return_code;
    char    data_buffer[MAX_AFDX_FRAME_SIZE];
    frame_data_t    *afdx_frame = (frame_data_t *) data_buffer;


    GET_TIME(&system_time, &return_code);
    
    if ((system_time - last_time) >= self->state.BAG)
        return;
    else {
        /* 
         * Take information from queue:
         * 1. copy from self->state.queue[head] size of size_of_element[0]
         * 2. Change the head of queue to head + size_of_element[0] + 1
         * 3. decreace amount_of_elements
         * 4. rewrite the array of sizes. It is necessary that all sizes are shifted 1 cell to the left
         * 
         */
        int i;
        memcpy(afdx_frame, &(self->state.queue[head]), size_of_element[0]);
        head += size_of_element[0] + 1; 
        amount_of_elements--;
        for (i = 0; i < amount_of_elements; i++)
            size_of_element[i] = size_of_element[i + 1];
        
        /*
         * Take packet and write information of interface_id and send to the cards 
         */
         
        //send to 1 card
        {
            printf("Sending to the card A \n");
            fill_afdx_interface_id(afdx_frame, NETWORK_CARD_A);
            
            AFDX_QUEUE_ENQUEUER_call_portNetA_send(self,
                    data_buffer,
                    sizeof(data_buffer),
                    0,
                    0
                    );

        }

        //send to 2 card
        {

            printf("Sending to the card B from \n");
            fill_afdx_interface_id(afdx_frame, NETWORK_CARD_B);
            
            AFDX_QUEUE_ENQUEUER_call_portNetB_send(self,
                    data_buffer,
                    sizeof(data_buffer),
                    0,
                    0
                    );

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
    // the queue is not empty
    if (empty == FALSE)
    {
        uint32_t head_temp;
        head_temp = tail + 1;
        tail += frame_size;
        amount_of_elements++;
        size_of_element[amount_of_elements] = frame_size;
        memcpy(&(self->state.queue[head_temp]), afdx_frame, frame_size);
        
        return 0;

    } else {
        empty = FALSE;
        // как понять, что конец очереди?
        // надо придумать как реализовать
        tail += frame_size;
        amount_of_elements++;
        size_of_element[amount_of_elements] = frame_size;
        memcpy(&(self->state.queue[head]), afdx_frame, frame_size);
        
        return 0;
    }
}
