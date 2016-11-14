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
/*==============================*/
#include <afdx/AFDX_ES.h>
#include <afdx/AFDX_ES_config.h>
#include <afdx/hexDump.h>
#include <afdx/AFDX_frame.h>
#include <afdx/AFDX_filling.h>
/*==============================*/
#include <net/network.h>
#include <depl.h>
#include <port_info.h>
/*==============================*/
#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>

#include <net/netdevices.h>
#include <channel_driver.h>
/*==============================*/
#define PRINT
#define PRINT_RECEIVE
#define PRINT_REDUNDANCY
/*==============================*/

#define UDP_H_LENGTH            8
#define SIZE_OF_HEADER            42
#define SECOND                     1000000000LL

#define MAX_AFDX_FRAME_SIZE        114
//~ #define MAX_AFDX_PAYLOAD_SIZE     64

//~ #define MAX_NB_MESSAGE             10

//~ #define POK_NETWORK_UDP         (14 + 20 + 8)
//~ #define POK_NETWORK_OVERHEAD     (POK_NETWORK_UDP)
#define NETWORK_CARD_A            0
#define NETWORK_CARD_B            1

QUEUING_PORT_ID_TYPE QP1, QP2;

pok_netdevice_t *current_netdevice;

void *callback_arg;
char *ipnet_netdev_name_1;
int network_card;  /* to divide cards */

MESSAGE_SIZE_TYPE payload_size;

/* 
 *         Временное решение
 * массив указателей на массивы, содержащие информацию об
 * ARINC портах, куда следует отсылать сообщения
 * 
 */
int *arinc_ports[VIRTUAL_LINKS_COUNT];
int arinc_one[] = {0, 0, 0, 3, 0, 0};
int arinc_two[] = {0, 0, 0, 0, 4, 0};


//~ pok_network_buffer_callback_t callback;
void send_callback_m(void *pointer)
{
    printf("yes");
}
/* This function search vl_index in configurations ports
 * INPUT: vl_id
 * OUTPUT: vl_temp (index of VL in array of ports)
 */
