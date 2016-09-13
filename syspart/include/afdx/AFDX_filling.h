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

#ifndef __AFDX_FILLING_H_
#define __AFDX_FILLING_H_

#include <afdx/AFDX_ES.h>
#include <afdx/AFDX_frame.h>

uint16_t fill_afdx_frame(frame_data_t *p, uint16_t src_arinc_port, ARINC_PORT_TYPE arinc_port_type, char afdx_payload[], uint16_t payload_size);
uint16_t udp_checksum(void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr);
uint16_t ip_checksum(const void *buf, uint16_t hdr_len);
void fill_afdx_interface_id (frame_data_t *p, int x);
void add_seq_numb(void * buf, uint16_t * size, uint8_t * number);

#endif
