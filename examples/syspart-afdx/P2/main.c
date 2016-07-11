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

//==============================
#include <afdx/AFDX_ES_config.h>
#include <afdx/hexDump.h>
#include <afdx/AFDX_frame.h>
#include <afdx/AFDX_filling.h>

//==============================
#include <net/network.h>
#include <depl.h>
#include <port_info.h>
//------------------------------
#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>

#include <net/netdevices.h>
#include <channel_driver.h>

QUEUING_PORT_ID_TYPE QP1, QP2;

//~ static BUFFER_ID_TYPE global_buffer_id;

#define SECOND 					1000000000LL

#define MAX_AFDX_FRAME_SIZE		114
#define MAX_AFDX_PAYLOAD_SIZE 	64
#define SIZE_OF_HEADER			42
#define MAX_NB_MESSAGE 			10

//~ #define POK_NETWORK_UDP 		(14 + 20 + 8)
//~ #define POK_NETWORK_OVERHEAD 	(POK_NETWORK_UDP)

#define NETWORK_CARD_A			0
#define NETWORK_CARD_B			1

pok_netdevice_t *current_netdevice_0;
pok_netdevice_t *current_netdevice_1;
void *callback_arg;
char *ipnet_netdev_name_1;

int vl_id_index;
int array_of_indexes[10]; //the array for transmitting information to process about his VL

MESSAGE_SIZE_TYPE payload_size;
uint16_t src_arinc_port = 1;

//pok_network_buffer_callback_t callback;
void send_callback_m(void *pointer)
{
	printf("yes");
}

/*
 * This process runs continuously, receive messages and
 * fill the frame, without information interface_id and
 * send frame to the buffer for each VL
 */ 
