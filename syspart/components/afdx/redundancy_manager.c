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

#define C_NAME "REDUNDANCY_MANAGER: "
#define NETWORK_CARD_A            0
#define NETWORK_CARD_B            1

/* DEBUG */
//~ #define PRINT_REDUNDANCY

ret_t rm_send_packet(REDUNDANCY_MANAGER *self,
                        const uint8_t *afdx_packet,
                        size_t afdx_packet_size)
{
    ret_t res = REDUNDANCY_MANAGER_call_portC_handle(self, afdx_packet, afdx_packet_size);
    if (res != EOK)
        printf(C_NAME"Error in send\n");
    return res;
}

void redundancy_manager_init(REDUNDANCY_MANAGER *self)
{
    /*
     * This variable helps to separate the situation
     * when the first message came very quickly (arrival_time < skew_max)
     * and its arrival timemay be 0.
     */

    int i, j, k;
    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        self->state.virtual_link_data[i].redundancy_management_data.first_message_received = FALSE;
        /*
        * This variable helps to separate the situation
        * when the first message came very quickly (arrival_time < skew_max)
        * and its arrival time may be 0.
        */

        for (k = 0; k < SUBNETWORKS_COUNT; k++)
            for (j = 0; j <= MAX_SEQUENCE_NUMBER; j++)
                self->state.virtual_link_data[i].redundancy_management_data.arrival_time[k][j] = 0;
    }

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
        } else {
            vl_right = vl_temp;
            vl_temp = (vl_left + vl_right) / 2;
        }
        i *= 2;
    }

    return vl_temp;
}

/*
 * This fuction implements an algorithm (Redundancy Concept) according to ARINC 664P7
 * INPUT:
 * Sequence number,
 * vl_index (from function identify_vl_index),
 * arrival time to the receiving side
 * network card, where the received message
 *
 * OUTPUT:
 * True - message approved
 * False - message rejected
 */
pok_bool_t redundancy_management(REDUNDANCY_MANAGER *self, uint8_t seq_numb, int vl_index, pok_time_t msg_arrival_time, int network_card)
{
    int another_net_card = (network_card == NETWORK_CARD_A) ? NETWORK_CARD_B : NETWORK_CARD_A;

#ifdef PRINT_REDUNDANCY
    if (network_card == NETWORK_CARD_A)
        printf("Compare with card B, vl_index %d\n", vl_index);
    else
        printf("Compare with card A, vl_index %d\n", vl_index);

    printf("arrival_time   %lli\n", msg_arrival_time);
    printf("ar_t_in_arr    %lli\n", self->state.virtual_link_data[vl_index].redundancy_management_data.arrival_time[another_net_card][seq_numb]);
    printf("diff           %lli\n", msg_arrival_time - self->state.virtual_link_data[vl_index].redundancy_management_data.arrival_time[another_net_card][seq_numb]);
    printf("skew_max       %lli\n", self->state.virtual_link_data[vl_index].skew_max);
#endif

    if (self->state.virtual_link_data[vl_index].redundancy_management_data.first_message_received == FALSE) {
    /*
     * Check that first message is not received.
     * change first_message_received to TRUE
     * set last_accepted_msg_time, last_accepted_seq_numb and arrivil time for current SN and Network Card
     */
        self->state.virtual_link_data[vl_index].redundancy_management_data.first_message_received = TRUE;
        self->state.virtual_link_data[vl_index].redundancy_management_data.arrival_time[network_card][seq_numb] = msg_arrival_time;
        return TRUE;
    } else if (msg_arrival_time - self->state.virtual_link_data[vl_index].redundancy_management_data.arrival_time[another_net_card][seq_numb] > self->state.virtual_link_data[vl_index].skew_max) {
        self->state.virtual_link_data[vl_index].redundancy_management_data.arrival_time[network_card][seq_numb] = msg_arrival_time;
        return TRUE;
    } else {
        return FALSE;
    }
}



/* function for network device A */
ret_t redundancy_manager_receive_packet_net_a(REDUNDANCY_MANAGER *self,
                                            const uint8_t *afdx_packet,
                                            size_t afdx_packet_size,
                                            SYSTEM_TIME_TYPE arrival_time)
{
    ret_t ret = check_afdx_frame_size(afdx_packet_size);
    if (ret != EOK) {
        return ret;
    }

    afdx_frame_t *afdx_frame = (afdx_frame_t *)afdx_packet;
    uint8_t seq_numb = afdx_packet[afdx_packet_size - 1];

    //~ printf(C_NAME" get message for net_card A: %16s, SN= %u\n", afdx_frame->afdx_payload, seq_numb);
    pok_bool_t rm_decision = redundancy_management(self, seq_numb, identify_vl_index(self, afdx_frame->mac_header.mac_dst_addr.vl_id), arrival_time, NETWORK_CARD_A);
    if (rm_decision == TRUE) {
        printf(C_NAME"NET_A: vl: %d TRUE\n", afdx_frame->mac_header.mac_dst_addr.vl_id);

        return rm_send_packet(self, afdx_packet, afdx_packet_size);
    }

    return EOK;
}

/* function for network device B */
ret_t redundancy_manager_receive_packet_net_b(REDUNDANCY_MANAGER *self,
                                            const uint8_t *afdx_packet,
                                            size_t afdx_packet_size,
                                            SYSTEM_TIME_TYPE arrival_time)
{
    ret_t ret = check_afdx_frame_size(afdx_packet_size);
    if (ret != EOK) {
        return ret;
    }

    uint8_t seq_numb = afdx_packet[afdx_packet_size - 1];
    afdx_frame_t *afdx_frame = (afdx_frame_t *)afdx_packet;

    //~ printf(C_NAME" get message for net_card B: %16s, SN= %u\n", afdx_frame->afdx_payload, seq_numb);
    pok_bool_t rm_decision = redundancy_management(self, seq_numb, identify_vl_index(self, afdx_frame->mac_header.mac_dst_addr.vl_id), arrival_time, NETWORK_CARD_B);
    if (rm_decision == TRUE) {
        printf(C_NAME"NET_B: vl %d TRUE\n", afdx_frame->mac_header.mac_dst_addr.vl_id);

        return rm_send_packet(self, afdx_packet, afdx_packet_size);
    }
    return EOK;
}
