#!/usr/bin/env python3

import sys
import yaml
import json

QUEUEING = "queueing"
SAMPLING = "sampling"

SAMPLING_PORT_TEMPLATE = """\
    {
        .header = {
            .kind = POK_PORT_KIND_SAMPLING,
            .name = %(name)s,
            .partition = %(partition)s,
            .direction = %(direction)s,
        },
        .max_message_size = %(max_message_size)s,
        .refresh = %(refresh)s,
        .data = %(data)s,
    },
"""

QUEUEING_PORT_TEMPLATE = """\
    {
        .header = {
            .kind = POK_PORT_KIND_QUEUEING,
            .name = %(name)s,
            .partition = %(partition)s,
            .direction = %(direction)s,
        },
        .max_message_size = %(max_message_size)s,
        
        .data = %(data)s,
        .data_stride = %(data_stride)s,
    },
"""

def get_port_mode(port):
    mode = port["mode"].lower()
    if mode in ("queueing", "queuing"):
        return QUEUEING
    if mode in ("sampling", ):
        return SAMPLING
    raise ValueError("%r is not valid port mode" % port["mode"])

def get_port_direction(port):
    direction = port["direction"].lower()
    if direction in ("source", "out"):
        return "POK_PORT_DIRECTION_OUT"
    if direction in ("destination", "in"):
        return "POK_PORT_DIRECTION_IN"
    raise ValueError("%r is not valid port direction" % port["direction"])

def generate_sampling_ports(ports, header, source):
    if not ports:
        return

    print("#define POK_NEEDS_PORTS_SAMPLING 1", file=header)
    print("#include <middleware/port.h>", file=source)

    for i, port in enumerate(ports):
        print("static char p%(i)d_data[sizeof(pok_port_data_t) + %(size)d];" % dict(i=i, size=port["max_message_size"]), file=source)

    print("pok_sampling_port_t sampling_ports[] = {", file=source)

    for i, port in enumerate(ports):
        print(SAMPLING_PORT_TEMPLATE % dict(
            name=json.dumps(port["name"]),
            partition=port["partition"],
            direction=get_port_direction(port),
            max_message_size=port["max_message_size"],
            refresh=port["refresh"].replace(" ", ""),
            data="&p%d_data" % i
        ), file=source)
    
    print("};", file=source)


def generate_ports(ports, header, source):
    if not ports:
        return

    sampling_ports = [port for port in ports if get_port_mode(port) == SAMPLING]

    generate_sampling_ports(sampling_ports, header, source)

def main():
    description = yaml.load(sys.stdin)
    
    # TODO argparse
    with open(sys.argv[1], "w") as header, open(sys.argv[2], "w") as source:
        generate_ports(description["ports"], header, source)

if __name__ == "__main__":
    main()
