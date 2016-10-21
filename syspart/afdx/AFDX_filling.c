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
*                   Function that fill the frame 
*
* The following file is a part of the AFDX project. Any modification should
* made according to the AFDX standard.
*
*
* Created by ....
*/

 #include <stdint.h>
 #include <afdx/AFDX_ES_config.h>
 #include <afdx/AFDX_filling.h>
 #include <net/byteorder.h>
 #include <string.h>
 #include <stdio.h>
 #include <afdx/hexDump.h>
 
#define MAC_CONST_DST 		0x3000000
#define MAC_CONST_SRC_1 	0x2
#define MAC_CONST_SRC_2 	0
#define MAC_CONST_SRC_3 	0
#define MAC_NETWOK_ID		0
#define MAC_INTERFACE_ID_A	0x1	// (001 b) for the network A
#define MAC_INTERFACE_ID_B	0x2	// (010 b) for the network B						
								
#define MAC_INTERFACE_ID_2	0
#define CONNECTION_TYPE		0x8000
#define IP_CONST_SRC 		0xA
#define IP_NETWOK_ID		0x8
#define IP_PARTITION_ID_1	0x0
#define IP_CONST_DST		0xE0E0

#define IP_VERSION			0x4
#define TYPE_OF_SERVICE 	0x00 		//in most IP protocols 0
#define IPPROTO_UDP  		0x11    	// UDP
#define FRAGMENT_ID			0
#define IHL					0x5		// 20 bytes IP Header

#define HEADER_LENGTH		42
#define UDP_H_LENGTH        8
#define IP_H_LENGTH         (IHL * 4) + UDP_H_LENGTH
#define FCS_LENGTH			4

#define MIN_PAYLOAD_SIZE    17

/*
 * UDP (User Datagram Protocol) packet type.
 * filling depends on bytes order
*/
void set_mac_addr_const_part(uint8_t x[3])
{	
#ifdef BIG_ENDIAN 
		x[0] = MAC_CONST_SRC_1;
		x[1] = MAC_CONST_SRC_2;
		x[2] = MAC_CONST_SRC_3;
		
#else 
		x[2] = MAC_CONST_SRC_1;
		x[1] = MAC_CONST_SRC_2;
		x[0] = MAC_CONST_SRC_3;
#endif
}

/*
 * This function takes the number of src arinc port and its type
 *  and return a pointer to structure of ports
 */
 //rename
afdx_dst_info_t* find_suitable_struct(uint16_t src_arinc_port, ARINC_PORT_TYPE arinc_port_type)
{
	if (arinc_port_type == SAMPLING)
		return (&(sampling_arinc_to_afdx_ports[src_arinc_port]));
	else
		return (&(queuing_arinc_to_afdx_ports[src_arinc_port]));
}

/*
 * This function fill the afdx frame without information of interface id.
 * The nformation of interface id will fill another function, which will send 
 * packets to the network cards
 */
