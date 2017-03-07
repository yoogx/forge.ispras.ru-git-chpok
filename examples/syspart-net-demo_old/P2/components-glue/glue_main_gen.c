/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (P2/components-glue/config.yaml).
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

struct port_ops{
    void *ops;
    void *owner;
};

    #include <ARINC_SENDER_gen.h>
        void __ARINC_SENDER_init__(ARINC_SENDER*);
        void __ARINC_SENDER_activity__(ARINC_SENDER*);
        ARINC_SENDER arinc_sender_1 = {
            .state = {
                .port_direction = DESTINATION,
                .q_port_max_nb_messages = 150,
                .port_max_message_size = 4096,
                .port_name = "UOUT",
                .overhead = 42,
                .is_queuing_port = 1,
            },

        };

    #include <UDP_IP_SENDER_gen.h>
        void __UDP_IP_SENDER_init__(UDP_IP_SENDER*);
        void __UDP_IP_SENDER_activity__(UDP_IP_SENDER*);
        UDP_IP_SENDER udp_ip_sender_1 = {
            .state = {
                .src_ip = IP_ADDR(192, 168, 56, 101),
                .dst_ip = IP_ADDR(192, 168, 56, 1),
                .dst_mac = {0xc2, 0xc0, 0x31, 0xe9, 0x2c, 0xfe},
                .src_port = 10002,
                .dst_port = 10003,
            },

        };

    #include <MAC_SENDER_gen.h>
        void __MAC_SENDER_init__(MAC_SENDER*);
        void __MAC_SENDER_activity__(MAC_SENDER*);
        MAC_SENDER mac_sender_1 = {
            .state = {
                .src_mac = {0x52, 0x54, 0x00, 0x01, 0x02, 0x03},
            },

        };

    #include <VIRTIO_NET_DEV_gen.h>
        void __VIRTIO_NET_DEV_init__(VIRTIO_NET_DEV*);
        void __VIRTIO_NET_DEV_activity__(VIRTIO_NET_DEV*);
        VIRTIO_NET_DEV net_dev_1 = {
            .state = {
                .pci_fn = 0,
                .pci_dev = 2,
                .pci_bus = 0,
            },

        };

    #include <ARP_ANSWERER_gen.h>
        void __ARP_ANSWERER_init__(ARP_ANSWERER*);
        void __ARP_ANSWERER_activity__(ARP_ANSWERER*);
        ARP_ANSWERER arp_answerer_1 = {
            .state = {
                .good_ips = {IP_ADDR(192, 168, 56, 101),IP_ADDR(192, 168, 56, 102)},
                .good_ips_len = 2,
                .src_mac = {0x52, 0x54, 0x00, 0x01, 0x02, 0x03},
            },

        };

    #include <MAC_RECEIVER_gen.h>
        void __MAC_RECEIVER_init__(MAC_RECEIVER*);
        void __MAC_RECEIVER_activity__(MAC_RECEIVER*);
        MAC_RECEIVER mac_receiver_1 = {
            .state = {
                .my_mac = {0x52, 0x54, 0x00, 0x01, 0x02, 0x03},
            },

        };

    #include <UDP_RECEIVER_gen.h>
        void __UDP_RECEIVER_init__(UDP_RECEIVER*);
        void __UDP_RECEIVER_activity__(UDP_RECEIVER*);
        UDP_RECEIVER udp_receiver = {

        };

    #include <ROUTER_gen.h>
        void __ROUTER_init__(ROUTER*);
        void __ROUTER_activity__(ROUTER*);
            struct port_ops router_array_for_portArray[1];
        ROUTER router = {
            .state = {
                .map_ip_port_to_idx = {{IP_ADDR(192, 168, 56, 101), 10001}},
                .map_ip_port_to_idx_len = 1,
            },

            .out = {
                .portArray = (void *)router_array_for_portArray,
            }
        };

    #include <ARINC_RECEIVER_gen.h>
        void __ARINC_RECEIVER_init__(ARINC_RECEIVER*);
        void __ARINC_RECEIVER_activity__(ARINC_RECEIVER*);
        ARINC_RECEIVER arinc_receiver_1 = {
            .state = {
                .port_direction = SOURCE,
                .is_queuing_port = 1,
                .port_max_message_size = 4096,
                .port_name = "UIN",
                .q_port_max_nb_messages = 150,
            },

        };



void __components_init__()
{
            __ARINC_SENDER_init__(&arinc_sender_1);

            __UDP_IP_SENDER_init__(&udp_ip_sender_1);

            __MAC_SENDER_init__(&mac_sender_1);

            __VIRTIO_NET_DEV_init__(&net_dev_1);

            __ARP_ANSWERER_init__(&arp_answerer_1);

            __MAC_RECEIVER_init__(&mac_receiver_1);

            __UDP_RECEIVER_init__(&udp_receiver);

            __ROUTER_init__(&router);

            __ARINC_RECEIVER_init__(&arinc_receiver_1);


        arinc_sender_1.out.portA.ops = &udp_ip_sender_1.in.portA.ops;
        arinc_sender_1.out.portA.owner = &udp_ip_sender_1;
        udp_ip_sender_1.out.portB.ops = &mac_sender_1.in.portA.ops;
        udp_ip_sender_1.out.portB.owner = &mac_sender_1;
        mac_sender_1.out.portB.ops = &net_dev_1.in.portA.ops;
        mac_sender_1.out.portB.owner = &net_dev_1;
        net_dev_1.out.portB.ops = &mac_receiver_1.in.portA.ops;
        net_dev_1.out.portB.owner = &mac_receiver_1;
        mac_receiver_1.out.port_ARP.ops = &arp_answerer_1.in.portA.ops;
        mac_receiver_1.out.port_ARP.owner = &arp_answerer_1;
        arp_answerer_1.out.portB.ops = &mac_sender_1.in.portA.ops;
        arp_answerer_1.out.portB.owner = &mac_sender_1;
        mac_receiver_1.out.port_UDP.ops = &udp_receiver.in.portA.ops;
        mac_receiver_1.out.port_UDP.owner = &udp_receiver;
        udp_receiver.out.portB.ops = &router.in.portA.ops;
        udp_receiver.out.portB.owner = &router;
        router.out.portArray[0].ops = &arinc_receiver_1.in.portA.ops;
        router.out.portArray[0].owner = &arinc_receiver_1;

}

void __components_activity__()
{
    while (1) {
                __ARINC_SENDER_activity__(&arinc_sender_1);
                __UDP_IP_SENDER_activity__(&udp_ip_sender_1);
                __MAC_SENDER_activity__(&mac_sender_1);
                __VIRTIO_NET_DEV_activity__(&net_dev_1);
                __ARP_ANSWERER_activity__(&arp_answerer_1);
                __MAC_RECEIVER_activity__(&mac_receiver_1);
                __UDP_RECEIVER_activity__(&udp_receiver);
                __ROUTER_activity__(&router);
                __ARINC_RECEIVER_activity__(&arinc_receiver_1);
    }

}
