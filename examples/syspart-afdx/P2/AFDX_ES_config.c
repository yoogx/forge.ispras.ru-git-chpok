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

 #include <stdint.h>
 #include <afdx/AFDX_ES_config.h>
 #include <afdx/AFDX_frame.h>

//~Configuration for virtual links
	vl_data_t vl_data[VIRTUAL_LINKS_COUNT] = {
		{
			.vl_id = 1,
			.BAG = SECOND,
			.L_max = 1516,
			.L_min = 62,
			.TTL = 5,
			.next_out_seq_numb = 1,
			.last_sending_BAG_numb = 0,
			.skew_max = 100000000,
			.afdx_buf_name = "afdx_buf_1",
			.afdx_buf_id = (int) NULL,
            
		},
		{
			.vl_id = 2,
			.BAG = SECOND,
			.L_max = 1516,
			.L_min = 62,
			.TTL = 5,
			.next_out_seq_numb = 1,
			.last_sending_BAG_numb = 0,
			.skew_max = 100000000,
			.afdx_buf_name = "afdx_buf_2",
			.afdx_buf_id = (int) NULL,
		}
	};

//~Configurations for ARINC ports []-number of port size == ES_QUEUING_ARINC_PORTS_COUNT
afdx_dst_info_t queuing_arinc_to_afdx_ports[ES_QUEUING_ARINC_PORTS_COUNT]= {
	{
		.src_partition_id = 1,
	    .src_afdx_port = 16001,
	    .dst_afdx_port = 16003,
	    .vl_data_index = 0,
	    .dst_partition_id = 2,
	    .type_of_packet = UNICAST_PACKET,
    },//vl 1 port 1
	{
		.src_partition_id = 1,
	    .src_afdx_port = 16002,
	    .dst_afdx_port = 16004,
	    .vl_data_index = 1,
	    .dst_partition_id = 2,
	    .type_of_packet = UNICAST_PACKET,
	}//, //vl2 port 2
	//~ {
		//~ .src_partition_id = 2,
	    //~ .src_afdx_port = 3,
	    //~ .dst_afdx_port = 1,
	    //~ .vl_data_index = 0,
	    //~ .dst_partition_id = 1,
	    //~ .type_of_packet = UNICAST_PACKET,
	 //~ }, //vl 1 port 3
	//~ {
		//~ .src_partition_id = 2,
	    //~ .src_afdx_port = 4,
	    //~ .dst_afdx_port = 2,
	    //~ .vl_data_index = 1,
	    //~ .dst_partition_id = 1,
	    //~ .type_of_packet = UNICAST_PACKET,
	 //~ }	//vl2 port 4
	};
	

// size == ES_SAMPLING_ARINC_PORTS_COUNT
afdx_dst_info_t sampling_arinc_to_afdx_ports[];