int identify_vl_index(const uint16_t vl_id)
{
    int i = 2; 
    int vl_left = 0;
    int vl_right = VIRTUAL_LINKS_COUNT;
    int vl_temp = (vl_right + vl_left) / 2;
    
    while ((VIRTUAL_LINKS_COUNT / i) > 0)
    {
        if (vl_id == vl_data[vl_temp].vl_id)
            break;
            
        if (vl_id > vl_data[vl_temp].vl_id)
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
 * This function initializes the initial state 
 * immediately after startup 
 */
void integrity_checking_init()
{
    int i, j;
    for (j = 0; j < VIRTUAL_LINKS_COUNT; j++)
        for (i = 0; i < SUBNETWORKS_COUNT; i++)
        {
            vl_data[j].integrity_check_data[i].last_in_seq_number = 0;
            vl_data[j].integrity_check_data[i].first_message_received = FALSE;
        }
}

/*
 * Comparisons last_in_seq_numb and sn (for each of the subnets),
 * if the difference between sn and last_in_seq_numb is 1 or 2,
 * we accept this message
 * (PS do not forget that zero message is sent immediately after the restart
 * of the system is sn = 0,
 * and the remaining messages are numbered from 1 to 255
 * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * INPUT:
 *  - sequence_number
 *  - index of VL id in array of ports
 * OUTPUT:
 *  - TRUE if message accepted
 *  - FALSE if message not accepted
 */
pok_bool_t integrity_checking(uint8_t seq_numb, int vl_index)
{
    /* Transmission ES restarted */
    if (seq_numb == 0)
    {
        vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number = seq_numb;
        vl_data[vl_index].integrity_check_data[network_card].first_message_received = TRUE;
        return TRUE;
    }

    /*
     * The first message is not received
     * accept any message
     */
    if (vl_data[vl_index].integrity_check_data[network_card].first_message_received == FALSE)
    {
        vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number = seq_numb;
        vl_data[vl_index].integrity_check_data[network_card].first_message_received = TRUE;
        return TRUE;
    }
    else
    {
        /* SN period ended */
        if (vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number > seq_numb)
        {
            if (vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number == 254)
            {
                if (seq_numb == 1)
                {
                    vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number = seq_numb;
                    return TRUE;
                }
            }
            
            if (vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number == 255)
            {
                if ((seq_numb == 1) || (seq_numb == 2))
                {
                    vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number = seq_numb;
                    return TRUE;
                }
            }
        }else
        {
            uint8_t diff_betw_sn = seq_numb - vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number;
            if ((diff_betw_sn == 1) || (diff_betw_sn == 2))
            {
                vl_data[vl_index].integrity_check_data[network_card].last_in_seq_number = seq_numb;
                return TRUE;
            }else
                return FALSE;
        }
    }

    return FALSE;
}


void redundancy_management_init()
{
    int i;
    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        vl_data[i].redundancy_management_data.last_accepted_seq_numb = 0;
        vl_data[i].redundancy_management_data.last_accepted_msg_time = 0;
    }
}
/*
 * 
 */
pok_bool_t redundancy_management(uint8_t seq_numb, int vl_index, pok_time_t msg_arrival_time)
{
    /* Step 1 */
    //~ pok_bool_t rm_decision = FALSE;
    int N = (vl_data[vl_index].skew_max / vl_data[vl_index].BAG) + 2;
    
    if (network_card == NETWORK_CARD_A)
    {
    #ifdef PRINT_REDUNDANCY
        printf("Compare with card B\n");
        printf("arrival_time   %lli\n", msg_arrival_time);
        printf("ar_t_in_arr    %lli\n", vl_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_B][seq_numb]);
        printf("diff           %lli\n", (msg_arrival_time - vl_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_B][seq_numb]));
        printf("skew_max       %lli\n", vl_data[vl_index].skew_max);
    #endif
        if ((msg_arrival_time - vl_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_B][seq_numb]) < vl_data[vl_index].skew_max)
        return FALSE;
    }else
    {
    #ifdef PRINT_REDUNDANCY
        printf("arrival_time   %lli\n", msg_arrival_time);
        printf("ar_t_in_arr    %lli\n", vl_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_A][seq_numb]);
        printf("diff           %lli\n", (msg_arrival_time - vl_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_A][seq_numb]));
        printf("skew_max       %lli\n", vl_data[vl_index].skew_max);

        printf("Compare with card A\n");
    #endif
        if ((msg_arrival_time - vl_data[vl_index].redundancy_management_data.arrival_time[NETWORK_CARD_A][seq_numb]) < vl_data[vl_index].skew_max)
        return FALSE;
    }

    /* Step 2 */
    if ((msg_arrival_time - vl_data[vl_index].redundancy_management_data.last_accepted_msg_time) > vl_data[vl_index].skew_max)
    {
        vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb = seq_numb;
        vl_data[vl_index].redundancy_management_data.last_accepted_msg_time = msg_arrival_time;
        return TRUE; /* Message approved */
    }else
    {
        if ((vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N) > 255)
        {
            if ((seq_numb > vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb) &&
                (seq_numb <= 255) &&
                (seq_numb >= 1) &&
                (seq_numb <= ((vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N) % 255)) )
            {
                vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb = seq_numb;
                vl_data[vl_index].redundancy_management_data.last_accepted_msg_time = msg_arrival_time;
            }
        //if ((vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N) < 255)
        }else {
            if ( (seq_numb > vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb) &&
                 (seq_numb <= (vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb + N)) )
            {
                vl_data[vl_index].redundancy_management_data.last_accepted_seq_numb = seq_numb;
                vl_data[vl_index].redundancy_management_data.last_accepted_msg_time = msg_arrival_time;  
            }
        }
    }
    
    return FALSE;

}

/*
 * This function is called by  reclaim_receive_buffers
 * and implement a process for receiving message
 */
