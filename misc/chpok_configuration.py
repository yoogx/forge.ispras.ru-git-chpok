#  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
This file defines classes and functions that can be used to
simplify generation of POK kernel configuration.

"""

import os
import json
import functools
import collections

class TimeSlot:
    def __init__(self, duration, partition):
        self.duration = duration
        self.partition = partition

class Partition:
    __slots__ = [
        "size", # allocated RAM size in bytes (code + static storage)
        "num_threads", # number of user threads, _not_ counting init thread
        "ports", # list of ports

        "num_arinc653_buffers",
        "num_arinc653_blackboards",
        "num_arinc653_semaphores",
        "num_arinc653_events",

        "buffer_data_size", # bytes allocated for buffer data
        "blackboard_data_size", # same, for blackboards

        "hm_table", # partition hm table
    ]

    def __init__(self):
        self.hm_table = []

    def get_all_ports(self):
        return list(self.ports)

    def get_all_sampling_ports(self):
        return [port for port in self.ports if isinstance(port, SamplingPort)]

    def get_all_queueing_ports(self):
        return [port for port in self.ports if isinstance(port, QueueingPort)]

    def validate(self):
        # validation is by no means complete
        # it's just basic sanity check

        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))

        for port in self.ports:
            port.validate()

    def get_needed_lock_objects(self):
        # all of them implicitly require a lock object
        return (
            len(self.ports) + 
            self.num_arinc653_buffers +
            self.num_arinc653_blackboards +
            self.num_arinc653_semaphores +
            self.num_arinc653_events
        )

    def get_needed_threads(self):
        return (
            1 + # init thread
            self.num_threads
        )

    def get_port_by_name(self, name):
        res = next((port for port in self.get_all_ports() if port.name == name), None)
        if not res:
            raise ValueError("no such named port %r in %r" % (name, self))
        return res

def _get_port_direction(port):
    direction = port.direction.lower()
    if direction in ("source", "out"):
        return "POK_PORT_DIRECTION_OUT"
    if direction in ("destination", "in"):
        return "POK_PORT_DIRECTION_IN"
    raise ValueError("%r is not valid port direction" % port["direction"])


class SamplingPort:
    __slots__ = [
        "name",
        "direction",
        "max_message_size",
        "refresh",
    ]

    def validate(self):
        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))

class QueueingPort:
    __slots__ = [
        "name",
        "direction",
        "max_nb_messages",
        "max_message_size",
    ]

    def validate(self):
        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))

class PortChannel:
    def __init__(self, src, dst):
        self.src = src
        self.dst = dst

class Configuration:

    __slots__ = [
        "partitions", 
        "slots", # time windows
        "channels", # queueing and sampling port channels (connections)

        # if this is set, POK writes a special string once 
        # there are no more schedulable threads
        # it's used by test runner as a sign that POK
        # can be terminated
        "test_support_print_when_all_threads_stopped",  
    ]

    def __init__(self):
        self.partitions = []
        self.slots = []
        self.channels = []

        self.test_support_print_when_all_threads_stopped = False

    def get_all_ports(self):
        return sum((part.get_all_ports() for part in self.partitions), [])

    def get_all_sampling_ports(self):
        return sum((part.get_all_sampling_ports() for part in self.partitions), [])

    def get_all_queueing_ports(self):
        return sum((part.get_all_queueing_ports() for part in self.partitions), [])

    def get_port_by_name_and_partition(self, partition_idx, port_name):
        return self.partitions[partition_idx].get_port_by_name(port_name)

    def validate(self):
        for part in self.partitions:
            part.validate()

COMMON_KERNEL_DEFINES = """\
#define POK_NEEDS_LOCKOBJECTS  1
#define POK_NEEDS_THREADS      1
#define POK_NEEDS_PARTITIONS   1
#define POK_NEEDS_SCHED        1
#define POK_NEEDS_TIME         1
#define POK_NEEDS_GETTICK      1
#define POK_NEEDS_DEBUG        1
#define POK_NEEDS_SERIAL       1
#define POK_NEEDS_CONSOLE      1
#define POK_NEEDS_ERROR_HANDLING 1
#define POK_NEEDS_THREAD_SUSPEND 1
#define POK_NEEDS_THREAD_SLEEP 1
#define POK_NEEDS_THREAD_ID 1

