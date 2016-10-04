/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/mac/config.yaml).
 */
#ifndef __MAC_RECEIVER_GEN_H__
#define __MAC_RECEIVER_GEN_H__


    #include <interfaces/message_handler_gen.h>

    #include <interfaces/message_handler_gen.h>
    #include <interfaces/message_handler_gen.h>

typedef struct MAC_RECEIVER_state {
    uint8_t my_mac[6];
}MAC_RECEIVER_state;

typedef struct {
    MAC_RECEIVER_state state;
    struct {
            struct {
                message_handler ops;
            } portA;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } port_UDP;
            struct {
                message_handler *ops;
                self_t *owner;
            } port_ARP;
    } out;
} MAC_RECEIVER;



      ret_t mac_receive(MAC_RECEIVER *, char *, size_t);

      ret_t MAC_RECEIVER_call_port_UDP_send(MAC_RECEIVER *, char *, size_t);
      ret_t MAC_RECEIVER_call_port_ARP_send(MAC_RECEIVER *, char *, size_t);






#endif
