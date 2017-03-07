/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (examples/syspart-afdx-queuing_ports/P2/glue_config.yaml).
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

    #include <ARINC_PORT_READER_gen.h>
        void __ARINC_PORT_READER_init__(ARINC_PORT_READER*);
        void __ARINC_PORT_READER_activity__(ARINC_PORT_READER*);
        ARINC_PORT_READER arinc_port_rdr_1 = {
            .instance_name = "arinc_port_rdr_1",
            .state = {
                .port_direction = DESTINATION,
                .q_port_max_nb_messages = 10,
                .port_max_message_size = 64,
                .port_name = "UOUT",
                .prepend_overhead = 42,
                .append_overhead = 1,
                .is_queuing_port = 1,
            },

        };

    #include <AFDX_FILLER_gen.h>
        void __AFDX_FILLER_init__(AFDX_FILLER*);
        void __AFDX_FILLER_activity__(AFDX_FILLER*);
        AFDX_FILLER afdx_filler_1 = {
            .instance_name = "afdx_filler_1",
            .state = {
                .vl_id = 1,
                .type_of_packet = UNICAST_PACKET,
                .dst_partition_id = 2,
                .ttl = 5,
                .src_afdx_port = 1,
                .dst_afdx_port = 2,
                .src_partition_id = 1,
            },

        };

    #include <AFDX_QUEUE_ENQUEUER_gen.h>
        void __AFDX_QUEUE_ENQUEUER_init__(AFDX_QUEUE_ENQUEUER*);
        void __AFDX_QUEUE_ENQUEUER_activity__(AFDX_QUEUE_ENQUEUER*);
        AFDX_QUEUE_ENQUEUER afdx_enqueuer_1 = {
            .instance_name = "afdx_enqueuer_1",
            .state = {
                .BAG = SECOND,
                .max_queue_size = 10,
            },

        };

    #include <VIRTIO_NET_DEV_gen.h>
        void __VIRTIO_NET_DEV_init__(VIRTIO_NET_DEV*);
        void __VIRTIO_NET_DEV_activity__(VIRTIO_NET_DEV*);
        VIRTIO_NET_DEV virtio_net_dev_1 = {
            .instance_name = "virtio_net_dev_1",
            .state = {
                .pci_fn = 0,
                .pci_dev = 1,
                .pci_bus = 0,
            },

        };

    #include <VIRTIO_NET_DEV_gen.h>
        void __VIRTIO_NET_DEV_init__(VIRTIO_NET_DEV*);
        void __VIRTIO_NET_DEV_activity__(VIRTIO_NET_DEV*);
        VIRTIO_NET_DEV virtio_net_dev_2 = {
            .instance_name = "virtio_net_dev_2",
            .state = {
                .pci_fn = 0,
                .pci_dev = 2,
                .pci_bus = 0,
            },

        };

    #include <AFDX_STOP_gen.h>
        void __AFDX_STOP_init__(AFDX_STOP*);
        void __AFDX_STOP_activity__(AFDX_STOP*);
        AFDX_STOP afdx_stop_send_1 = {
            .instance_name = "afdx_stop_send_1",

        };



void glue_main()
{
            __ARINC_PORT_READER_init__(&arinc_port_rdr_1);

            __AFDX_FILLER_init__(&afdx_filler_1);

            __AFDX_QUEUE_ENQUEUER_init__(&afdx_enqueuer_1);

            __VIRTIO_NET_DEV_init__(&virtio_net_dev_1);

            __VIRTIO_NET_DEV_init__(&virtio_net_dev_2);

            __AFDX_STOP_init__(&afdx_stop_send_1);


        arinc_port_rdr_1.out.portA.ops = &afdx_filler_1.in.portA.ops;
        arinc_port_rdr_1.out.portA.owner = &afdx_filler_1;
        afdx_filler_1.out.portB.ops = &afdx_enqueuer_1.in.portB.ops;
        afdx_filler_1.out.portB.owner = &afdx_enqueuer_1;
        afdx_enqueuer_1.out.portNetA.ops = &virtio_net_dev_1.in.portA.ops;
        afdx_enqueuer_1.out.portNetA.owner = &virtio_net_dev_1;
        afdx_enqueuer_1.out.portNetB.ops = &virtio_net_dev_2.in.portA.ops;
        afdx_enqueuer_1.out.portNetB.owner = &virtio_net_dev_2;
        virtio_net_dev_1.out.portB.ops = &afdx_stop_send_1.in.portA.ops;
        virtio_net_dev_1.out.portB.owner = &afdx_stop_send_1;
        virtio_net_dev_2.out.portB.ops = &afdx_stop_send_1.in.portA.ops;
        virtio_net_dev_2.out.portB.owner = &afdx_stop_send_1;

}

void glue_activity()
{
    while (1) {
                __ARINC_PORT_READER_activity__(&arinc_port_rdr_1);
                __AFDX_FILLER_activity__(&afdx_filler_1);
                __AFDX_QUEUE_ENQUEUER_activity__(&afdx_enqueuer_1);
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_1);
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_2);
                __AFDX_STOP_activity__(&afdx_stop_send_1);
    }

}