static void first_process(void)
{
	char	afdx_payload[MAX_AFDX_PAYLOAD_SIZE];
	char	data_buffer[MAX_AFDX_FRAME_SIZE];
    RETURN_CODE_TYPE ret;
    uint16_t frame_size;
    int i = -1;
    
    while (1) {
		i++;
		if (i >= VIRTUAL_LINKS_COUNT)
		i = 0;
		if (i == 0){
        RECEIVE_QUEUING_MESSAGE(QP1, INFINITE_TIME_VALUE, (MESSAGE_ADDR_TYPE) &afdx_payload, &payload_size, &ret);
		}else{
        RECEIVE_QUEUING_MESSAGE(QP1, INFINITE_TIME_VALUE, (MESSAGE_ADDR_TYPE) &afdx_payload, &payload_size, &ret);
		}
        
        if (ret == NO_ERROR) {
			printf("Received queueing message: %s, length %d\n", afdx_payload, (int) payload_size);

			//filling the frame
			frame_data_t *afdx_frame = data_buffer;

			frame_size = fill_afdx_frame(afdx_frame, src_arinc_port, QUEUING, afdx_payload, payload_size);
			 
			SEND_BUFFER(vl_data[i].afdx_buf_id, (MESSAGE_ADDR_TYPE) afdx_frame, frame_size, 0, &ret);
			if (ret != NO_ERROR) {
				printf("couldn't send to the buffer: %d\n", (int) ret);
				break;
			} 	
            
        }else {
            printf("P2_error %d\n", (int) ret);
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
    RETURN_CODE_TYPE ret_b;
    RETURN_CODE_TYPE ret_per_w;
    MESSAGE_SIZE_TYPE len;
	char	data_buffer[MAX_AFDX_FRAME_SIZE];
	frame_data_t *afdx_frame = data_buffer;
	PROCESS_ID_TYPE proc_id;
	int i;
	
	GET_MY_ID(&proc_id, &ret_b); //добавить проверки
	i = array_of_indexes[proc_id]; // установка соответствия VL и процесса
	printf("proc_id = %d\n", (int) proc_id);
	printf("vl_id_index = %d\n", i);
   
    while (1) {
		RECEIVE_BUFFER(vl_data[i].afdx_buf_id, 0, (MESSAGE_ADDR_TYPE) afdx_frame, &len, &ret_b);

		if (ret_b == NOT_AVAILABLE)
		{
			printf("ret = NOT_AVAILABLE, buffer %s proc %d\n", vl_data[i].afdx_buf_name, (int) proc_id);
			PERIODIC_WAIT(&ret_per_w);
			continue;
			//break; if break will be here everething fals down
		}else
			if (ret_b != NO_ERROR) {
				printf("couldn'd receive from the %s: %d\n",vl_data[i].afdx_buf_name, (int) ret_b);
				break; // необходимо проверить эту ситуацию, может все упасть
			}
		
		if (ret_b == NO_ERROR)
		{	
			fill_afdx_interface_id(afdx_frame, NETWORK_CARD_A);
			printf("received message from the %s: %s\n",vl_data[i].afdx_buf_name, afdx_frame->afdx_payload);
			printf("afdx_frame size %d\n", (int) len);
			
			//проверка для ret - not done
			ipnet_netdev_name_1 ="virtio-net1";
			current_netdevice_0 = get_netdevice(ipnet_netdev_name);
			current_netdevice_1 = get_netdevice(ipnet_netdev_name_1);
			//~ printf("netdevice %p\n", current_netdevice_0);
			//~ printf("netdevice %p\n", current_netdevice_1);
			
			//send to 1 card
			printf("Sending to the card A from %s\n",vl_data[i].afdx_buf_name);
			current_netdevice_0->ops->send_frame(current_netdevice_0, data_buffer, len + POK_NETWORK_OVERHEAD, send_callback_m, NULL);
			current_netdevice_0->ops->flush_send(current_netdevice_0);
			
			//send to 2 card
			printf("Sending to the card B from %s\n", vl_data[i].afdx_buf_name);
			fill_afdx_interface_id(afdx_frame, NETWORK_CARD_B);
			current_netdevice_1->ops->send_frame(current_netdevice_1, data_buffer, len + POK_NETWORK_OVERHEAD, send_callback_m, NULL);
			current_netdevice_1->ops->flush_send(current_netdevice_1);
			
			PERIODIC_WAIT(&ret_per_w);
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
    
    // create buffer
    int i;
    for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
    {
		CREATE_BUFFER(vl_data[i].afdx_buf_name, MAX_AFDX_FRAME_SIZE, MAX_NB_MESSAGE, FIFO, &id, &ret);
		if (ret != NO_ERROR) {
			printf("error creating a %s: %d\n", vl_data[i].afdx_buf_name, (int) ret);
			return 1;
		} else {
			printf("%s successfully created\n", vl_data[i].afdx_buf_name);
		}
		vl_data[i].afdx_buf_id = id;
		printf("buffer_id = %d\n", (int) id);
	}
    
    /* 
     * create process 1 
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
     * create process 2 for each Vertual Link
     * using vl_id
     * 
     */ 
	for (i = 0; i < VIRTUAL_LINKS_COUNT; i++)
{
    process_attrs.PERIOD = SECOND;
    process_attrs.TIME_CAPACITY = SECOND / 2;
    process_attrs.ENTRY_POINT = second_process;
	
	snprintf(name, sizeof(name), "process vl %d", vl_data[i].vl_id);
	strncpy(process_attrs.NAME, name, sizeof(PROCESS_NAME_TYPE));
	printf("process_attrs.NAME = %s\n", name);
	
   
    CREATE_PROCESS(&process_attrs, &pid, &ret);
    if (ret != NO_ERROR) {
        printf("P2_couldn't create process for VL %d: %d\n", vl_data[i].vl_id ,(int) ret);
        return 1;
    } else {
        printf("P2_process for VL %d created\n", vl_data[i].vl_id);
    }
	
	array_of_indexes[pid] = i;	//try to send vl_id index to process, but no sucesses ((
	
    START(pid, &ret);
    if (ret != NO_ERROR) {
        printf("P2_couldn't start process for VL %d: %d\n", vl_data[i].vl_id, (int) ret);
        return 1;
    } else {
        printf("P2_process for VL %d \"started\" (it won't actually run until operating mode becomes NORMAL)\n", vl_data[i].vl_id);
    }
}
	
	// create ports  1
	CREATE_QUEUING_PORT("QP2", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, DESTINATION , FIFO, &QP1, &ret);
	printf("P2_QP2 = %d\n", (int) QP1);
	
	if (ret != NO_ERROR) {
        printf("P2_couldn't create port QP2, ret %d\n", (int) ret);
	}  
	// creat port 2
	CREATE_QUEUING_PORT("QP4", MAX_AFDX_PAYLOAD_SIZE, MAX_NB_MESSAGE, DESTINATION , FIFO, &QP2, &ret);
	printf("P2_QP4 = %d\n", (int) QP2);
	
	if (ret != NO_ERROR) {
        printf("P2_couldn't create port QP4, ret %d\n", (int) ret);
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