void packet_receive(const char *buffer, size_t size)
{
    
    RETURN_CODE_TYPE ret;
    int vl_temp;
    char afdx_message[MAX_AFDX_FRAME_SIZE];
    uint8_t sequence_number;
    frame_data_t *afdx_frame = (frame_data_t *) buffer;
    pok_time_t msg_arrival_time;
    
    GET_TIME(&msg_arrival_time, &ret);
    
    //~ printf("Arrival time %lli\n", msg_arrival_time);
    printf("==========================================\n");
    if (network_card == NETWORK_CARD_A)
        printf("Works with net_card A\n");
    else
        printf("Works with net_card B\n");
    /* get payload */
    //~ strncpy(afdx_message, afdx_frame->afdx_payload, size - POK_NETWORK_OVERHEAD - sizeof(uint8_t));
    strncpy(afdx_message, afdx_frame->afdx_payload, afdx_frame->udp_header.udp_length - UDP_H_LENGTH); // RIGHT of NOT?

    /* identify vl_index in vl_data */
    vl_temp = identify_vl_index(afdx_frame->mac_header.mac_dst_addr.vl_id);

#ifdef PRINT_RECEIVE
    printf("VL_index =     %d \n", vl_temp);

    /* Just Now VL_ID's start from 1, until indexes which star from 0*/
    printf("Arinc Port #   %d\n", arinc_ports[afdx_frame->mac_header.mac_dst_addr.vl_id - 1][queuing_arinc_to_afdx_ports[vl_temp].dst_afdx_port]);
    
    printf("AFDX payload: %s\n", afdx_message);
    printf("Size of message %d \n", size);
#endif
    /* Integrity checking */
    memcpy(&sequence_number, (buffer + size - sizeof(uint8_t)), sizeof(uint8_t));

#ifdef PRINT_RECEIVE
    printf("SN =           %d\n", sequence_number);
#endif
    
    vl_data[vl_temp].redundancy_management_data.arrival_time[network_card][sequence_number] = msg_arrival_time;
    
    if (integrity_checking(sequence_number, vl_temp) == TRUE)
    {
        if (redundancy_management(sequence_number, vl_temp, msg_arrival_time)== TRUE)
            printf("RM_Message accepted\n");
        else printf("RM_Message NOT accepted\n");

    }
    else
    {
        printf("IC_Message NOT accepted\n");
    }
    printf("==========================================\n");
    /*     Some check     */
    //~ int i;
    //~ if (sequence_number == 3)
    //~ for (i = 0; i < 3; i ++)
    //~ {
       //~ printf("Time vl_0: %lli : ", vl_data[0].redundancy_management_data.arrival_time[NETWORK_CARD_A][i]);
       //~ printf("%lli\n", vl_data[0].redundancy_management_data.arrival_time[NETWORK_CARD_B][i]);
       //~ 
       //~ printf("Time vl_1: %lli : ", vl_data[1].redundancy_management_data.arrival_time[NETWORK_CARD_A][i]);
       //~ printf("%lli\n", vl_data[1].redundancy_management_data.arrival_time[NETWORK_CARD_B][i]);
    //~ }


}

/*
 * This process is the main for receiving ES
 * 
 */
static void first_process(void)
{
    RETURN_CODE_TYPE ret;

    integrity_checking_init();

    while (1) {

        /* TRY to get message from card A*/
        {
            current_netdevice = get_netdevice(ipnet_netdev_name);
            network_card = NETWORK_CARD_A;
            current_netdevice->ops->set_packet_received_callback(current_netdevice, packet_receive);
            current_netdevice->ops->reclaim_receive_buffers(current_netdevice);
        }
        
        /* TRY to get message from card B*/
        {
            ipnet_netdev_name_1 ="virtio-net1";
            current_netdevice = get_netdevice(ipnet_netdev_name_1);
            network_card = NETWORK_CARD_B;
            current_netdevice->ops->set_packet_received_callback(current_netdevice, packet_receive);
            current_netdevice->ops->reclaim_receive_buffers(current_netdevice);
        }
        
        TIMED_WAIT(SECOND, &ret);

    }
}

static int real_main(void)
{
    /* обдумать организацию */
    arinc_ports[0] = arinc_one;
    arinc_ports[1] = arinc_two;
   
    integrity_checking_init();
    redundancy_management_init();

    RETURN_CODE_TYPE ret;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };


    /*
     * create process 1
     * will read messages from network cards
     * and will filter them
    */
    process_attrs.ENTRY_POINT = first_process;
    strncpy(process_attrs.NAME, "process 1", sizeof(PROCESS_NAME_TYPE));

    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("P2_couldn't create process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("P2_process 1 created\n");
    }

    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("P2_couldn't start process 1: %d\n", (int) ret);
        return 1;
    } else {
        printf("P2_process 1 \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
    }



drivers_init();

    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns

    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("P2_couldn't transit to normal operating mode: %d\n", (int) ret);
    }

    STOP_SELF();
    return 0;
}

void main(void) {
    real_main();
    STOP_SELF();
}
