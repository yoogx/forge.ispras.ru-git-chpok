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

            .channels=%(channels)s,
            .num_channels=%(num_channels)s,
        },
        .max_message_size = %(max_message_size)s,
        .data = (void *) %(data)s,
    },
"""

SAMPLING_PORT_DATA_TEMPLATE = """\
static struct {
    pok_port_size_t message_size;
    char data[%(max_message_size)d];
} %(varname)s;
"""

QUEUEING_PORT_TEMPLATE = """\
    {
        .header = {
            .kind = POK_PORT_KIND_QUEUEING,
            .name = %(name)s,
            .partition = %(partition)s,
            .direction = %(direction)s,
            
            .num_channels=%(num_channels)s,
        },
        .max_message_size = %(max_message_size)s,
        
        .data = (void *) %(data)s,
        .data_stride = %(data_stride)s,
    },
"""

QUEUEING_PORT_DATA_TEMPALTE = """\
static struct {
    pok_port_size_t message_size;
    char data[%(max_message_size)d];
} %(varname)s[%(max_nb_message)d];
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
    print("#define POK_CONFIG_NB_SAMPLING_PORTS %d" % len(ports), file=header)
    print("#include <middleware/port.h>", file=source)
    
    port_names = [port["name"] for port in ports]

    for i, port in enumerate(ports):
        print(SAMPLING_PORT_DATA_TEMPLATE % dict(
            varname="p{}".format(i),
            max_message_size=port["max_message_size"],
        ), file=source)
        
        channels = port.get("channels", [])
        
        if channels:
            channel_dest_indices = [port_names.index(name) for name in channels]
            print("static pok_port_id_t p%dchannels[] = {%s};\n" % (i, ",".join(str(i) for i in channel_dest_indices)), file=source)

    print("pok_port_sampling_t pok_sampling_ports[] = {", file=source)


    for i, port in enumerate(ports):
        print(SAMPLING_PORT_TEMPLATE % dict(
            name=json.dumps(port["name"]),
            partition=port["partition"],
            direction=get_port_direction(port),
            max_message_size=port["max_message_size"],
            channels="p{}channels".format(i) if port.get("channels") else "NULL",
            num_channels=len(port.get("channels", [])),
            data="&p{}".format(i)
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
