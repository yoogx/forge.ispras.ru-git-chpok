#******************************************************************
#
# Institute for System Programming of the Russian Academy of Sciences
# Copyright (C) 2016 ISPRAS
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation, Version 3.
#
# This program is distributed in the hope # that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License version 3 for more details.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

"""
This file defines classes and functions that can be used to
simplify generation of POK kernel configuration.

"""

from __future__ import print_function

import sys
import os
import abc
import json
import functools
import collections
import ipaddr


class PartitionLayout():
    """
    Contain minimal information, needed for determine layout of the partition.
    """
    def __init__(self, name, is_system=False):
        self.name = name
        self.is_system = is_system

# Single time slot for execute something.
#
# - duration - duration of given slot, in miliseconds.
class TimeSlot():
    __metaclass__ = abc.ABCMeta
    __slots__ = ["duration"]

    @abc.abstractmethod
    def get_kind_constant(self):
        pass

    def __init__(self, duration):
        self.duration = duration

    def validate(self):
        if not isinstance(self.duration, int):
            raise TypeError

class TimeSlotSpare(TimeSlot):
    __slots__ = []

    def get_kind_constant(self):
        return "POK_SLOT_SPARE"

    def __init__(self, duration):
        TimeSlot.__init__(self, duration)

# Time slot used for execute partition.
#
# - partition - reference to the partition to execute during given timeslot.
# - periodic_processing_start - whether given slot can be used for start periodic processes.
class TimeSlotPartition(TimeSlot):
    __slots__ = [
        "partition",
        "periodic_processing_start",
    ]

    def get_kind_constant(self):
        return "POK_SLOT_PARTITION"

    def __init__(self, duration, partition, periodic_processing_start):
        TimeSlot.__init__(self, duration)
        self.partition = partition
        self.periodic_processing_start = periodic_processing_start

    def validate(self):
        super(TimeSlotPartition, self).validate()

        if not isinstance(self.periodic_processing_start, bool):
            raise TypeError

# Time slot for network.
class TimeSlotNetwork(TimeSlot):
    __slots__ = []

    def __init__(self, duration):
        TimeSlot.__init__(self, duration)

    def get_kind_constant(self):
        return "POK_SLOT_NETWORKING"

# Time slot for monitor.
class TimeSlotMonitor(TimeSlot):
    __slots__ = []

    def __init__(self, duration):
        TimeSlot.__init__(self, duration)

    def get_kind_constant(self):
        return "POK_SLOT_MONITOR"
class TimeSlotGDB(TimeSlot):
    __slots__ = []
    def __init__(self, duration):
        TimeSlot.__init__(self, duration)
    def get_kind_constant(self):
        return "POK_SLOT_GDB"


# Possible system states(ordered, without prefix)
system_states = [
    'INIT_PARTOS',
    'INIT_PARTUSER',
    'INTERRUPT_HANDLER',
    'OS_MOD',
    'OS_PART',
    'ERROR_HANDLER',
    'USER'
]

# Possible error identificators(ordered, without prefix)
error_ids = [
    'MODPOSTPROCEVENT_ELIST',
    'ILLEGAL_REQUEST',
    'APPLICATION_ERROR',
    'PARTLOAD_ERROR',
    'NUMERIC_ERROR',
    'MEMORY_VIOLATION',
    'DEADLINE_MISSED',
    'HARDWARE_FAULT',
    'POWER_FAIL',
    'STACK_OVERFLOW',
    'PROCINIT_ERROR',
    'NOMEMORY_PROCDATA',
    'ASSERT',
    'CONFIG_ERROR',
    'CHECK_POOL',
    'UNHANDLED_INT'
]

class HMTable:
    def __init__(self):
        # Error level selector.
        #
        # Maps (system state) on map (error id) => {0, 1}
        # Absence of corresponded mapping defaults to 0.
        self.level_selector = {}

        # Actions for errors.
        #
        # Maps (system state) on map (error id) => action
        #
        # Absence of corresponded mapping defaults to subclass-specific value.
        self.actions = {}

    # Compute aggregate for level selector for specific error id.
    def level_selector_total(self, error_id):
        res = 0
        for shift, s in enumerate(system_states):
            if not s in self.level_selector:
                continue
            selector_per_state = self.level_selector[s]
            if not error_id in selector_per_state:
                continue

            res += selector_per_state[error_id] << shift

        return res

    # Return action for given system state and error id.
    # If mapping is absent, return given 'default_action'.
    def get_action_generic(self, system_state, error_id, default_action):
        if not system_state in self.actions:
            return default_action

        actions_per_state = self.actions[system_state]
        if not error_id in actions_per_state:
            return default_action

        return actions_per_state[error_id]

