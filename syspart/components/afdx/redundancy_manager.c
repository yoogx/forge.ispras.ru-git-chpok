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
#include "REDUNDANCY_MANAGER_gen.h"
#include "afdx.h"
//~ #include "redundancy_manager_struct.h"
  
#define C_NAME "REDUNDANCY_MANAGER: "
#define NETWORK_CARD_A            0
#define NETWORK_CARD_B            1
  
void redundancy_manager_init(REDUNDANCY_MANAGER *self)
{
    int i, j;
    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        self->state.virtual_link_data[i].redundancy_management_data.last_accepted_seq_numb = 0;
        self->state.virtual_link_data[i].redundancy_management_data.last_accepted_msg_time = 0;
    }
    
    for (i = 0; i < SUBNETWORKS_COUNT; i++)
        for (j = 0; j <= MAX_SEQUENCE_NUMBER; j++)
        self->state.arrival_time[i][j] = 0;
}

/* This function search vl_index in configurations ports
 * INPUT: vl_id
 * OUTPUT: vl_temp (index of VL in array of ports)
 */
int identify_vl_index(REDUNDANCY_MANAGER *self, const uint16_t vl_id)
{
    int i = 2; 
    int vl_left = 0;
    int vl_right = VIRTUAL_LINKS_COUNT;
    int vl_temp = (vl_right + vl_left) / 2;
    
    while ((VIRTUAL_LINKS_COUNT / i) > 0)
    {
        if (vl_id == self->state.virtual_link_data[vl_temp].vl_id)
            break;
            
        if (vl_id > self->state.virtual_link_data[vl_temp].vl_id)
        {
            vl_left = vl_temp;
            vl_temp = (vl_left + vl_right) / 2;
        }
        else{
            vl_right = vl_temp;
            vl_temp = (vl_left + vl_right) / 2;
        }
        i *= 2;
    }
    
    return vl_temp;
}

/*
 * 
 */
pok_bool_t redundancy_management(REDUNDANCY_MANAGER *self, uint8_t seq_numb, int vl_index, pok_time_t msg_arrival_time, int network_card)
{
    /* Step 1 */
    //~ pok_bool_t rm_decision = FALSE;
    int N = (self->state.virtual_link_data[vl_index].skew_max / self->state.virtual_link_data[vl_index].BAG) + 2;
    
    if (network_card == NETWORK_CARD_A)
    {
    #ifdef PRINT_REDUNDANCY
        printf("Compare with card B\n");
        printf("arrival_time   %lli\n", msg_arrival_time);
        printf("ar_t_in_arr    %lli\n", self->state.arrival_time[NETWORK_CARD_B][seq_numb]);
        printf("diff           %lli\n", (msg_arrival_time - self->state.arrival_time[NETWORK_CARD_B][seq_numb]));
        printf("skew_max       %lli\n", self->state.virtual_link_data[vl_index].skew_max);
    #endif
        if ((msg_arrival_time - self->state.arrival_time[NETWORK_CARD_B][seq_numb]) < self->state.virtual_link_data[vl_index].skew_max)
        return FALSE;
    }else
    {
    #ifdef PRINT_REDUNDANCY
        printf("arrival_time   %lli\n", msg_arrival_time);
        printf("ar_t_in_arr    %lli\n", self->state.virtual_link_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_A][seq_numb]);
        printf("diff           %lli\n", (msg_arrival_time - self->state.arrival_time[NETWORK_CARD_A][seq_numb]));
        printf("skew_max       %lli\n", self->state.virtual_link_data[vl_index].skew_max);

        printf("Compare with card A\n");
    #endif
        if ((msg_arrival_time - self->state.arrival_time[NETWORK_CARD_A][seq_numb]) < self->state.virtual_link_data[vl_index].skew_max)
        return FALSE;
    }

    /* Step 2 */
    if ((msg_arrival_time - self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_msg_time) > self->state.virtual_link_data[vl_index].skew_max)
    {
        self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb = seq_numb;
        self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_msg_time = msg_arrival_time;
        return TRUE; /* Message approved */
    }else
    {
        if ((self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N) > 255)
        {
            if ((seq_numb > self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb) &&
                (seq_numb <= 255) &&
                (seq_numb >= 1) &&
                (seq_numb <= ((self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N) % 255)) )
            {
                self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb = seq_numb;
                self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_msg_time = msg_arrival_time;
            }
        //if ((vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N) < 255)
        }else {
            if ( (seq_numb > self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb) &&
                 (seq_numb <= (self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N)) )
            {
                self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_seq_numb = seq_numb;
                self->state.virtual_link_data[vl_index].redundancy_management_data.last_accepted_msg_time = msg_arrival_time;  
            }
        }
    }
    
    return FALSE;

}



// function for network device A
ret_t redundancy_manager_receive_packet_net_a(REDUNDANCY_MANAGER *self,
                                            const char *payload, 
                                            size_t payload_size,
                                            SYSTEM_TIME_TYPE arrival_time)
{
    frame_data_t *afdx_frame = (frame_data_t *)payload;
    uint8_t seq_numb = payload[payload_size - 1];

    //~ printf(C_NAME" get message for net_card A: %16s, SN= %u\n", afdx_frame->afdx_payload, seq_numb);
    pok_bool_t send = redundancy_management(self, seq_numb, identify_vl_index(self, afdx_frame->mac_header.mac_dst_addr.vl_id), arrival_time, NETWORK_CARD_A);
    if (send == TRUE) {
        printf(C_NAME"NET_A: TRUE\n");
        ret_t res = REDUNDANCY_MANAGER_call_portC_handle(self, payload, payload_size);
        //~ printf(C_NAME"============== A: SN= %u\n", seq_numb);
        if (res != EOK)
                printf(C_NAME"Error in send\n");
    }
        
    return EOK;
}

// function for network device B
ret_t redundancy_manager_receive_packet_net_b(REDUNDANCY_MANAGER *self,
                                            const char *payload, 
                                            size_t payload_size,
                                            SYSTEM_TIME_TYPE arrival_time)
{
    uint8_t seq_numb = payload[payload_size - 1];
    frame_data_t *afdx_frame = (frame_data_t *)payload;

    //~ printf(C_NAME" get message for net_card B: %16s, SN= %u\n", afdx_frame->afdx_payload, seq_numb);
    pok_bool_t send = redundancy_management(self, seq_numb, identify_vl_index(self, afdx_frame->mac_header.mac_dst_addr.vl_id), arrival_time, NETWORK_CARD_B);
    if (send == TRUE) {
        printf(C_NAME"NET_B: TRUE\n");
        ret_t res = REDUNDANCY_MANAGER_call_portC_handle(self, payload, payload_size);
        //~ printf(C_NAME"============== B: SN= %u\n", seq_numb);
        if (res != EOK)
                printf(C_NAME"Error in send\n");
    }
    return EOK;
}
