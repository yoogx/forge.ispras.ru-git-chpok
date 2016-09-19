/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (P2/components-glue/config.yaml).
 */
    #include <X_gen.h>
        void __X_init__(X*);
        void __X_activity__(X*);
        X x_1 = {
            .state = {
                .x = 1,
            }
        };
        void __X_init__(X*);
        void __X_activity__(X*);
        X x_2 = {
            .state = {
                .x = 2,
            }
        };

    #include <Y_gen.h>
        void __Y_init__(Y*);
        void __Y_activity__(Y*);
        Y y_1 = {
            .state = {
                .y = 1,
            }
        };
        void __Y_init__(Y*);
        void __Y_activity__(Y*);
        Y y_2 = {
            .state = {
                .y = 2,
            }
        };

    #include <ARINC_SENDER_gen.h>
        void __ARINC_SENDER_init__(ARINC_SENDER*);
        void __ARINC_SENDER_activity__(ARINC_SENDER*);
        ARINC_SENDER arinc_sender_1 = {
            .state = {
                .port_direction = DESTINATION,
                .q_port_max_nb_messages = 10,
                .port_max_message_size = 64,
                .port_name = "UOUT",
                .overhead = 42,
                .is_queuing_port = 1,
            }
        };

    #include <UDP_IP_SENDER_gen.h>
        void __UDP_IP_SENDER_init__(UDP_IP_SENDER*);
        void __UDP_IP_SENDER_activity__(UDP_IP_SENDER*);
        UDP_IP_SENDER udp_ip_sender_1 = {
            .state = {
                .src_ip = IP_ADDR(192, 168, 56, 101),
                .dst_ip = IP_ADDR(192, 168, 56, 1),
                .dst_mac = {0x08, 0x00, 0x27, 0x00, 0x88, 0xAD},
                .src_port = 10002,
                .dst_port = 10003,
            }
        };

    #include <MAC_SENDER_gen.h>
        void __MAC_SENDER_init__(MAC_SENDER*);
        void __MAC_SENDER_activity__(MAC_SENDER*);
        MAC_SENDER mac_sender_1 = {
            .state = {
                .src_mac = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
            }
        };

    #include <VIRTIO_NET_DEV_gen.h>
        void __VIRTIO_NET_DEV_init__(VIRTIO_NET_DEV*);
        void __VIRTIO_NET_DEV_activity__(VIRTIO_NET_DEV*);
        VIRTIO_NET_DEV virtio_net_dev_1 = {
        };



void __components_init__()
{
            __X_init__(&x_1);
            __X_init__(&x_2);

            __Y_init__(&y_1);
            __Y_init__(&y_2);

            __ARINC_SENDER_init__(&arinc_sender_1);

            __UDP_IP_SENDER_init__(&udp_ip_sender_1);

            __MAC_SENDER_init__(&mac_sender_1);

            __VIRTIO_NET_DEV_init__(&virtio_net_dev_1);


        y_1.out.portB.ops = &x_1.in.portC.ops;
        y_1.out.portB.owner = &x_1;
        y_2.out.portB.ops = &x_2.in.portC.ops;
        y_2.out.portB.owner = &x_2;
        arinc_sender_1.out.portA.ops = &udp_ip_sender_1.in.portA.ops;
        arinc_sender_1.out.portA.owner = &udp_ip_sender_1;
        udp_ip_sender_1.out.portB.ops = &mac_sender_1.in.portA.ops;
        udp_ip_sender_1.out.portB.owner = &mac_sender_1;

}

void __components_activity__()
{
    while (1) {
                __X_activity__(&x_1);
                __X_activity__(&x_2);
                __Y_activity__(&y_1);
                __Y_activity__(&y_2);
                __ARINC_SENDER_activity__(&arinc_sender_1);
                __UDP_IP_SENDER_activity__(&udp_ip_sender_1);
                __MAC_SENDER_activity__(&mac_sender_1);
                __VIRTIO_NET_DEV_activity__(&virtio_net_dev_1);
    }

}