""" 

COMMON_PARTITION_DEFINES = """\
#define POK_NEEDS_TIMER 1
#define POK_NEEDS_THREADS 1

#define POK_NEEDS_PARTITIONS 1

#define POK_NEEDS_DEBUG 1
#define POK_NEEDS_CONSOLE 1

#define POK_NEEDS_LIBC_STDLIB 1
#define POK_CONFIG_NEEDS_FUNC_MEMCPY         1
#define POK_CONFIG_NEEDS_FUNC_MEMSET         1
#define POK_CONFIG_NEEDS_FUNC_MEMCMP         1
#define POK_CONFIG_NEEDS_FUNC_STRCMP         1
#define POK_CONFIG_NEEDS_FUNC_STRNCMP        1
#define POK_CONFIG_NEEDS_FUNC_STRCPY         1
#define POK_CONFIG_NEEDS_FUNC_STRNCPY        1
#define POK_CONFIG_NEEDS_FUNC_STRLEN         1
#define POK_CONFIG_NEEDS_FUNC_STREQ          1
#define POK_CONFIG_NEEDS_FUNC_ITOA           1
#define POK_CONFIG_NEEDS_FUNC_UDIVDI3        1

#define POK_NEEDS_MIDDLEWARE 1

#define POK_NEEDS_ARINC653_PARTITION 1
#define POK_NEEDS_ARINC653_PROCESS 1
#define POK_NEEDS_ARINC653_ERROR 1
#define POK_NEEDS_ARINC653_SAMPLING 1
#define POK_NEEDS_ARINC653_QUEUEING 1
#define POK_NEEDS_ARINC653_TIME 1
"""

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
        .max_nb_messages = %(max_nb_messages)d,
        
        .data = (void *) %(data)s,
        .data_stride = %(data_stride)s,
    },
"""

QUEUEING_PORT_DATA_TEMPLATE = """\
static struct {
    pok_port_size_t message_size;
    char data[%(max_message_size)d];
} %(varname)s[%(max_nb_messages)d];
"""

def _c_string(b):
    # accidentally, JSON string is very close to C string literal
    return json.dumps(b)

def write_configuration(conf, kernel_dir, partition_dirs):
    with open(os.path.join(kernel_dir, "deployment.h"), "w") as f:
        write_kernel_deployment_h(conf, f)

    with open(os.path.join(kernel_dir, "deployment.c"), "w") as f:
        write_kernel_deployment_c(conf, f)

    # caller may pass more directories than required
    # we just ignore them
    assert len(partition_dirs) >= len(conf.partitions)

    for i in range(len(conf.partitions)):
        with open(os.path.join(partition_dirs[i], "deployment.h"), "w") as f:
            write_partition_deployment_h(conf, i, f)
            
        with open(os.path.join(partition_dirs[i], "deployment.c"), "w") as f:
            write_partition_deployment_c(conf, i, f)

