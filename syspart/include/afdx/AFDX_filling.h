#ifndef __AFDX_FILLING_H_
#define __AFDX_FILLING_H_

#include <afdx/AFDX_ES.h>
#include <afdx/AFDX_frame.h>

uint16_t fill_afdx_frame(frame_data_t *p, uint16_t src_arinc_port, ARINC_PORT_TYPE arinc_port_type, char afdx_payload[], uint16_t payload_size);
uint16_t udp_checksum(void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr);
uint16_t ip_checksum(const void *buf, uint16_t hdr_len);
void fill_afdx_interface_id (frame_data_t *p, int x);

#endif