class ModuleHMTable(HMTable):
    def __init__(self):
        HMTable.__init__(self)
    def get_action(self, system_state, error_id):
        return self.get_action_generic(system_state, error_id, 'SHUTDOWN')

class PartitionHMTable(HMTable):
    # List of system states corresponded to partition
    partition_system_states = [
        'OS_PART',
        'ERROR_HANDLER',
        'USER'
    ]

    def __init__(self):
        HMTable.__init__(self)

        # Map error id -> (error code (without prefix), description)
        # for state 'USER'. Absence of corresponded mapping means that
        # given error id is never passed to error handler.
        self.user_level_codes = {}

    def get_action(self, system_state, error_id):
        return self.get_action_generic(system_state, error_id, 'IDLE')

class Space:
    """
    Definition of single memory space.

    - size - allocated RAM size in bytes (code + static storage)
    - part - partition, which should fit into given space.

    TODO: This should be make arch-dependent somehow.
    TODO: Config files uses 'size' as contained stack.
           Arch code allocates stack from other memory.
    """
    def __init__(self, size, part):
        self.size = size
        self.part = part

# ARINC partition.
#
# - name - name of the partition
# ...
# - part_id - identificator(number) of given partition.
# - part_index - index of the partition in the array. Filled automatically.
class Partition:
    __slots__ = [
        "arch", # Value propagated from 'Configuration' object.
        "name",

        "is_system",

        # Requested size of the heap.
        #
        # Note: ARINC requirements for buffers and co. shouldn't be counted here.
        "heap",

        "num_threads", # number of user threads, _not_ counting init thread and error handler
        "ports_queueing", # list of queuing ports
        "ports_sampling", # list of sampling ports

        "num_arinc653_buffers",
        "num_arinc653_blackboards",
        "num_arinc653_semaphores",
        "num_arinc653_events",

        "buffer_data_size", # bytes allocated for buffer data
        "blackboard_data_size", # same, for blackboards

        "hm_table", # partition hm table

        "ports_queueing_system", # list of queuing ports with non-empty protocol set
        "ports_sampling_system", # list of sampling ports with non-empty protocol set
    ]

    def __init__(self, arch, part_id, name):
        self.arch = arch
        self.name = name
        self.part_id = part_id

        # If application needs, it may set 'period' and 'duration' attributes.
        #
        # Otherwise, these attributes are treated as:
        #
        # 'period' is major time frame
        # 'duration' is sum of all timeslots, denoted for partition.
        self.period = None
        self.duration = None

        self.heap = 0

        self.num_threads = 0

        self.num_arinc653_buffers = 0
        self.num_arinc653_blackboards = 0
        self.num_arinc653_semaphores = 0
        self.num_arinc653_events = 0

        self.buffer_data_size = 0
        self.blackboard_data_size = 0

        self.hm_table = PartitionHMTable()

        self.ports_queueing = []
        self.ports_sampling = []

        self.ports_queueing_system = []
        self.ports_sampling_system = []

        # Internal
        self.part_index = None # Not set yet
        self.port_names_map = dict() # Map `port_name` => `port`

        self.total_time = 0 # Incremented every time timeslot is added.
        # When timeslot with periodic processing start is added, this is set to True.
        self.has_periodic_processing_start = False

    def set_index(self, part_index):
        self.part_index = part_index

    def add_port_queueing(self, port):
        if port.name in self.port_names_map:
            raise RuntimeError("Port with name %s already exists in partition %s" % (port.name, self.name))
        self.ports_queueing.append(port)
        self.port_names_map[port.name] = port

        port.partition = self

        if port.protocol is not None:
            self.ports_queueing_system.append(port)

    def add_port_sampling(self, port):
        if port.name in self.port_names_map:
            raise RuntimeError("Port with name %s already exists in partition %s" % (port.name, self.name))
        self.ports_sampling.append(port)
        self.port_names_map[port.name] = port

        port.partition = self

        if port.protocol is not None:
            self.ports_sampling_system.append(port)


    def get_all_sampling_ports(self):
        return ports_sampling

    def get_all_queueing_ports(self):
        return ports_queueing

    def validate(self):
        # validation is by no means complete
        # it's just basic sanity check

        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))

        if self.part_index is None:
            raise ValueError("Index is not set for partition '%s' (Partition is added via conf.add_partition(), isn't it?).")

        for port in self.ports_sampling + self.ports_queueing:
            port.validate()
            if port.channel_id is None:
                # Uncomment if processing of non-binded ports will be implemented
                # raise RuntimeError("Port '%s' is not connected to any channel" % port.name)
                port.channel_id = 0

    def get_needed_threads(self):
        return (
            1 + # init thread
            1 + # error handler
            self.num_threads
        )

    def get_port_by_name(self, port_name):
        return self.port_names_map[port_name]

    # Return size of memory, needed by single buffer structure.
    # TODO: This should be arch-specific somehow.
    def get_buffer_size(self):
        return 150

    # Return size of memory, needed by single blackboard structure.
    # TODO: This should be arch-specific somehow.
    def get_blackboard_size(self):
        return 100

    # Return size of memory, needed by single semaphore structure.
    # TODO: This should be arch-specific somehow.
    def get_semaphore_size(self):
        return 50

    # Return size of memory, needed by single event structure.
    # TODO: This should be arch-specific somehow.
    def get_event_size(self):
        return 50

    # Return memory size, needed by intra-partition communication mechanisms.
    def get_intra_size(self):
        return ( self.buffer_data_size + self.blackboard_data_size
            + self.num_arinc653_buffers * self.get_buffer_size()
            + self.num_arinc653_blackboards * self.get_blackboard_size()
            + self.num_arinc653_semaphores * self.get_semaphore_size()
            + self.num_arinc653_events * self.get_event_size()
        )

    def get_heap_size(self):
        heap_size = self.get_intra_size()
        if self.heap > 0:
            heap_size += self.heap + 16 # alignment. TODO: this should be arch-specific.
        return heap_size

