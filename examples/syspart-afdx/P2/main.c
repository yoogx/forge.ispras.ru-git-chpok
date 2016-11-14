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
#include <afdx/AFDX_ES.h>
#include <afdx/AFDX_ES_config.h>
#include <afdx/hexDump.h>
#include <afdx/AFDX_frame.h>
#include <afdx/AFDX_filling.h>

/* ============================== */
#include <net/network.h>
#include <depl.h>
#include <port_info.h>

/* ============================== */
#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>

#include <net/netdevices.h>
#include <channel_driver.h>

/* ============================== */
#define PRINT_RECEIVE
#define PRINT_BUF
/* ============================== */

QUEUING_PORT_ID_TYPE QP1, QP2;

//~ static BUFFER_ID_TYPE global_buffer_id;
//~ #define MAX_AFDX_PAYLOAD_SIZE   64
//~ #define MAX_NB_MESSAGE          10
//~ #define POK_NETWORK_UDP         (14 + 20 + 8)
//~ #define POK_NETWORK_OVERHEAD    (POK_NETWORK_UDP)

#define SECOND                      1000000000LL

#define MAX_AFDX_FRAME_SIZE         114
#define SIZE_OF_HEADER              42

#define NETWORK_CARD_A              0
#define NETWORK_CARD_B              1



void *callback_arg;
char *ipnet_netdev_name_1;

int vl_id_index;
int array_of_indexes[10]; //the array for transmitting information to process about his VL

MESSAGE_SIZE_TYPE payload_size;
uint16_t src_arinc_port = 1;


// this struct consists information about ports and per VL
//~ typedef struct
//~ {
    //~ QUEUING_PORT_ID_TYPE    id;
//~ 
//~ } ports_list_t;

typedef struct
{
    uint16_t                  vl_id;            /* vl identificator */
    BUFFER_ID_TYPE            afdx_buf_id;      /* name of buffer for VL */
    uint16_t                ports_index;        /* index for checking created ports
                                                    and the amount of ports per VL */
    uint16_t                ports_count;        /* count of ports belonged to VL */
    QUEUING_PORT_ID_TYPE    *ports_list;        /* array of numbers of ports,  belonged to VL */

} vl_info_t;

vl_info_t    vl_info[VIRTUAL_LINKS_COUNT];

//pok_network_buffer_callback_t callback;
void send_callback_m(void *pointer)
{
    printf("Message is sent \n");
}

/*
 * This process runs continuously, receive messages and
 * fill the frame, without information interface_id and
 * send frame to the buffer for each VL
 */ 
static void first_process(void)
{
    char    afdx_payload[MAX_AFDX_PAYLOAD_SIZE];
    char    data_buffer[MAX_AFDX_FRAME_SIZE];
    RETURN_CODE_TYPE ret;
    uint16_t frame_size;
    int i = -1;
    int j;
    uint8_t  sequence_number = 0;

    /* data for tests */

    /* end data for tests */

    while (1) {
        i++;
        if (i >= VIRTUAL_LINKS_COUNT)
            i = 0;

        for (j = 0; j < vl_info[i].ports_count; j++)
        {
            RECEIVE_QUEUING_MESSAGE(vl_info[i].ports_list[j],
                                    INFINITE_TIME_VALUE,
                                    (MESSAGE_ADDR_TYPE) &afdx_payload,
                                    &payload_size,
                                    &ret);

            //~ printf("---%ld\n", vl_info[i].ports_list[j]);

            if (ret == NO_ERROR) {
            #ifdef PRINT_RECEIVE
                printf("Received queueing message: _%s_ , length %d\n", afdx_payload, (int) payload_size);
            #endif
                /* filling the frame */
                frame_data_t *afdx_frame = (frame_data_t *) data_buffer;
                                        /* because indexes starts from 0 unlike ports indexes starts from 1*/
                frame_size = fill_afdx_frame(afdx_frame, (vl_info[i].ports_list[j] - 1), QUEUING, afdx_payload, payload_size);

                /* adding seauence number of message */
                add_seq_numb(data_buffer, &frame_size, &sequence_number);
            #ifdef PRINT_RECEIVE
                printf("frame_size = %d\n", frame_size);
            #endif
                sequence_number++;

                /* check for SN
                 * first message after initialization has SN = 0
                 * other messages have SN from 1 to 255 */
                if (sequence_number == 0)
                    sequence_number = 1;

                SEND_BUFFER(vl_info[i].afdx_buf_id, (MESSAGE_ADDR_TYPE) afdx_frame, frame_size, 0, &ret);


                if (ret != NO_ERROR) {
                    printf("couldn't send to the buffer: %d\n", (int) ret);
                    break;
                }

            } else {
                printf("P2_error %d\n", (int) ret);
            }
        }
    }
}
/*
 * This process runs with a period equal to BAG, receive messages from 
 * the buffer and fill the Interface_id for each Net Card and
 * send frame to the cards
 */

