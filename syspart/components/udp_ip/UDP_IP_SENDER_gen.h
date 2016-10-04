/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
#ifndef __UDP_IP_SENDER_GEN_H__
#define __UDP_IP_SENDER_GEN_H__

    #include "ip_addr.h"

    #include <interfaces/preallocated_sender_gen.h>

    #include <interfaces/ethernet_packet_sender_gen.h>

typedef struct UDP_IP_SENDER_state {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t dst_mac[6];
}UDP_IP_SENDER_state;

typedef struct {
    UDP_IP_SENDER_state state;
    struct {
            struct {
                preallocated_sender ops;
            } portA;
    } in;
    struct {
            struct {
                ethernet_packet_sender *ops;
                self_t *owner;
            } portB;
    } out;
} UDP_IP_SENDER;



      ret_t udp_ip_send(UDP_IP_SENDER *, char *, size_t, size_t);
      ret_t udp_ip_flush(UDP_IP_SENDER *);

      ret_t UDP_IP_SENDER_call_portB_mac_send(UDP_IP_SENDER *, char *, size_t, size_t, uint8_t *, enum ethertype);
      ret_t UDP_IP_SENDER_call_portB_flush(UDP_IP_SENDER *);






#endif
