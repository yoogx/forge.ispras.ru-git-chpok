/* System partition specific */
#include <net/network.h>
#include <port_info.h>
#include <sysconfig.h>

<$for port_queueing in part.ports_queueing_system$>
static struct {
    pok_port_size_t message_size;
    char data[POK_NETWORK_UDP + {{port_queueing.max_message_size}}];
} qp_{{loop.index0}}_{{port_queueing.protocol}}_data[10];

<$endfor$>

sys_sampling_port_t sys_sampling_ports[] = {
    <$for port_sampling in part.ports_sampling_system$>
#error TODO
    <$endfor$>
};
unsigned sys_sampling_ports_nb = {{part.ports_sampling_system | length}};

sys_queuing_port_t sys_queuing_ports[] = {
    <$for port_queueing in part.ports_queueing_system$>
    {
        .header = {
            .kind = POK_PORT_KIND_QUEUEING,
            .name = "{{port_queueing.name}}",
            .direction = <$if port_queueing.is_source$>SOURCE<$else$>DESTINATION<$endif$>,
            .overhead = POK_NETWORK_{{port_queueing.protocol}},
        },
        .max_message_size = {{port_queueing.max_message_size}},
        .max_nb_messages = {{port_queueing.max_nb_message}},
        
        .data = (void *) &qp_{{loop.index0}}_{{port_queueing.protocol}}_data,
        .data_stride = sizeof(qp_{{loop.index0}}_{{port_queueing.protocol}}_data[0]),
    },
    <$endfor$>
};

unsigned sys_queuing_ports_nb = {{part.ports_queueing_system | length}};
