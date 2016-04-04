#ifndef __POK_SYS_CHANNEL_DRIVER_H__
#define __POK_SYS_CHANNEL_DRIVER_H__
typedef struct channel_driver {
    pok_bool_t (*send)(
        char *buffer,
        size_t buffer_size,
        void *driver_data,
        pok_network_buffer_callback_t callback,
        void *callback_arg
    );
    void (*reclaim_send_buffers)();
    void (*receive)();

} channel_driver_t;
#endif
