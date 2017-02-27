/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (examples/syspart-afdx-queuing_ports-receive/P2/components-glue/config.yaml).
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

    #include <VIRTIO_NET_DEV_gen.h>
        void __VIRTIO_NET_DEV_init__(VIRTIO_NET_DEV*);
        void __VIRTIO_NET_DEV_activity__(VIRTIO_NET_DEV*);
        VIRTIO_NET_DEV virtio_net_dev_1 = {
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
            .state = {
                .pci_fn = 0,
                .pci_dev = 2,
                .pci_bus = 0,
            },

        };

    #include <AFDX_TIME_ADDER_gen.h>
        void __AFDX_TIME_ADDER_init__(AFDX_TIME_ADDER*);
        void __AFDX_TIME_ADDER_activity__(AFDX_TIME_ADDER*);
        AFDX_TIME_ADDER afdx_time_adder_1 = {

        };

    #include <AFDX_TIME_ADDER_gen.h>
        void __AFDX_TIME_ADDER_init__(AFDX_TIME_ADDER*);
        void __AFDX_TIME_ADDER_activity__(AFDX_TIME_ADDER*);
        AFDX_TIME_ADDER afdx_time_adder_2 = {

        };

    #include <AFDX_ROUTER_gen.h>
        void __AFDX_ROUTER_init__(AFDX_ROUTER*);
        void __AFDX_ROUTER_activity__(AFDX_ROUTER*);
            struct port_ops afdx_router_1_array_for_portArray[2];
        AFDX_ROUTER afdx_router_1 = {
            .state = {
                .map_vl_id_to_idx_len = 2,
                .map_vl_id_to_idx = {{0}, {1}},
            },

            .out = {
                .portArray = (void *)afdx_router_1_array_for_portArray,
            }
        };

    #include <AFDX_ROUTER_gen.h>
        void __AFDX_ROUTER_init__(AFDX_ROUTER*);
        void __AFDX_ROUTER_activity__(AFDX_ROUTER*);
            struct port_ops afdx_router_2_array_for_portArray[2];
        AFDX_ROUTER afdx_router_2 = {
            .state = {
                .map_vl_id_to_idx_len = 2,
                .map_vl_id_to_idx = {{0}, {1}},
            },

            .out = {
                .portArray = (void *)afdx_router_2_array_for_portArray,
            }
        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_integrity_checker_vl_0_net_a = {
            .state = {
                .network_card = 0,
            },

        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_integrity_checker_vl_1_net_a = {
            .state = {
                .network_card = 0,
            },

        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_integrity_checker_vl_0_net_b = {
            .state = {
                .network_card = 1,
            },

        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_integrity_checker_vl_1_net_b = {
            .state = {
                .network_card = 1,
            },

        };

    #include <REDUNDANCY_MANAGER_gen.h>
        void __REDUNDANCY_MANAGER_init__(REDUNDANCY_MANAGER*);
        void __REDUNDANCY_MANAGER_activity__(REDUNDANCY_MANAGER*);
        REDUNDANCY_MANAGER redundancy_manager = {
            .state = {
                .virtual_link_data[1].vl_id = 1,
                .virtual_link_data[0].skew_max = 100000000,
                .virtual_link_data[1].skew_max = 100000000,
                .virtual_link_data[0].vl_id = 0,
                .virtual_link_data[1].BAG = 100,
                .virtual_link_data[0].BAG = 100,
            },

        };

    #include <AFDX_TO_ARINC_ROUTER_gen.h>
        void __AFDX_TO_ARINC_ROUTER_init__(AFDX_TO_ARINC_ROUTER*);
        void __AFDX_TO_ARINC_ROUTER_activity__(AFDX_TO_ARINC_ROUTER*);
            struct port_ops afdx_to_arinc_router_array_for_portArray[3];
        AFDX_TO_ARINC_ROUTER afdx_to_arinc_router = {
            .state = {
                .map_afdx_dst_port_to_idx = {0, 1, 2},
                .map_afdx_dst_port_to_idx_len = 3,
            },

            .out = {
                .portArray = (void *)afdx_to_arinc_router_array_for_portArray,
            }
        };

    #include <ARINC_PORT_WRITER_gen.h>
        void __ARINC_PORT_WRITER_init__(ARINC_PORT_WRITER*);
        void __ARINC_PORT_WRITER_activity__(ARINC_PORT_WRITER*);
        ARINC_PORT_WRITER arinc_port_writer_1 = {
            .state = {
                .port_direction = SOURCE,
                .is_queuing_port = 1,
                .port_max_message_size = 64,
                .port_name = "UIN",
                .q_port_max_nb_messages = 10,
            },

        };



void __components_init__()
{
            __VIRTIO_NET_DEV_init__(&virtio_net_dev_1);

            __VIRTIO_NET_DEV_init__(&virtio_net_dev_2);

            __AFDX_TIME_ADDER_init__(&afdx_time_adder_1);

            __AFDX_TIME_ADDER_init__(&afdx_time_adder_2);

            __AFDX_ROUTER_init__(&afdx_router_1);

            __AFDX_ROUTER_init__(&afdx_router_2);

            __INTEGRITY_CHECKER_init__(&afdx_integrity_checker_vl_0_net_a);

            __INTEGRITY_CHECKER_init__(&afdx_integrity_checker_vl_1_net_a);

            __INTEGRITY_CHECKER_init__(&afdx_integrity_checker_vl_0_net_b);

            __INTEGRITY_CHECKER_init__(&afdx_integrity_checker_vl_1_net_b);

            __REDUNDANCY_MANAGER_init__(&redundancy_manager);

            __AFDX_TO_ARINC_ROUTER_init__(&afdx_to_arinc_router);

            __ARINC_PORT_WRITER_init__(&arinc_port_writer_1);


        virtio_net_dev_1.out.portB.ops = &afdx_time_adder_1.in.portB.ops;
        virtio_net_dev_1.out.portB.owner = &afdx_time_adder_1;
        afdx_time_adder_1.out.portA.ops = &afdx_router_1.in.portA.ops;
        afdx_time_adder_1.out.portA.owner = &afdx_router_1;
        afdx_router_1.out.portArray[0].ops = &afdx_integrity_checker_vl_0_net_a.in.portA.ops;
        afdx_router_1.out.portArray[0].owner = &afdx_integrity_checker_vl_0_net_a;
        afdx_router_1.out.portArray[1].ops = &afdx_integrity_checker_vl_1_net_a.in.portA.ops;
        afdx_router_1.out.portArray[1].owner = &afdx_integrity_checker_vl_1_net_a;
        afdx_integrity_checker_vl_0_net_a.out.portB.ops = &redundancy_manager.in.portA.ops;
        afdx_integrity_checker_vl_0_net_a.out.portB.owner = &redundancy_manager;
        afdx_integrity_checker_vl_1_net_a.out.portB.ops = &redundancy_manager.in.portA.ops;
        afdx_integrity_checker_vl_1_net_a.out.portB.owner = &redundancy_manager;
        virtio_net_dev_2.out.portB.ops = &afdx_time_adder_2.in.portB.ops;
        virtio_net_dev_2.out.portB.owner = &afdx_time_adder_2;
        afdx_time_adder_2.out.portA.ops = &afdx_router_2.in.portA.ops;
        afdx_time_adder_2.out.portA.owner = &afdx_router_2;
        afdx_router_2.out.portArray[0].ops = &afdx_integrity_checker_vl_0_net_b.in.portA.ops;
        afdx_router_2.out.portArray[0].owner = &afdx_integrity_checker_vl_0_net_b;
        afdx_router_2.out.portArray[1].ops = &afdx_integrity_checker_vl_1_net_b.in.portA.ops;
        afdx_router_2.out.portArray[1].owner = &afdx_integrity_checker_vl_1_net_b;
        afdx_integrity_checker_vl_0_net_b.out.portB.ops = &redundancy_manager.in.portB.ops;
        afdx_integrity_checker_vl_0_net_b.out.portB.owner = &redundancy_manager;
        afdx_integrity_checker_vl_1_net_b.out.portB.ops = &redundancy_manager.in.portB.ops;
        afdx_integrity_checker_vl_1_net_b.out.portB.owner = &redundancy_manager;
        redundancy_manager.out.portC.ops = &afdx_to_arinc_router.in.portC.ops;
        redundancy_manager.out.portC.owner = &afdx_to_arinc_router;
        afdx_to_arinc_router.out.portArray[0].ops = &arinc_port_writer_1.in.portA.ops;
        afdx_to_arinc_router.out.portArray[0].owner = &arinc_port_writer_1;
        afdx_to_arinc_router.out.portArray[1].ops = &arinc_port_writer_1.in.portA.ops;
        afdx_to_arinc_router.out.portArray[1].owner = &arinc_port_writer_1;
        afdx_to_arinc_router.out.portArray[2].ops = &arinc_port_writer_1.in.portA.ops;
        afdx_to_arinc_router.out.portArray[2].owner = &arinc_port_writer_1;

}

void __components_activity__()
{
    while (1) {
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_1);
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_2);
                __AFDX_TIME_ADDER_activity__(&afdx_time_adder_1);
                __AFDX_TIME_ADDER_activity__(&afdx_time_adder_2);
                __AFDX_ROUTER_activity__(&afdx_router_1);
                __AFDX_ROUTER_activity__(&afdx_router_2);
                __INTEGRITY_CHECKER_activity__(&afdx_integrity_checker_vl_0_net_a);
                __INTEGRITY_CHECKER_activity__(&afdx_integrity_checker_vl_1_net_a);
                __INTEGRITY_CHECKER_activity__(&afdx_integrity_checker_vl_0_net_b);
                __INTEGRITY_CHECKER_activity__(&afdx_integrity_checker_vl_1_net_b);
                __REDUNDANCY_MANAGER_activity__(&redundancy_manager);
                __AFDX_TO_ARINC_ROUTER_activity__(&afdx_to_arinc_router);
                __ARINC_PORT_WRITER_activity__(&arinc_port_writer_1);
    }

}
