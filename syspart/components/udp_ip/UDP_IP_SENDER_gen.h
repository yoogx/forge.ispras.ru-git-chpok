/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
#ifndef __UDP_IP_SENDER_GEN_H__
#define __UDP_IP_SENDER_GEN_H__

    #include "ip_addr.h"

    #include <interfaces/send_net_data_gen.h>

    #include <interfaces/mac_send_data_gen.h>

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
                send_net_data ops;
            } portA;
    } in;
    struct {
            struct {
                mac_send_data *ops;
                self_t *owner;
            } portB;
    } out;
} UDP_IP_SENDER;



      ret_t udp_ip_send(UDP_IP_SENDER *, char *, size_t, size_t);
      ret_t udp_ip_flush(UDP_IP_SENDER *);

      ret_t UDP_IP_SENDER_call_portB_mac_send(UDP_IP_SENDER *, char *, size_t, size_t, uint8_t *, enum ethertype);
      ret_t UDP_IP_SENDER_call_portB_flush(UDP_IP_SENDER *);






#endif