static void second_process(void)
{
    RETURN_CODE_TYPE    ret_b;
    MESSAGE_SIZE_TYPE   len;
    PROCESS_ID_TYPE     proc_id;
    char    data_buffer[MAX_AFDX_FRAME_SIZE];
    frame_data_t        *afdx_frame = (frame_data_t *) data_buffer;

    /* test */
    char payload[MAX_AFDX_FRAME_SIZE];

    GET_MY_ID(&proc_id, &ret_b);    /* add some checks */
    
    while (1) {
        vl_id_index = array_of_indexes[proc_id];        /*matching VL and Process */
        RECEIVE_BUFFER(vl_info[vl_id_index].afdx_buf_id, 0, (MESSAGE_ADDR_TYPE) afdx_frame, &len, &ret_b);

        if (ret_b == NOT_AVAILABLE)
        {
        #ifdef PRINT_BUF
            printf("ret = NOT_AVAILABLE, buffer %s proc %d\n", vl_data[vl_id_index].afdx_buf_name, (int) proc_id); //?
        #endif
            PERIODIC_WAIT(&ret_b);

        } else if (ret_b == NO_ERROR)
        {

            /* print received payload */
            memcpy(&payload, (data_buffer + POK_NETWORK_OVERHEAD) , len - POK_NETWORK_OVERHEAD - 1);
        #ifdef PRINT_BUF
            printf("received message from the %s: %s\n", vl_data[vl_id_index].afdx_buf_name, payload);

            printf("afdx_frame size %ld\n", len);

            /*SN*/
            printf("SN_R %d\n", data_buffer[len - 1]);
        #endif
            // может быть можно вынести в 1 метод 2 отправки
            //send to 1 card
            {
                pok_netdevice_t *current_netdevice = get_netdevice(ipnet_netdev_name);
                printf("Sending to the card A from %s\n",vl_data[vl_id_index].afdx_buf_name);
                fill_afdx_interface_id(afdx_frame, NETWORK_CARD_A);
                current_netdevice->ops->send_frame(current_netdevice, data_buffer, len, send_callback_m, NULL);
                current_netdevice->ops->flush_send(current_netdevice);
                current_netdevice->ops->reclaim_send_buffers(current_netdevice);
            }

            //send to 2 card
            {
                ipnet_netdev_name_1 = "virtio-net1";
                pok_netdevice_t *current_netdevice = get_netdevice(ipnet_netdev_name_1); // можно вынести за цикл

                printf("Sending to the card B from %s\n", vl_data[vl_id_index].afdx_buf_name);
                fill_afdx_interface_id(afdx_frame, NETWORK_CARD_B);
                current_netdevice->ops->send_frame(current_netdevice, data_buffer, len, send_callback_m, NULL);
                current_netdevice->ops->flush_send(current_netdevice);
                current_netdevice->ops->reclaim_send_buffers(current_netdevice);
            }

            PERIODIC_WAIT(&ret_b);
        } else
        {
            printf("couldn'd receive from the %s: %u\n", vl_data[vl_id_index].afdx_buf_name, (int) ret_b); //???
            break;
            /* It is necessary to check this situation, everythihg falls down */
        }
    }
}