def _get_port_direction(port):
    direction = port.direction.lower()
    if direction in ("source", "out"):
        return "POK_PORT_DIRECTION_OUT"
    if direction in ("destination", "in"):
        return "POK_PORT_DIRECTION_IN"
    raise ValueError("%r is not valid port direction" % port["direction"])


# Common parameters for Sampling and Queueing ports.
class Port:
    __metaclass__ = abc.ABCMeta
    __slots__ = [
        "partition",
        "name",
        "is_direction_src",
        "max_message_size",
        "protocol",
        "channel_id" # id of corresponded channel. Set internally.
    ]

    @abc.abstractmethod
    def get_kind_constant(self):
        pass

    def __init__(self, name, direction, max_message_size):
        self.name = name
        _direction = direction.lower()
        if _direction in ("source", "out"):
            self.is_direction_src = True
        elif _direction in ("destination", "in"):
            self.is_direction_src = False
        else:
            raise ValueError("%r is not valid port direction" % direction)

        self.max_message_size = max_message_size
        self.channel_id = None
        self.partition = None

    def is_src(self):
        return self.is_direction_src;

    def is_dst(self):
        return not self.is_direction_src;

    def setChannel(self, channel_id):
        self.channel_id = channel_id

    def validate(self):
        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))


class SamplingPort(Port):
    __slots__ = [
        "refresh",
    ]

    def get_kind_constant(self):
        return "sampling"

    def __init__(self, name, direction, max_message_size, refresh):
        Port.__init__(self, name, direction, max_message_size)
        self.refresh = refresh

    def validate(self):
        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))

class QueueingPort(Port):
    __slots__ = [
        "max_nb_message",
    ]

    def get_kind_constant(self):
        return "queueing"

    def __init__(self, name, direction, max_message_size, max_nb_message):
        Port.__init__(self, name, direction, max_message_size)
        self.max_nb_message = max_nb_message


    def validate(self):
        for attr in self.__slots__:
            if not hasattr(self, attr):
                raise ValueError("%r is not set for %r" % (attr, self))

# Generic channel connecting two connections.
#
# - max_message_size - maximum size of the message passed to the channel.

class Channel:
    __metaclass__ = abc.ABCMeta
    __slots__ = ["src", "dst"]

    def __init__(self, src, dst, max_message_size):
        self.max_message_size = max_message_size

        self.src = src
        self.dst = dst

    def validate(self):
        if not isinstance(self.src, Connection):
            raise TypeError
        if not isinstance(self.dst, Connection):
            raise TypeError

        if self.get_local_connection() == None:
            raise ValueError("at least one connection per channel must be local")

        self.src.validate()
        self.dst.validate()

    def get_local_connection(self):
        if isinstance(self.src, LocalConnection):
            return self.src
        if isinstance(self.dst, LocalConnection):
            return self.dst
        return None

    @abc.abstractmethod
    def get_kind_constant(self):
        pass

    def requires_network(self):
        return any(isinstance(x, UDPConnection) for x in [self.src, self.dst])

