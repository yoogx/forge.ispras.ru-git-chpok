/*
 * Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __POK_NET_NETWORK_H__
#define __POK_NET_NETWORK_H__

#include <types.h>

#if 1
#define NETWORK_DRIVER pok_network_virtio_device
#define NETWORK_DRIVER_OPS (pok_network_virtio_device.ops)
#elif 0
#define NETWORK_DRIVER pok_network_ne2000_device
#define NETWORK_DRIVER_OPS (pok_network_ne2000_device.ops)
#elif 1
#define NETWORK_DRIVER pok_network_p3041_device
#define NETWORK_DRIVER_OPS (pok_network_p3041_device.ops)
#endif

// ethernet + ip + udp
#define POK_NETWORK_OVERHEAD_PROTO (14+20+8)
#define POK_NETWORK_OVERHEAD (POK_NETWORK_OVERHEAD_PROTO)

typedef void (*pok_network_buffer_callback_t)(void*);

struct send_callback {
    pok_network_buffer_callback_t func;
    void *argument;
};


// a linked list of callbacks, actually
typedef struct pok_network_udp_receive_callback_t {
    pok_bool_t (*callback)(uint32_t ip, uint16_t port, const char *payload, size_t length);
    struct pok_network_udp_receive_callback_t *next;
} pok_network_udp_receive_callback_t;

typedef struct {
    char *buffer;
    size_t size;
} pok_network_sg_list_t;

//XXX maybe pok_bool_t is a bad choice
typedef struct {
    pok_bool_t (*send_frame)(char *buffer,
                             size_t size,
                             pok_network_buffer_callback_t callback,
                             void *callback_arg);

    pok_bool_t (*send_frame_gather)(const pok_network_sg_list_t *sg_list,
                                    size_t sg_list_len,
                                    pok_network_buffer_callback_t callback,
                                    void *callback_arg);

    void (*set_packet_received_callback)(void (*f)(const char *, size_t));

    void (*reclaim_send_buffers)(void);
    void (*reclaim_receive_buffers)(void);

    void (*flush_send)(void);
} pok_network_driver_ops_t;

typedef struct {
    uint8_t *mac;
    const pok_network_driver_ops_t *ops;
} pok_network_driver_device_t;

/*
 * Must be called early when the kernel is
 * being initialized.
 */ 
void pok_network_init(void);

/*
 * Send buffer to destination.
 *
 * It's serialized as an UDP packet.
 *
 * Buffer must contain (possibly unitialized) POK_NETWORK_OVERHEAD bytes,
 * followed by 'size' bytes.
 * 
 * Once buffer is used by the driver (sent to the network),
 * callback runs.
 *
 * Buffer CANNOT BE touched until callback runs.
 *
 * Returns true if send was (more or less) successful.
 */
pok_bool_t pok_network_send_udp(
    char *buffer,
    size_t size,
    uint32_t dst_ip,
    uint16_t dst_port,
    pok_network_buffer_callback_t callback,
    void *callback_arg
);

pok_bool_t pok_network_send_udp_gather(
    const pok_network_sg_list_t *sg_list,
    size_t sg_list_len,
    uint32_t dst_ip,
    uint16_t dst_port,
    pok_network_buffer_callback_t callback,
    void *callback_arg
);

void pok_network_register_udp_receive_callback(
    pok_network_udp_receive_callback_t *cb
);

/*
 * Reclaim buffers used for pok_network_send,
 * running callbacks associated with them.
 *
 * This operation is very cheap, at least in virtio driver.
 */
void pok_network_reclaim_send_buffers(void);

/*
 * Reclaim used (internal) receive buffers,
 * processing them as necessary.
 *
 * This operation may take some time, depending on
 * amount of incoming packets.
 */
void pok_network_reclaim_receive_buffers(void);

void pok_network_reclaim_buffers(void);

/*
 * Must be called after a bunch of packets
 * have been queued for sending.
 *
 * If you forget to do that, their transmission
 * might be postponed indefinitely.
 */
void pok_network_flush_send(void);

/*
 * Network thread entry.
 */
void pok_network_thread(void);

/*
 * Defined in deployment.c
 */
extern const uint32_t pok_network_ip_address;

#endif
