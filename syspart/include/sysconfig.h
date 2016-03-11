#ifndef __POK_SYSCONFIG_H__
#define __POK_SYSCONFIG_H__

#include <arinc653/types.h>
#include <middleware/port.h>
#include <port_info.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define IP_ADDR(a,b,c,d) ((uint32_t)((a) & 0xff) << 24) | \
                         ((uint32_t)((b) & 0xff) << 16) | \
                         ((uint32_t)((c) & 0xff) << 8)  | \
                          (uint32_t)((d) & 0xff)

extern sys_sampling_port_t sys_sampling_ports[];
extern sys_queuing_port_t  sys_queuing_ports[];
extern unsigned sys_sampling_ports_nb;
extern unsigned sys_queuing_ports_nb;

typedef struct
{
    uint32_t max_message_size;
    uint32_t max_nb_messages;
    uint32_t nb_message;
    uint32_t head;
    size_t   data_stride;
    char    *data;
} queue_t;

typedef struct
{
    uint32_t max_message_size;
    void    *data;
} sample_t;


typedef struct
{
    pok_port_kinds_t kind;
    PORT_DIRECTION_TYPE direction;
    NAME_TYPE name;
    APEX_INTEGER id;
    union {
        struct {
            MESSAGE_SIZE_TYPE  max_message_size;
            MESSAGE_RANGE_TYPE max_nb_messages;
        } queueing_data;
        struct {
            MESSAGE_SIZE_TYPE  max_message_size;
        } sampling_data;
    };
} port_info_t;

enum protocol_kind {
    UDP,
//  RS232,
};

enum driver_kind {
    DRIVER_VIRTIO,
//  DRIVER_SERIAL,
};

typedef struct {
    uint32_t ip;
    uint16_t port;
} udp_data_t;

typedef struct
{
    uint32_t device_id;
    uint32_t buffer_idx; //in queues or samples arrays
    port_info_t linked_port_info;
    
    enum protocol_kind protocol;
    union {
        udp_data_t udp_data; //destination data!
    };
} link_t;

typedef struct
{
    uint32_t port_id; //in queues or samples arrays

    enum protocol_kind protocol;
    union {
        udp_data_t udp_data; //destination data!
    };
} sys_link_t;


extern unsigned sys_sampling_links_nb;
extern sys_link_t sys_sampling_links[];
extern unsigned sys_queuing_links_nb;
extern sys_link_t sys_queuing_links[];

extern unsigned sysconfig_queues_nb;
extern queue_t queues[];

extern unsigned sysconfig_samples_nb;
extern sample_t samples[];

extern unsigned sysconfig_links_nb;
extern link_t links[];

#endif