def write_kernel_deployment_h(conf, f):
    p = functools.partial(print, file=f)

    p("#ifndef __POK_KERNEL_GENERATED_DEPLOYMENT_H_")
    p("#define __POK_KERNEL_GENERATED_DEPLOYMENT_H_")
    
    p(COMMON_KERNEL_DEFINES)

    total_threads = (
        1 + # kernel thread
        1 + # idle thread
        sum(
            part.get_needed_threads()
            for part in conf.partitions
        )
    )

    p("#define POK_CONFIG_NB_THREADS %d" % total_threads)

    p("#define POK_CONFIG_PARTITIONS_NTHREADS {%s}" % ", ".join(
        str(part.get_needed_threads()) for part in conf.partitions
    ))


    p("#define POK_CONFIG_NB_PARTITIONS %d" % len(conf.partitions))

    p("#define POK_CONFIG_NB_LOCKOBJECTS %d" % 
        sum(part.get_needed_lock_objects() for part in conf.partitions))
    p("#define POK_CONFIG_PARTITIONS_NLOCKOBJECTS {%s}" % ", ".join(
        str(part.get_needed_lock_objects()) for part in conf.partitions
    ))
    
    p("#define POK_CONFIG_PARTITIONS_SIZE {%s}" % ", ".join(
        str(part.size) for part in conf.partitions
    ))

    p("#define POK_CONFIG_SCHEDULING_NBSLOTS %d" % len(conf.slots))

    p("#define POK_CONFIG_SCHEDULING_MAJOR_FRAME %d" %
        sum(slot.duration for slot in conf.slots)
    )

    p("#define POK_CONFIG_SCHEDULING_SLOTS {%s}" % ", ".join(
        str(slot.duration) for slot in conf.slots
    ))

    p("#define POK_CONFIG_SCHEDULING_SLOTS_ALLOCATION {%s}" % ", ".join(
        str(slot.partition) for slot in conf.slots
    ))

    n_sampling_ports = len(conf.get_all_sampling_ports())
    n_queueing_ports = len(conf.get_all_queueing_ports())

    if n_sampling_ports > 0:
        p("#define POK_NEEDS_PORTS_SAMPLING 1")
        p("#define POK_CONFIG_NB_SAMPLING_PORTS %d" % n_sampling_ports)
    if n_queueing_ports > 0:
        p("#define POK_NEEDS_PORTS_QUEUEING 1")
        p("#define POK_CONFIG_NB_QUEUEING_PORTS %d" % n_queueing_ports)

    if conf.test_support_print_when_all_threads_stopped:
        p("#define POK_TEST_SUPPORT_PRINT_WHEN_ALL_THREADS_STOPPED 1")

    p("#endif") # __POK_KERNEL_GENERATED_DEPLOYMENT_H_ 

def write_kernel_deployment_c(conf, f):
    p = functools.partial(print, file=f)
    
    p('#include "deployment.h"')
    if len(conf.get_all_ports()) > 0:
        write_kernel_deployment_c_ports(conf, f)

    write_kernel_deployment_c_hm_tables(conf, f)

def write_kernel_deployment_c_ports(conf, f):
    p = functools.partial(print, file=f)

    p("#include <middleware/port.h>")

    def get_partition(port):
        for i, part in enumerate(conf.partitions):
            if port in part.ports:
                return i
        raise ValueError

    def get_internal_port_name(port, suffix=""):
        # the name that is used for static variables
        # including port data, channels, etc.
        
        if isinstance(port, SamplingPort):
            return "sp_%d_%s" % (all_sampling_ports.index(port), suffix)
        if isinstance(port, QueueingPort):
            return "qp_%d_%s" % (all_queueing_ports.index(port), suffix)

        assert False

    def get_port_index(port):
        # returns port index in port array
        # queueing and sampling port have distinct "indexspaces"

        if isinstance(port, SamplingPort):
            return all_sampling_ports.index(port)
        if isinstance(port, QueueingPort):
            return all_queueing_ports.index(port)

        assert False
    

    all_ports = [
        port
        for part in conf.partitions
        for port in part.ports
    ]

    all_sampling_ports = [
        port
        for port in all_ports
        if isinstance(port, SamplingPort)
    ]

    all_queueing_ports = [
        port
        for port in all_ports
        if isinstance(port, QueueingPort)
    ]

    # print static data storage
    for i, port in enumerate(all_sampling_ports):
        p(SAMPLING_PORT_DATA_TEMPLATE % dict(
            varname=get_internal_port_name(port, "data"),
            max_message_size=port.max_message_size,
        ))

    for i, port in enumerate(all_queueing_ports):
        p(QUEUEING_PORT_DATA_TEMPLATE % dict(
            varname=get_internal_port_name(port, "data"),
            max_message_size=port.max_message_size,
            max_nb_messages=port.max_nb_messages,
        ))

    # print static channel definitions
    channels_per_port = collections.defaultdict(list)
    for chan in conf.channels:
        channels_per_port[chan.src].append(chan)

    # the value are tuples (varname, nchannels)
    channel_descriptors_by_port = {}
    for port, channels in channels_per_port.items():
        varname = get_internal_port_name(port, "channels")

        channel_dests = [chan.dst for chan in channels]
        channel_string = ", ".join(str(get_port_index(dst)) for dst in channel_dests)
        
        p("static pok_port_id_t %s[] = {%s};" % (varname, channel_string))

        channel_descriptors_by_port[port] = (varname, len(channels))

    # print non-static definitions

    p("pok_port_sampling_t pok_sampling_ports[] = {")
    for i, port in enumerate(all_sampling_ports):
        var_channels, num_channels = channel_descriptors_by_port.get(port, ("NULL", 0))

        p(SAMPLING_PORT_TEMPLATE % dict(
            name=_c_string(port.name),
            partition=get_partition(port),
            direction=_get_port_direction(port),
            max_message_size=port.max_message_size,
            channels=var_channels,
            num_channels=num_channels,
            data="&" + get_internal_port_name(port, "data")
        ))
    p("};")

    
    p("pok_port_queueing_t pok_queueing_ports[] = {")
    for i, port in enumerate(all_queueing_ports):
        var_channels, num_channels = channel_descriptors_by_port.get(port, ("NULL", 0))

        p(QUEUEING_PORT_TEMPLATE % dict(
            name=_c_string(port.name),
            partition=get_partition(port),
            direction=_get_port_direction(port),
            max_message_size=port.max_message_size,
            max_nb_messages=port.max_nb_messages,
            channels=var_channels,
            num_channels=num_channels,
            data="&" + get_internal_port_name(port, "data"),
            data_stride="sizeof(%s[0])" % get_internal_port_name(port, "data")
        ))
    p("};")

