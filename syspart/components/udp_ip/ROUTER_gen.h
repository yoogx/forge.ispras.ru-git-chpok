/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/udp_ip/config.yaml).
 */
#ifndef __ROUTER_GEN_H__
#define __ROUTER_GEN_H__

    #include "state_structs.h"
    #include "ip_addr.h"

    #include <interfaces/udp_message_handler_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct ROUTER_state {
    size_t map_ip_port_to_idx_len;
    struct udp_ip_pair map_ip_port_to_idx[10];
}ROUTER_state;

typedef struct {
    ROUTER_state state;
    struct {
            struct {
                udp_message_handler ops;
            } portA;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } *portArray;
    } out;
} ROUTER;



      ret_t receive_packet(ROUTER *, char *, size_t, uint32_t, uint16_t);

      ret_t ROUTER_call_portArray_send_by_index(int, ROUTER *, char *, size_t);






#endif
