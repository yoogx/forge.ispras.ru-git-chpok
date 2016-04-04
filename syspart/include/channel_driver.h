#ifndef __POK_SYS_CHANNEL_DRIVER_H__
#define __POK_SYS_CHANNEL_DRIVER_H__
#include <net/network.h>
typedef struct channel_driver {
    pok_bool_t (*send)(
        char *buffer,
        size_t buffer_size,
        void *driver_data,
        pok_network_buffer_callback_t callback,
        void *callback_arg
    );

    /* will call callbacks on sent packets */
    void (*reclaim_send_buffers)();

    void (*register_received_callback)(
            pok_bool_t (*callback)(
                uint32_t ip,
                uint16_t port,
                const char *payload,
                size_t length
                )
            );

    /* will call callbacks on received packets */
    void (*receive)();

    /* We finished sending portion, so driver MAY flush send buffers */
    void (*flush_send)();


} channel_driver_t;
#endif
