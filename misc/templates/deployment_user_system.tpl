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

/* System partition specific */
#include <net/network.h>
#include <port_info.h>
#include <depl.h>

{%for port_queueing in part.ports_queueing_system%}
static struct {
    pok_port_size_t message_size;
    char data[POK_NETWORK_UDP + {{port_queueing.max_message_size}}];
} qp_{{loop.index0}}_{{port_queueing.protocol}}_data;

{%endfor%}

sys_sampling_port_t sys_sampling_ports[] = {
    {%for port_sampling in part.ports_sampling_system%}
#error TODO
    {%endfor%}
};
unsigned sys_sampling_ports_nb = {{part.ports_sampling_system | length}};

sys_queuing_port_t sys_queuing_ports[] = {
    {%for port_queueing in part.ports_queueing_system%}
    {
        .header = {
            .name = "{{port_queueing.name}}",
            .direction = {%if port_queueing.is_src()%}SOURCE{%else%}DESTINATION{%endif%},
            .overhead = POK_NETWORK_{{port_queueing.protocol}},
        },
        .max_message_size = {{port_queueing.max_message_size}},
        .max_nb_messages = {{port_queueing.max_nb_message}},

        .data = (void *) &qp_{{loop.index0}}_{{port_queueing.protocol}}_data,
    },
    {%endfor%}
};

unsigned sys_queuing_ports_nb = {{part.ports_queueing_system | length}};
