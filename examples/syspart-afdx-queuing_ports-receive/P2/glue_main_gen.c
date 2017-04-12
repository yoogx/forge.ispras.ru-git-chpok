/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (examples/syspart-afdx-queuing_ports-receive/P2/glue_config.yaml).
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

    #include <AFDX_TIME_ADDER_gen.h>
        void __AFDX_TIME_ADDER_init__(AFDX_TIME_ADDER*);
        void __AFDX_TIME_ADDER_activity__(AFDX_TIME_ADDER*);
        AFDX_TIME_ADDER afdx_time_add_1 = {
            .instance_name = "afdx_time_add_1",

        };

    #include <AFDX_TIME_ADDER_gen.h>
        void __AFDX_TIME_ADDER_init__(AFDX_TIME_ADDER*);
        void __AFDX_TIME_ADDER_activity__(AFDX_TIME_ADDER*);
        AFDX_TIME_ADDER afdx_time_add_2 = {
            .instance_name = "afdx_time_add_2",

        };

    #include <AFDX_ROUTER_gen.h>
        void __AFDX_ROUTER_init__(AFDX_ROUTER*);
        void __AFDX_ROUTER_activity__(AFDX_ROUTER*);
            struct port_ops afdx_router_1_array_for_portArray[2];
        AFDX_ROUTER afdx_router_1 = {
            .instance_name = "afdx_router_1",
            .state = {
                .map_vl_id_to_idx_len = 2,
                .map_vl_id_to_idx = {1, 2},
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
            .instance_name = "afdx_router_2",
            .state = {
                .map_vl_id_to_idx_len = 2,
                .map_vl_id_to_idx = {1, 2},
            },

            .out = {
                .portArray = (void *)afdx_router_2_array_for_portArray,
            }
        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_ic_vl1_ntA = {
            .instance_name = "afdx_ic_vl1_ntA",
            .state = {
                .network_card = 0,
            },

        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_ic_vl2_ntA = {
            .instance_name = "afdx_ic_vl2_ntA",
            .state = {
                .network_card = 0,
            },

        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_ic_vl1_ntB = {
            .instance_name = "afdx_ic_vl1_ntB",
            .state = {
                .network_card = 1,
            },

        };

    #include <INTEGRITY_CHECKER_gen.h>
        void __INTEGRITY_CHECKER_init__(INTEGRITY_CHECKER*);
        void __INTEGRITY_CHECKER_activity__(INTEGRITY_CHECKER*);
        INTEGRITY_CHECKER afdx_ic_vl2_ntB = {
            .instance_name = "afdx_ic_vl2_ntB",
            .state = {
                .network_card = 1,
            },

        };

    #include <REDUNDANCY_MANAGER_gen.h>
        void __REDUNDANCY_MANAGER_init__(REDUNDANCY_MANAGER*);
        void __REDUNDANCY_MANAGER_activity__(REDUNDANCY_MANAGER*);
        REDUNDANCY_MANAGER red_mngr = {
            .instance_name = "red_mngr",
            .state = {
                .virtual_link_data = {{.vl_id=1, .skew_max=100000000, .BAG=100}, {.vl_id=2, .skew_max=100000000, .BAG=100}},
            },

        };

    #include <AFDX_TO_ARINC_ROUTER_gen.h>
        void __AFDX_TO_ARINC_ROUTER_init__(AFDX_TO_ARINC_ROUTER*);
        void __AFDX_TO_ARINC_ROUTER_activity__(AFDX_TO_ARINC_ROUTER*);
            struct port_ops afdx_t_arinc_rtr_array_for_portArray[2];
        AFDX_TO_ARINC_ROUTER afdx_t_arinc_rtr = {
            .instance_name = "afdx_t_arinc_rtr",
            .state = {
                .map_afdx_dst_port_vl_id_to_idx = {{1,1}, {2,2}},
                .map_afdx_dst_port_to_idx_len = 2,
            },

            .out = {
                .portArray = (void *)afdx_t_arinc_rtr_array_for_portArray,
            }
        };

    #include <ARINC_PORT_WRITER_gen.h>
        void __ARINC_PORT_WRITER_init__(ARINC_PORT_WRITER*);
        void __ARINC_PORT_WRITER_activity__(ARINC_PORT_WRITER*);
        ARINC_PORT_WRITER arinc_prt_wrtr_1 = {
            .instance_name = "arinc_prt_wrtr_1",
            .state = {
                .port_direction = SOURCE,
                .is_queuing_port = 1,
                .port_max_message_size = 64,
                .port_name = "UIN",
                .q_port_max_nb_messages = 10,
            },

        };

    #include <ARINC_PORT_WRITER_gen.h>
        void __ARINC_PORT_WRITER_init__(ARINC_PORT_WRITER*);
        void __ARINC_PORT_WRITER_activity__(ARINC_PORT_WRITER*);
        ARINC_PORT_WRITER arinc_prt_wrtr_2 = {
            .instance_name = "arinc_prt_wrtr_2",
            .state = {
                .port_direction = SOURCE,
                .is_queuing_port = 1,
                .port_max_message_size = 64,
                .port_name = "UIN",
                .q_port_max_nb_messages = 10,
            },

        };



void glue_main()
{
            __VIRTIO_NET_DEV_init__(&virtio_net_dev_1);

            __VIRTIO_NET_DEV_init__(&virtio_net_dev_2);

            __AFDX_TIME_ADDER_init__(&afdx_time_add_1);

            __AFDX_TIME_ADDER_init__(&afdx_time_add_2);

            __AFDX_ROUTER_init__(&afdx_router_1);

            __AFDX_ROUTER_init__(&afdx_router_2);

            __INTEGRITY_CHECKER_init__(&afdx_ic_vl1_ntA);

            __INTEGRITY_CHECKER_init__(&afdx_ic_vl2_ntA);

            __INTEGRITY_CHECKER_init__(&afdx_ic_vl1_ntB);

            __INTEGRITY_CHECKER_init__(&afdx_ic_vl2_ntB);

            __REDUNDANCY_MANAGER_init__(&red_mngr);

            __AFDX_TO_ARINC_ROUTER_init__(&afdx_t_arinc_rtr);

            __ARINC_PORT_WRITER_init__(&arinc_prt_wrtr_1);

            __ARINC_PORT_WRITER_init__(&arinc_prt_wrtr_2);


        virtio_net_dev_1.out.portB.ops = &afdx_time_add_1.in.portB.ops;
        virtio_net_dev_1.out.portB.owner = &afdx_time_add_1;
        afdx_time_add_1.out.portA.ops = &afdx_router_1.in.portA.ops;
        afdx_time_add_1.out.portA.owner = &afdx_router_1;
        afdx_router_1.out.portArray[0].ops = &afdx_ic_vl1_ntA.in.portA.ops;
        afdx_router_1.out.portArray[0].owner = &afdx_ic_vl1_ntA;
        afdx_router_1.out.portArray[1].ops = &afdx_ic_vl2_ntA.in.portA.ops;
        afdx_router_1.out.portArray[1].owner = &afdx_ic_vl2_ntA;
        afdx_ic_vl1_ntA.out.portB.ops = &red_mngr.in.portA.ops;
        afdx_ic_vl1_ntA.out.portB.owner = &red_mngr;
        afdx_ic_vl2_ntA.out.portB.ops = &red_mngr.in.portA.ops;
        afdx_ic_vl2_ntA.out.portB.owner = &red_mngr;
        virtio_net_dev_2.out.portB.ops = &afdx_time_add_2.in.portB.ops;
        virtio_net_dev_2.out.portB.owner = &afdx_time_add_2;
        afdx_time_add_2.out.portA.ops = &afdx_router_2.in.portA.ops;
        afdx_time_add_2.out.portA.owner = &afdx_router_2;
        afdx_router_2.out.portArray[0].ops = &afdx_ic_vl1_ntB.in.portA.ops;
        afdx_router_2.out.portArray[0].owner = &afdx_ic_vl1_ntB;
        afdx_router_2.out.portArray[1].ops = &afdx_ic_vl2_ntB.in.portA.ops;
        afdx_router_2.out.portArray[1].owner = &afdx_ic_vl2_ntB;
        afdx_ic_vl1_ntB.out.portB.ops = &red_mngr.in.portB.ops;
        afdx_ic_vl1_ntB.out.portB.owner = &red_mngr;
        afdx_ic_vl2_ntB.out.portB.ops = &red_mngr.in.portB.ops;
        afdx_ic_vl2_ntB.out.portB.owner = &red_mngr;
        red_mngr.out.portC.ops = &afdx_t_arinc_rtr.in.portC.ops;
        red_mngr.out.portC.owner = &afdx_t_arinc_rtr;
        afdx_t_arinc_rtr.out.portArray[0].ops = &arinc_prt_wrtr_1.in.portA.ops;
        afdx_t_arinc_rtr.out.portArray[0].owner = &arinc_prt_wrtr_1;
        afdx_t_arinc_rtr.out.portArray[1].ops = &arinc_prt_wrtr_2.in.portA.ops;
        afdx_t_arinc_rtr.out.portArray[1].owner = &arinc_prt_wrtr_2;

}

void glue_activity()
{
    while (1) {
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_1);
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_2);
                __AFDX_TIME_ADDER_activity__(&afdx_time_add_1);
                __AFDX_TIME_ADDER_activity__(&afdx_time_add_2);
                __AFDX_ROUTER_activity__(&afdx_router_1);
                __AFDX_ROUTER_activity__(&afdx_router_2);
                __INTEGRITY_CHECKER_activity__(&afdx_ic_vl1_ntA);
                __INTEGRITY_CHECKER_activity__(&afdx_ic_vl2_ntA);
                __INTEGRITY_CHECKER_activity__(&afdx_ic_vl1_ntB);
                __INTEGRITY_CHECKER_activity__(&afdx_ic_vl2_ntB);
                __REDUNDANCY_MANAGER_activity__(&red_mngr);
                __AFDX_TO_ARINC_ROUTER_activity__(&afdx_t_arinc_rtr);
                __ARINC_PORT_WRITER_activity__(&arinc_prt_wrtr_1);
                __ARINC_PORT_WRITER_activity__(&arinc_prt_wrtr_2);
    }

}