uint16_t fill_afdx_frame(frame_data_t *p,
						 uint16_t src_arinc_port,
						 ARINC_PORT_TYPE arinc_port_type,
						 char afdx_payload[],
						 uint16_t payload_size)
{
    uint8_t pad_size = 0;    
    size_t vl_data_index = find_suitable_struct(src_arinc_port, arinc_port_type)->vl_data_index;
    
	afdx_dst_info_t *arinc_to_afdx_port =  find_suitable_struct(src_arinc_port, arinc_port_type);
//MAC
	p->mac_header.mac_dst_addr.mac_const_dst = hton32(MAC_CONST_DST);
	p->mac_header.mac_dst_addr.vl_id = hton16(vl_data[vl_data_index].vl_id);
	set_mac_addr_const_part(p->mac_header.mac_src_addr.mac_const_src);
	
	p->mac_header.mac_src_addr.network_id = (MAC_NETWOK_ID << 4 | DOMAIN_ID);
	p->mac_header.mac_src_addr.equipment_id = (SIDE_ID << 5 | LOCATION_ID);
	p->mac_header.mac_src_addr.interface_id = 0;
	p->mac_header.connection_type = hton16(CONNECTION_TYPE);
//IP
	p->ip_header.version_and_ihl = (IP_VERSION << 4 | IHL);
	p->ip_header.type_of_service = TYPE_OF_SERVICE;
	p->ip_header.total_length = hton16(payload_size + IP_H_LENGTH);	//total length = (payload size + IP header size) 
	p->ip_header.fragment_id = hton16(FRAGMENT_ID);
	p->ip_header.flag_and_fragment_offset = hton16(0x2 << 13 | 0x2); // flag = 0x2 -DO not fragment 
																	//fragment_offset = 0
	p->ip_header.ttl = vl_data[vl_data_index].TTL;
	p->ip_header.protocol = IPPROTO_UDP;
	p->ip_header.header_checksum = hton16(ip_checksum(&p->ip_header, payload_size + IP_H_LENGTH));
	//src
	p->ip_header.u_src_addr.ip_src_addr.ip_const_src = IP_CONST_SRC;
	p->ip_header.u_src_addr.ip_src_addr.network_id = (IP_NETWOK_ID << 4 | DOMAIN_ID);
	p->ip_header.u_src_addr.ip_src_addr.equipment_id = (SIDE_ID << 5 | LOCATION_ID);
	//
	p->ip_header.u_src_addr.ip_src_addr.partition_id = (IP_PARTITION_ID_1 << 3 | arinc_to_afdx_port->src_partition_id);
	//dst??
    // !
	if (arinc_to_afdx_port->type_of_packet == UNICAST_PACKET)
	{
		p->ip_header.u_dst_addr.ip_unicast_dst_addr.ip_const_src = IP_CONST_SRC;
		p->ip_header.u_dst_addr.ip_unicast_dst_addr.network_id = (IP_NETWOK_ID << 4 | DOMAIN_ID);
		p->ip_header.u_dst_addr.ip_unicast_dst_addr.equipment_id = (SIDE_ID << 5 | LOCATION_ID);
		//
		p->ip_header.u_dst_addr.ip_unicast_dst_addr.partition_id = (IP_PARTITION_ID_1 << 3 | arinc_to_afdx_port->dst_partition_id);
	}else
	{
		p->ip_header.u_dst_addr.ip_multicast_dst_addr.ip_const_dst = hton16(IP_CONST_DST);
		p->ip_header.u_dst_addr.ip_multicast_dst_addr.vl_id =  hton16(vl_data[vl_data_index].vl_id);
	}
//UDP
		//
		p->udp_header.afdx_src_port = hton16(arinc_to_afdx_port->src_afdx_port);
		p->udp_header.afdx_dst_port = hton16(arinc_to_afdx_port->dst_afdx_port);
		//
	p->udp_header.udp_length = hton16(payload_size + UDP_H_LENGTH); //udp_length = (payload size + udp header size)
	p->udp_header.udp_checksum = hton16(udp_checksum(&p->udp_header,
													payload_size + UDP_H_LENGTH,
													p->ip_header.u_src_addr.ip_general_src_addr,
													p->ip_header.u_dst_addr.ip_general_dst_addr));
													
	//переписал стуктуру кадра добавив в юнионы uint32_t для того, чтобы использовать их потом в udp_checksum  
	
//PAYLOAD
	//uint16_t k;
	memcpy(p->afdx_payload, afdx_payload, payload_size);
	
    if (payload_size < MIN_PAYLOAD_SIZE)
    {
        memcpy((p->afdx_payload + payload_size), 0, pad_size);
    }
            
    //scp ifg?
	if (pad_size == 0)
        return (payload_size + HEADER_LENGTH);  //FCS_LENGTH
    else
        return (payload_size + pad_size + HEADER_LENGTH);
}
void fill_afdx_interface_id (frame_data_t *p, int net_card)
{
	if (net_card == 0)
	p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_A << 5 | MAC_INTERFACE_ID_2);
	else 
	p->mac_header.mac_src_addr.interface_id = (MAC_INTERFACE_ID_B << 5 | MAC_INTERFACE_ID_2);
}	

/* Calculate the UDP checksum (calculated with the whole packet).
 * \param buff The UDP packet.
 * \param len The UDP packet length. (header + payload)
 * \param src_addr The IP source address (in network format).
 * \param dest_addr The IP destination address (in network format).
 * \return The result of the checksum.
 */
uint16_t udp_checksum(void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr)    //uint_32
{		
        //hexDump("udp_checksum", buff, len);
        //printf("");
        
        const uint16_t *buf=buff;
        uint16_t *ip_src=(void *)&src_addr;
        uint16_t *ip_dst=(void *)&dest_addr;
        uint32_t sum;
        size_t length=len;

        // Calculate the sum
        sum = 0;
        while (len > 1)
        {
			sum += *buf++;
            if (sum & 0x80000000)
                        sum = (sum & 0xFFFF) + (sum >> 16);
            len -= 2;
         }
 
         if ( len & 1 )
                // Add the padding if the packet lenght is odd
                sum += *((uint8_t *)buf);
        //~ printf("sum_1 %u\n", sum);
         // Add the pseudo-header
         sum += *(ip_src++);
        //~ printf("sum_2 %u\n", sum);
         sum += *ip_src;
        //~ printf("sum_3 %u\n", sum);
         sum += *(ip_dst++);
        //~ printf("sum_4 %u\n", sum); 
         sum += *ip_dst;
        //~ printf("sum_5 %u\n", sum);
         sum += hton16(IPPROTO_UDP);
        //~ printf("sum_6 %u\n", sum);
         sum += hton16(length);
        //~ printf("sum_7 %u\n", sum);
         // Add the carries
         while (sum >> 16)
                 sum = (sum & 0xFFFF) + (sum >> 16);

         // Return the one's complement of sum
         return ( (uint16_t)(~sum)  );
}
/*
 * brief Calculate the IP header checksum.
 * param buf The IP header content.
 * param hdr_len The IP header length.
 * return The result of the checksum.
 */
uint16_t ip_checksum(const void *buf, uint16_t hdr_len)
{
        unsigned long sum = 0;
        const uint16_t *ip1;

        ip1 = buf;
        while (hdr_len > 1)
        {
                sum += *ip1++;
                if (sum & 0x80000000)
                        sum = (sum & 0xFFFF) + (sum >> 16);
                hdr_len -= 2;
        }

        while (sum >> 16)
                sum = (sum & 0xFFFF) + (sum >> 16);
 
        return(~sum);
}

/*
 * This function adds SN at the end of Payload
 * and increases frame_size by 1
 */

void add_seq_numb(void * buf, uint16_t * f_size, uint8_t * s_number)
{
    memcpy((buf + (*f_size)), s_number, sizeof(uint8_t));
    *f_size = *f_size + (uint16_t)(sizeof(uint8_t));   
}

 
