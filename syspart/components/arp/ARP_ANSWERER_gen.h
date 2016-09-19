/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/arp/config.yaml).
 */
#ifndef __ARP_ANSWERER_GEN_H__
#define __ARP_ANSWERER_GEN_H__


    #include <interfaces/simple_send_data_gen.h>

    #include <interfaces/mac_send_data_gen.h>

typedef struct ARP_ANSWERER_state {
    uint8_t src_mac[6];
    uint32_t good_ips_len;
    uint32_t good_ips[10];
}ARP_ANSWERER_state;

typedef struct {
    ARP_ANSWERER_state state;
    struct {
            struct {
                simple_send_data ops;
            } portA;
    } in;
    struct {
            struct {
                mac_send_data *ops;
                self_t *owner;
            } portB;
    } out;
} ARP_ANSWERER;



      ret_t arp_receive(ARP_ANSWERER *, char *, size_t);

      ret_t ARP_ANSWERER_call_portB_mac_send(ARP_ANSWERER *, char *, size_t, size_t, uint8_t *, enum ethertype);
      ret_t ARP_ANSWERER_call_portB_flush(ARP_ANSWERER *);






#endif
