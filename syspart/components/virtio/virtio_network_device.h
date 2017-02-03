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

#ifndef __VIRTIO_NETWORK_DEVICE_H__
#define __VIRTIO_NETWORK_DEVICE_H__

#include "virtio_virtqueue.h"
#include "virtio_net.h"
#include "virtio_pci.h"
#include <pci.h>

#define MAX_PACKET_SIZE 1500

struct receive_buffer {
    struct virtio_net_hdr virtio_net_hdr;
    char packet[MAX_PACKET_SIZE];
} __attribute__((packed));

struct send_buffer {
    char data[MAX_PACKET_SIZE];
} __attribute__((packed));

struct virtio_network_device {
    s_pci_dev pci_device;

    struct virtio_virtqueue rx_vq, tx_vq;

    uint8_t mac[6];

    void (*packet_received_callback)(const char *, size_t);

    struct receive_buffer *receive_buffers;
    struct send_buffer *send_buffers;
    int inited; //is initialized
};

#endif
