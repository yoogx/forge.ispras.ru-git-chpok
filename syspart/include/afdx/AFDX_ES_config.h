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
*                   AFDX End-System configuration
*
* The following file is a part of the AFDX project. Any modification should
* made according to the AFDX standard.
*
*
* Created by ....
*/

#ifndef __AFDX_ES_CONFIG_H_
#define __AFDX_ES_CONFIG_H_

#include <afdx/AFDX_ES.h>
#include <stdlib.h>


#define SUBNETWORKS_COUNT  2
#define VIRTUAL_LINKS_COUNT 2						//max(65535)
//#define ARRAY_SIZE(sys_queuing_ports) 4
//#define ARRAY_SIZE(sys_sampling_ports) 4
#define ES_QUEUING_ARINC_PORTS_COUNT 4	//ARRAY_SIZE(sys_queuing_ports)
#define ES_SAMPLING_ARINC_PORTS_COUNT 4	//ARRAY_SIZE(sys_sampling_ports)
#define DOMAIN_ID		0xA	//The 0000 and 1111 are forbidden values. 
							//Will be specified for each hosting equipment

#define SIDE_ID			0x4	//The 000 and 111 are forbidden values. 
							//Will be specified for each hosting equipment

#define LOCATION_ID		0x5	//The 00000 and 11111 are forbidden values.
							//Will be specified for each hosting equipment





/*
 * This structure (vl_data_t) describes parameters for each Virtual Link
 * static parameters:
 * vl_id    -Virtual Link identificator
 * BAG      -time to send the message (128ms)
 * L_max    -Maximum packet length
 * L_min    -Minimum packet length
 * TTL      -time to live (the maximum number of nodes traversed by the message)
 *
 * dynamic parameters:
 * next_out_seq_number    	-the sequence number of the outgoing message
 *   The frame sequence number should be one octet long with a range of 0 to 255.
 * last_sending_BAG_number  -the number of the BAG, in which last message was sent
 * integrity_check          -struct of arrays which consist information for integrity check
 * skew_max					-is given by configuration per VL, shows the difference
 * 							between time of subnetworks
 */
typedef struct
{
    const uint16_t          vl_id;
    const pok_time_t        BAG;
    const uint16_t          L_max;
    const uint16_t          L_min;
    const uint8_t           TTL;
    uint8_t                 next_out_seq_numb;
    uint64_t                last_sending_BAG_numb;
    const pok_time_t        skew_max;

/*
 * this data is needed for receive ES
 */
//    integrity_check_data_t  integrity_check_data[SUBNETWORKS_COUNT];
//	redundancy_management_data_t	redundancy_management_data;

} vl_data_t;

/********************************************/
/*
 * This structure  describes packeges types
 * The packet may be unicast or multicast
 */
typedef enum
{
   UNICAST_PACKET       = 0,
   MULTICAST_PACKET     = 1,
} PACKET_TYPE;

/********************************************/
/*
 * This structure  describes ports types
 * The packet may be sampling or queuing
 */
typedef enum
{
	QUEUING,
    SAMPLING,
} ARINC_PORT_TYPE;


/*
 * This structure (afdx_dst_info_t) describes a statically AFDX ports for system
 * Source:
 * src_partition_id     -the number of the source partition
 * src_afdx_port        -the number of the source AFDX port
 * Destination:
 * dst_afdx_port        -the number of the destination AFDX port
 * vl_data_index        -the index of vl in static and dynamic arrays
 * dst_partition_id     -destination partition id
 * type_of_packet       -the type of packege
 *
 */
typedef struct
{
    uint8_t         src_partition_id;
    uint16_t        src_afdx_port;
    uint16_t        dst_afdx_port;
    size_t          vl_data_index;
    uint8_t         dst_partition_id;
    PACKET_TYPE     type_of_packet;
} afdx_dst_info_t;


extern vl_data_t vl_data[];	// the size of array: VIRTUAL_LINKS_COUNT
extern afdx_dst_info_t queuing_arinc_to_afdx_ports[]; // the size of array: ES_QUEUING_ARINC_PORTS_COUNT
extern afdx_dst_info_t sampling_arinc_to_afdx_ports[];// the size of array: ES_SAMPLING_ARINC_PORTS_COUNT

#endif