class ChannelQueueing(Channel):
    def __init__(self, src, dst, max_message_size, max_nb_message_send, max_nb_message_receive):
        Channel.__init__(self, src, dst, max_message_size)

        self.max_nb_message_send = max_nb_message_send
        self.max_nb_message_receive = max_nb_message_receive

    def get_kind_constant(self):
        return "queueing"


class ChannelSampling(Channel):
    def __init__(self, src, dst, max_message_size):
        Channel.__init__(self, src, dst, max_message_size)

    def get_kind_constant(self):
        return "sampling"

# One point of the channel
class Connection():
    __metaclass__ = abc.ABCMeta
    @abc.abstractmethod
    def get_kind_constant(self):
        pass

    @abc.abstractmethod
    def validate(self):
        pass

# Connection to partition's port.
class LocalConnection(Connection):
    __slots__ = ["port"]

    def __init__(self, port):
        self.port = port

    def get_kind_constant(self):
        return "Local"

    def validate(self):
        if not hasattr(self, "port") or self.port == None:
            raise ValueError

        if not isinstance(self.port, (QueueingPort, SamplingPort)):
            raise TypeError

class UDPConnection(Connection):
    __slots__ = ["host", "port"]

    def get_kind_constant(self):
        return "UDP"

    def validate(self):
        if not hasattr(self, "host"):
            raise AttributeError("host")

        if not isinstance(self.host, ipaddr.IPv4Address):
            raise TypeError(type(self.host))

        if not hasattr(self, "port"):
            raise AttributeError("port")

        if not isinstance(self.port, int):
            raise TypeError(type(self.port))

        if port < 0 or port > 0xFFFF:
            raise ValueError(self.port)

class NetworkConfiguration:
    __slots__ = [
        #"mac", # mac address
        "ip", # IP used as source
    ]

    def validate(self):
        #if not hasattr(self, "mac"):
        #    raise AttributeError
        #if self.mac is not None:
        #    if not isinstance(self.mac, bytes):
        #        raise TypeError
        #    if len(self.mac) != 6:
        #        raise ValueError
        #    if not (self.mac[0] & 0x2):
        #        print("Warning! MAC address is not locally administered one", file=sys.stderr)

        if not hasattr(self, "ip"):
            raise AttributeError
        if not isinstance(self.ip, ipaddr.IPv4Address):
            raise TypeError

    #def mac_to_string(self):
    #    return "{%s}" % ", ".join(hex(i) for i in self.mac)

