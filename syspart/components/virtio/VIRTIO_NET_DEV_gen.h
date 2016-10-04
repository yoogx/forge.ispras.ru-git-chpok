/*
 * GENERATED! DO NOT MODIFY!
 *
 * Instead of modifying this file, modify the one it generated from (syspart/components/virtio/config.yaml).
 */
#ifndef __VIRTIO_NET_DEV_GEN_H__
#define __VIRTIO_NET_DEV_GEN_H__

    #include "virtio_network_device.h"

    #include <interfaces/preallocated_sender_gen.h>

    #include <interfaces/message_handler_gen.h>

typedef struct VIRTIO_NET_DEV_state {
    struct virtio_network_device info;
    int dev_index;
}VIRTIO_NET_DEV_state;

typedef struct {
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

      ret_t VIRTIO_NET_DEV_call_portB_send(VIRTIO_NET_DEV *, char *, size_t);



    void virtio_init(VIRTIO_NET_DEV *);

    void virtio_receive_activity(VIRTIO_NET_DEV *);


#endif
