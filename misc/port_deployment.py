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
            
            .channels=%(channels)s,
            .num_channels=%(num_channels)s,
        },
        .max_message_size = %(max_message_size)d,
        .max_nb_message = %(max_nb_message)d,
        
        .data = (void *) %(data)s,
        .data_stride = %(data_stride)s,
    },
"""

QUEUEING_PORT_DATA_TEMPLATE = """\
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

def generate_channels(ports, source, prefix=""):
    port_names = [port["name"] for port in ports]

    # list of tuples (channel_ptr, nchannels)
    result = []

    for i, port in enumerate(ports):
        channels = port.get("channels", [])
        if channels:
            channel_dest_indices = [port_names.index(name) for name in channels]
            
            varname = "p{}{}channels".format(prefix, i)
            channel_string = ", ".join(str(i) for i in channel_dest_indices) 

            print("static pok_port_id_t %s[] = {%s};\n" % (varname, channel_string), file=source)

            result.append((varname, len(channels)))
        else:
            result.append(("NULL", 0))
    
    return result

def generate_sampling_ports(ports, header, source):
    if not ports:
        return

    print("#define POK_NEEDS_PORTS_SAMPLING 1", file=header)
    print("#define POK_CONFIG_NB_SAMPLING_PORTS %d" % len(ports), file=header)
    print("#include <middleware/port.h>", file=source)

    for i, port in enumerate(ports):
        print(SAMPLING_PORT_DATA_TEMPLATE % dict(
            varname="ps{}".format(i),
            max_message_size=port["max_message_size"],
        ), file=source)
       
    channels = generate_channels(ports, source, prefix="s")

    print("pok_port_sampling_t pok_sampling_ports[] = {", file=source)

    for i, port in enumerate(ports):
        print(SAMPLING_PORT_TEMPLATE % dict(
            name=json.dumps(port["name"]),
            partition=port["partition"],
            direction=get_port_direction(port),
            max_message_size=port["max_message_size"],
            channels=channels[i][0],
            num_channels=channels[i][1],
            data="&ps{}".format(i)
        ), file=source)
    
    print("};", file=source)

def generate_queueing_ports(ports, header, source):
    if not ports:
        return

    print("#define POK_NEEDS_PORTS_QUEUEING 1", file=header)
    print("#define POK_CONFIG_NB_QUEUEING_PORTS %d" % len(ports), file=header)
    print("#include <middleware/port.h>", file=source)
    
    channels = generate_channels(ports, source, prefix="q")

    if not all(channels_per_port[1] <= 1 for channels_per_port in channels):
        raise ValueError("multicasting for queueing ports isn't supported ATM")
    
    for i, port in enumerate(ports):
        print(QUEUEING_PORT_DATA_TEMPLATE % dict(
            varname="qp{}".format(i),
            max_message_size=port["max_message_size"],
            max_nb_message=port["max_nb_message"],
        ), file=source)
        
    print("pok_port_queueing_t pok_queueing_ports[] = {", file=source)

    for i, port in enumerate(ports):
        print(QUEUEING_PORT_TEMPLATE % dict(
            name=json.dumps(port["name"]),
            partition=port["partition"],
            direction=get_port_direction(port),
            max_message_size=port["max_message_size"],
            max_nb_message=port["max_nb_message"],
            channels=channels[i][0],
            num_channels=channels[i][1],
            data="&qp{}".format(i),
            data_stride="sizeof(&qp{}[0])".format(i),
        ), file=source)
    
    print("};", file=source)
def generate_ports(ports, header, source):
    if not ports:
        return

    sampling_ports = [port for port in ports if get_port_mode(port) == SAMPLING]
    queueing_ports = [port for port in ports if get_port_mode(port) == QUEUEING]

    generate_sampling_ports(sampling_ports, header, source)
    generate_queueing_ports(queueing_ports, header, source)

def main():
    description = yaml.load(sys.stdin)
    
    # TODO argparse
    with open(sys.argv[1], "w") as header, open(sys.argv[2], "w") as source:
        generate_ports(description["ports"], header, source)

if __name__ == "__main__":
    main()
