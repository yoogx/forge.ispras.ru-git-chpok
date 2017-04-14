/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/virtio/config.yaml).
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

#ifndef __VIRTIO_NET_DEV_GEN_H__
#define __VIRTIO_NET_DEV_GEN_H__

#include <memblocks.h>
    #include "virtio_network_device.h"

    #include <interfaces/preallocated_sender_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct VIRTIO_NET_DEV_state {
    struct virtio_network_device info;
    uint8_t pci_fn;
    uint8_t pci_dev;
    uint8_t pci_bus;
}VIRTIO_NET_DEV_state;

typedef struct {
    char instance_name[16];
    VIRTIO_NET_DEV_state state;
    struct {
            struct {
                preallocated_sender ops;
            } portA;
    } in;
    struct {
            struct {
                message_handler *ops;
                self_t *owner;
            } portB;
    } out;
} VIRTIO_NET_DEV;



      ret_t send_frame(VIRTIO_NET_DEV *, char *, size_t, size_t);
      ret_t flush_send(VIRTIO_NET_DEV *);

      ret_t VIRTIO_NET_DEV_call_portB_handle(VIRTIO_NET_DEV *, const char *, size_t);


 pok_ret_t VIRTIO_NET_DEV_get_memory_block_status(
         VIRTIO_NET_DEV *self,
         const char *name,
         jet_memory_block_status_t *mb_status);

    void virtio_init(VIRTIO_NET_DEV *);

    void virtio_receive_activity(VIRTIO_NET_DEV *);


#endif
