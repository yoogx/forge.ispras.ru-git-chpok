/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/mac/config.yaml).
 */
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



      ret_t mac_receive(MAC_RECEIVER *, const char *, size_t);

      ret_t MAC_RECEIVER_call_port_UDP_handle(MAC_RECEIVER *, const char *, size_t);
      ret_t MAC_RECEIVER_call_port_ARP_handle(MAC_RECEIVER *, const char *, size_t);






#endif