static int real_main(void)
{
    char name[20]; // name for process 
    
    RETURN_CODE_TYPE ret;
    BUFFER_ID_TYPE id;
    PROCESS_ID_TYPE pid;
    PROCESS_ATTRIBUTE_TYPE process_attrs = {
        .PERIOD = INFINITE_TIME_VALUE,
        .TIME_CAPACITY = INFINITE_TIME_VALUE,
        .STACK_SIZE = 8096, // the only accepted stack size!
        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
        .DEADLINE = SOFT,
    };
    int i;

    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        int j;
        vl_info[i].vl_id = vl_data[i].vl_id;
        vl_info[i].ports_count = 0;
        vl_info[i].ports_index = 0;
        for (j = 0; j < ES_QUEUING_ARINC_PORTS_COUNT; j++)
        {
            if (queuing_arinc_to_afdx_ports[j].vl_data_index == i)
                vl_info[i].ports_count++;
        }

    }

    /*
     * malloc memmory for arrays of ports per VL
     */
    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        vl_info[i].ports_list = malloc(vl_info[i].ports_count * sizeof(QUEUING_PORT_ID_TYPE));
    }

    /*
     * Creation of the buffer foe each VL
     */ 

    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        CREATE_BUFFER(vl_data[i].afdx_buf_name, MAX_AFDX_FRAME_SIZE, MAX_NB_MESSAGE, FIFO, &id, &ret);
        if (ret != NO_ERROR) {
            printf("error creating a %s: %d\n", vl_data[i].afdx_buf_name, (int) ret);
            return 1;
        } else {
            printf("%s successfully created\n", vl_data[i].afdx_buf_name);
        }
        vl_info[i].afdx_buf_id = id;
        printf("buffer_id = %d\n", (int) id);
    }

    /*
     * Creation of the First Process
     * will read messages from ports
     * and will send them to the buffers
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

    /*
     * Second Process created for each Virtual Link
     * using vl_id
     * with a period equal to BAG
     */ 
    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        process_attrs.PERIOD = vl_data[i].BAG;
        process_attrs.TIME_CAPACITY = vl_data[i].BAG / 2;    //???
        process_attrs.ENTRY_POINT = second_process;

        snprintf(name, sizeof(name), "process vl %d", vl_data[i].vl_id);
        strncpy(process_attrs.NAME, name, sizeof(PROCESS_NAME_TYPE));
        printf("process_attrs.NAME = %s\n", name);


        CREATE_PROCESS(&process_attrs, &pid, &ret);
        if (ret != NO_ERROR) {
            printf("P2_couldn't create process for VL %d: %d\n", vl_data[i].vl_id, (int) ret);
            return 1;
        } else {
            printf("P2_process for VL %d created\n", vl_data[i].vl_id);
        }

        array_of_indexes[pid] = i;    //try to send vl_id index to process, but no sucesses ((

        START(pid, &ret);
        if (ret != NO_ERROR) {
            printf("P2_couldn't start process for VL %d: %d\n", vl_data[i].vl_id, (int) ret);
            return 1;
        } else {
            printf("P2_process for VL %d \"started\" (it won't actually run until operating mode becomes NORMAL)\n", 
                    vl_data[i].vl_id);
        }
    }

    // create ports  1
    //create with information from depl.c and create struct in main, for dynamic information
    for (i = 0; i < config_queuing_port_list_size; i ++)
    {
        size_t index_for_vl = queuing_arinc_to_afdx_ports[i].vl_data_index;
        //~ printf("index_for_vl = %d\n", (int) index_for_vl);
        //~ printf("VL_INFO: ports_index = %d\n", (int) vl_info[index_for_vl].ports_index);


        CREATE_QUEUING_PORT(config_queuing_port_list[i].name,
                            config_queuing_port_list[i].msg_size,
                            config_queuing_port_list[i].max_nb_msg,
                            config_queuing_port_list[i].port_dir,
                            config_queuing_port_list[i].que_disc,
                            &vl_info[index_for_vl].ports_list[vl_info[index_for_vl].ports_index],
                            &ret);

        printf("CREATE QUEUING PORT:\n");
        printf("P2_%s = %ld\n", config_queuing_port_list[i].name, vl_info[index_for_vl].ports_list[vl_info[index_for_vl].ports_index]);

        if (ret != NO_ERROR) {
            printf("P2_couldn't create port %s, ret %d\n", config_queuing_port_list[i].name, (int) ret);
        }
        vl_info[index_for_vl].ports_index++;

        if (vl_info[index_for_vl].ports_index > vl_info[index_for_vl].ports_count)
            printf("ERROR\n");
    }


     drivers_init();

    // transition to NORMAL operating mode
    // N.B. if everything is OK, this never returns
    SET_PARTITION_MODE(NORMAL, &ret);

    if (ret != NO_ERROR) {
        printf("P2_couldn't transit to normal operating mode: %d\n", (int) ret);
    }

    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
        free(vl_info[i].ports_list);
    }


    STOP_SELF();
    return 0;
}

void main(void) {
    real_main();
    STOP_SELF();
}
