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

enum protocol_kind {
    UDP,
//  RS232,
};

typedef struct {
    uint32_t ip;
    uint16_t port;
} udp_data_t;

typedef struct
{
    uint32_t port_id;

    enum protocol_kind protocol;
    union {
        udp_data_t udp_data; //destination data!
    };
} sys_link_t;


extern unsigned sys_sampling_links_nb;
extern sys_link_t sys_sampling_links[];
extern unsigned sys_queuing_links_nb;
extern sys_link_t sys_queuing_links[];
#endif