def write_kernel_deployment_c_hm_tables(conf, f):
    p = functools.partial(print, file=f)

    p('#include <core/error.h>')

    for i, part in enumerate(conf.partitions):
        p("static const pok_error_hm_partition_t partition_hm_table%d[] = {" % i)

        for error, action in part.hm_table:
            p("  {%s, %s}," % (error, action))

        p("  {POK_ERROR_KIND_INVALID, POK_ERROR_ACTION_IGNORE} /* sentinel value */")

        p("};")
        p("")

    p("const pok_error_hm_partition_t * const pok_partition_hm_tables[POK_CONFIG_NB_PARTITIONS] = {")
    for i in range(len(conf.partitions)):
        p("  partition_hm_table%d," % i)
    p("};")


def write_partition_deployment_h(conf, partition_idx, f):
    p = functools.partial(print, file=f)

    p("#ifndef __POK_USER_GENERATED_DEPLOYMENT_H_")
    p("#define __POK_USER_GENERATED_DEPLOYMENT_H_")
    
    p(COMMON_PARTITION_DEFINES)

    part = conf.partitions[partition_idx]

    if part.num_arinc653_buffers > 0:
        p("#define POK_NEEDS_BUFFERS 1")
        p("#define POK_NEEDS_ARINC653_BUFFER 1")
        p("#define POK_CONFIG_NB_BUFFERS %d" % part.num_arinc653_buffers)

        p("#define POK_CONFIG_BUFFER_DATA_SIZE %d" % part.buffer_data_size)
        
    if part.num_arinc653_blackboards > 0:
        p("#define POK_NEEDS_BLACKBOARDS 1")
        p("#define POK_NEEDS_ARINC653_BLACKBOARD 1")
        p("#define POK_CONFIG_NB_BLACKBOARDS %d" % part.num_arinc653_buffers)
        p("#define POK_CONFIG_BLACKBOARD_DATA_SIZE %d" % part.blackboard_data_size)

    if part.num_arinc653_semaphores > 0:
        p("#define POK_NEEDS_ARINC653_SEMAPHORE 1")
        p("#define POK_CONFIG_ARINC653_NB_SEMAPHORES %d" % part.num_arinc653_semaphores)

    if part.num_arinc653_events > 0:
        p("#define POK_NEEDS_ARINC653_EVENT 1")
        p("#define POK_CONFIG_ARINC653_NB_EVENTS %d" % part.num_arinc653_events)
        p("#define POK_CONFIG_NB_EVENTS %d" % part.num_arinc653_events)

    p("#define POK_CONFIG_NB_THREADS %d" % part.get_needed_threads())

    p("#endif")

def write_partition_deployment_c(conf, partition_idx, f):
    p = functools.partial(print, file=f)

    p('#include "deployment.h"')