class Configuration:
    system_states_all = system_states
    error_ids_all = error_ids

    __slots__ = [
        "partitions",
        "slots", # time windows
        "channels_queueing", # queueing port channels (connections)
        "channels_sampling", # sampling port channels (connections)
        "network", # NetworkConfiguration object (or None)

        "spaces", # Array of 'Space' objects.

        # if this is set, POK writes a special string once
        # there are no more schedulable threads
        # it's used by test runner as a sign that POK
        # can be terminated
        "test_support_print_when_all_threads_stopped",
    ]

    def __init__(self, arch):
        self.arch = arch

        self.module_hm_table = ModuleHMTable()

        self.partitions = []
        self.slots = []
        self.channels_queueing = []
        self.channels_sampling = []
        self.network = None

        self.spaces = []

        self.test_support_print_when_all_threads_stopped = False

        self.major_frame = 0

        # For internal usage
        self.partition_names_map = dict()
        self.partition_ids_map = dict()
        self.next_partition_id = 0

        self.next_channel_id_sampling = 0
        self.next_channel_id_queueing = 0

    def add_partition(self, part_name, part_size, part_id = None):
        if part_name in self.partition_names_map:
            raise RuntimeError("Adding already existed partition '%s'" % part_name)

        part_id_real = None

        if part_id is not None:
            if part_id in self.partition_ids_map:
                raise RuntimeError("Adding already existed partition (by id) '%s'" % part_id)
            part_id_real = part_id
        else:
            part_id_real = self.next_partition_id
            self.next_partition_id += 1

        part = Partition(self.arch, part_id_real, part_name)
        part.set_index(len(self.partitions))

        self.partitions.append(part)
        self.partition_names_map[part_name] = part

        if part_id is not None:
            self.partition_ids_map[part_id] = part

        self.spaces.append(Space(part_size, part))

        return part

    def add_channel(self, src_connection, dst_connection):
        channel_type = None
        channel_max_message_size = None
        max_nb_message_receive = 1 # Only for queueing channel
        max_nb_message_send = 1 # Only for queueing channel

        for connection in [src_connection, dst_connection]:
            if connection is not None:
                if not isinstance(connection, LocalConnection):
                    raise RuntimeError("Non-local connections are not supported now")
                if isinstance(connection.port, SamplingPort):
                    if channel_type is not None:
                        if channel_type != "sampling":
                            raise RuntimeError("Channel for ports of different types: %s and %s" %
                                (src_connection.port.name, dst_connection.port.name))
                    else:
                        channel_type = "sampling"
                    connection.port.setChannel(self.next_channel_id_sampling)
                else: # Local connection to queueing port
                    if channel_type is not None:
                        if channel_type != "queueing":
                            raise RuntimeError("Channel for ports of different types: %s and %s" %
                                (src_connection.port.name, dst_connection.port.name))
                    else:
                        channel_type = "queueing"
                    connection.port.setChannel(self.next_channel_id_queueing)

                    if connection == src_connection:
                        if not connection.port.is_src():
                            raise RuntimeError("Using dst port '%s' as src connection for the channel" % connection.port.name)
                        max_nb_message_send = connection.port.max_nb_message
                    else:
                        if not connection.port.is_dst():
                            raise RuntimeError("Using dst port '%s' as src connection for the channel" % connection.port.name)
                        max_nb_message_receive = connection.port.max_nb_message

                if channel_max_message_size is not None:
                    if channel_max_message_size > connection.port.max_message_size:
                        raise RuntimeError("Max message size of dst port '%s' is less than one for src port '%s'" %
                            (src_connection.port.name, dst_connection.port.name))
                else:
                    channel_max_message_size = connection.port.max_message_size

        if channel_type is None:
            raise RuntimeError("At least one connection for channel should be local")

        if channel_type == 'sampling':
            channel = ChannelSampling(src_connection, dst_connection, channel_max_message_size)
            self.channels_sampling.append(channel)
            self.next_channel_id_sampling += 1
        else:
            channel = ChannelQueueing(src_connection, dst_connection, channel_max_message_size,
                max_nb_message_send, max_nb_message_receive)
            self.channels_queueing.append(channel)
            self.next_channel_id_queueing += 1

    def add_time_slot(self, slot):
        if isinstance(slot, TimeSlotPartition):
            slot.partition.total_time += slot.duration
            if slot.periodic_processing_start:
                slot.partition.has_periodic_processing_start = True

        self.slots.append(slot)
        self.major_frame += slot.duration

    def get_all_ports(self):
        return sum((part.get_all_ports() for part in self.partitions), [])

    def get_all_sampling_ports(self):
        return sum((part.get_all_sampling_ports() for part in self.partitions), [])

    def get_all_queueing_ports(self):
        return sum((part.get_all_queueing_ports() for part in self.partitions), [])

    def get_partition_by_name(self, name):
        return self.partition_names_map[name]

    def get_partition_by_id(self, part_id):
        return self.partition_ids_map[part_id]

    def get_port_by_partition_and_name(self, partition_name, port_name):
        return self.get_partition_by_name(partition_name).get_port_by_name(port_name)

    def validate(self):
        for part in self.partitions:
            part.validate()

        # network stuff
        networking_time_slot_exists = any(isinstance(slot, TimeSlotNetwork) for slot in self.slots)

        if self.network:
            self.network.validate()

            if not networking_time_slot_exists:
                raise ValueError("Networking is enabled, but no dedicated network processing time slot is present")
        else:
            if networking_time_slot_exists:
                raise ValueError("Networking is disabled, but there's (unnecessary) network processing time slot in the schedule")

            #if any(chan.requires_network() for chan in self.channels):
            #    raise ValueError("Network channel is present, but networking is not configured")

        # validate schedule
        if not isinstance(self.slots[0], TimeSlotPartition):
            raise ValueError("First time slot must be partition slot")

        for partition in self.partitions:
            if not partition.has_periodic_processing_start:
                raise ValueError("partitions '%s' don't have periodic processing points set" % partition.name)

    def get_all_channels(self):
        return self.channels
