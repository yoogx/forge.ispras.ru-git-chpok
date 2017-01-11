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
Internal configuration of the module.
"""

import chpok_configuration
import types_requirements
import memory_definition

from text_serialization import *

class ConfigurationInternal(SerializableObject):
    yaml_tag = '!IConfiguration'

    copy_slots = [
        'module_hm_table',
        'partitions',
        'slots',
        'channels_sampling',
        'channels_queueing',
        'major_frame',
        'phys_total'
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    # Serialization/deserialization methods
    def save_to_file(self, filename):
        serialize_object_as_text(self, filename)

    @classmethod
    def load_from_file(cls, filename):
        return deserialize_object_from_text(cls, filename)

    @classmethod
    def from_normal(cls, conf, types_requirements, partition_elf_map):
        """
        Create object of type 'ConfigurationInternal' from 'Configuration' one.

        'types_requirements' - object of type TypesRequirements.
        'partition_elf_map' - dictionary which maps partition name to partition elf.
        """

        conf_internal = ConfigurationInternal(
            module_hm_table = ModuleHMTableInternal.from_normal(conf.module_hm_table),
            partitions = [],
            slots = [],
            channels_sampling = [],
            channels_queueing = [],
            major_frame = 0,
            phys_total = 2**32, # TODO: Make it configurable
        )

        for index, part in enumerate(conf.partitions):
            part_internal = PartitionInternal.from_normal(part, index, partition_elf_map[part.name])
            conf_internal.partitions.append(part_internal)

        process_time_slots(conf_internal, conf)

        for channel in conf.channels:
            process_channel(conf_internal, channel)

        for part in conf.partitions:
            compute_arinc_requirements(part, types_requirements)

        return conf_internal

    def create_memory_constraints(self):
        """
        Create object ModuleMemoryDefinition as memory constraints.
        """
        md = memory_definition.ModuleMemoryDefinition(
            partitions = [],
            phys_total = self.phys_total
        )

        for part in self.partitions:
            pmd = memory_definition.PartitionMemoryDefinition(
                name = part.name,
                space_id = part.space_id,
                memory_blocks = [],
                part_elf = part.part_elf,
                elf_size = part.memory_size
            )

            # Memory block for heap
            heap_mbd = memory_definition.MemoryBlockDefinition(
                name = ".HEAP",
                size = part.heap_size,
                align = part.heap_align
            )

            pmd.memory_blocks.append(heap_mbd)

            # Memory block for ARINC heap
            arinc_heap_mbd = memory_definition.MemoryBlockDefinition(
                name = ".ARINC_HEAP",
                size = part.arinc_heap_size,
                align = part.arinc_heap_align
            )

            pmd.memory_blocks.append(arinc_heap_mbd)

            # Kernel SHared Data
            kshd_mbd = memory_definition.MemoryBlockDefinition(
                name = ".KSHD",
                size = 4096, # TODO: Compute it somehow.
                align = 4096
            )

            pmd.memory_blocks.append(kshd_mbd)

            # Memory block for stacks
            stacks_mbd = memory_definition.MemoryBlockDefinition(
                name = ".STACKS",
                size = part.stack_size_all,
                align = 4096 # Hardcoded. TODO: Should it be arch-depended?
            )

            pmd.memory_blocks.append(stacks_mbd)

            # TODO: Fill user-defined memory blocks

            md.partitions.append(pmd)

        return md


class HMActionInternal(SerializableObject):
    """
    Abstract HM action.
    """

    copy_slots = [
        'level', # one of two possible levels
        'recovery_action', # One of recovery actions
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

class HMTableInternal(SerializableObject):
    """
    Abstract HM table.

    Subclasses need to define fields:

     - 'levels' - contains pair of possible error levels
    """
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

    copy_slots = [
        'actions', # map <system_state> * <error_id> => <subclass-of HMActionInternal>
    ]

    def __init__(self, actions):
        self.actions = actions

    # Compute aggregate for level selector for specific error id.
    # Used while generate deployment.c
    def level_selector_total(self, error_id):
        res = 0
        for shift, s in enumerate(HMTableInternal.system_states):
            actions_per_state = self.actions.get(s, {})
            action = actions_per_state.get(error_id, None)
            if action is None:
                level_val = 0
            else:
                level_val = self.__class__.levels.index(action.level)

            res += level_val << shift

        return res

    def recovery_action(self, system_state, error_id, default_action = None):
        if system_state not in self.actions:
            return default_action

        return self.actions[system_state][error_id].recovery_action

class ModuleHMActionInternal(HMActionInternal):
    yaml_tag = '!IModuleHMAction'


class ModuleHMTableInternal(HMTableInternal):
    yaml_tag = '!IModuleHMTable'

    levels = ('MODULE', 'PARTITION')

    @classmethod
    def from_normal(cls, module_hm_table):

        actions = {}

        for state in cls.system_states:
            error_ids_levels = module_hm_table.level_selector.get(state, {})
            error_ids_map = {}
            for error_id in HMTableInternal.error_ids:
                level_val = error_ids_levels.get(error_id, 0)
                level = ModuleHMTableInternal.levels[level_val]
                error_ids_map[error_id] = ModuleHMActionInternal (
                    level = level,
                    recovery_action = module_hm_table.get_action(state, error_id)
                )
            actions[state] = error_ids_map

        module_hm_table_internal = ModuleHMTableInternal(
            actions = actions
        )

        return module_hm_table_internal

def align_val(val, align):
    return ((val + align - 1) / align) * align

class PartitionInternal(SerializableObject):
    yaml_tag = '!IPartition'

    copy_slots = [
        'name',
        'part_id',
        'index',

        'space_id',

        'is_system',

        'period',
        'duration',

        'memory_size',
        'memory_align',

        'heap_size',
        'heap_align',

        'num_threads',

        'num_threads_total', # Include main thread and error thread.

        'stack_size_all', # Total memory for all stacks

        "ports_queueing", # list of queuing ports
        "ports_sampling", # list of sampling ports

        "num_arinc653_buffers",
        "num_arinc653_blackboards",
        "num_arinc653_semaphores",
        "num_arinc653_events",

        "buffer_data_size", # bytes allocated for buffer data
        "blackboard_data_size", # same, for blackboards

        "arinc_heap_size", # Size of the heap needed for ARINC needs.
        "arinc_heap_align",

        'partition_hm_table',

        'part_elf',
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    @classmethod
    def from_normal(cls, part, index, part_elf):
        """
        Create internal partition's configuration from normal one.
        Valid 'index' and 'part_elf' are not required when configure single partition.
        """
        partition_hm_table = PartitionHMTableInternal.from_normal(part.hm_table)

        part_internal = PartitionInternal(
            name = part.name,
            part_id = part.part_id,
            index = index,

            space_id = index + 1,

            is_system = part.is_system,

            period = None, # Will be set later
            duration = None, # Will be set later

            memory_size = part.memory_size,
            memory_align = 4096, # Hardcoded. TODO: Read that from elf.

            heap_size = part.heap_size,
            heap_align = 4096, # Hardcoded.

            num_threads = part.num_threads,
            num_threads_total = part.num_threads + 2,

            stack_size_all = part.stack_size_all,

            ports_queueing = [],
            ports_sampling = [],

            num_arinc653_buffers = part.num_arinc653_buffers,
            num_arinc653_blackboards = part.num_arinc653_blackboards,
            num_arinc653_semaphores = part.num_arinc653_semaphores,
            num_arinc653_events = part.num_arinc653_events,

            buffer_data_size = part.buffer_data_size,
            blackboard_data_size = part.blackboard_data_size,

            arinc_heap_size = 0, # Will be counted
            arinc_heap_align = 1, # Will be counted

            partition_hm_table = partition_hm_table,

            part_elf = part_elf
        )

        part_private = PartitionPrivate(part_internal)
        part.private_data = part_private

        for index, port_sampling in enumerate(part.ports_sampling):
            if port_sampling.is_src():
                direction = 'OUT'
            else:
                direction = 'IN'

            port_internal = SamplingPortInternal(
                name = port_sampling.name,
                index = index,

                direction = direction,
                max_message_size = port_sampling.max_message_size,
                refresh = port_sampling.refresh,

                sampling_channel_index = None # Not set yet
            )

            port_sampling.private_data = port_internal

            part_internal.ports_sampling.append(port_internal)

        for index, port_queueing in enumerate(part.ports_queueing):
            if port_queueing.is_src():
                direction = 'OUT'
            else:
                direction = 'IN'

            port_internal = QueueingPortInternal(
                name = port_queueing.name,
                index = index,

                direction = direction,
                max_message_size = port_queueing.max_message_size,
                max_nb_message = port_queueing.max_nb_message,

                queueing_channel_index = None # Not set yet
            )

            port_queueing.private_data = port_internal

            part_internal.ports_queueing.append(port_internal)

        return part_internal


    def arinc_heap_add_array(self, n_elems, types_requirements,
        type_name):
        """
        Extend ARINC heap for new array with elements of given type.

        If n_elems is 0, do nothing.
        """
        if n_elems == 0:
            return

        type_requirements_single = types_requirements.types[type_name]
        elem_align = type_requirements_single.align
        elem_size = type_requirements_single.size

        if self.arinc_heap_align < elem_align:
            self.arinc_heap_align = elem_align

        stride = align_val(elem_size, elem_align)

        array_size = elem_size + (n_elems - 1) * stride

        self.arinc_heap_size = align_val(self.arinc_heap_size, elem_size) + array_size

class SamplingPortInternal(SerializableObject):
    yaml_tag = '!ISamplingPort'

    copy_slots = [
        'name',
        'index', # Index in the list of sampling ports for partition.

        'direction', # 'IN' or 'OUT'
        'max_message_size',
        'refresh',

        'sampling_channel_index'
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

class QueueingPortInternal(SerializableObject):
    yaml_tag = '!IQueueingPort'

    copy_slots = [
        'name',
        'index', # Index in the list of queueing ports for partition.

        'direction', # 'IN' or 'OUT'
        'max_message_size',

        'max_nb_message',

        'queueing_channel_index'
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

class PartitionHMActionInternal(HMActionInternal):
    yaml_tag = '!IPartitionHMAction'

    def __init__(self, level, recovery_action, error_code = None, description = None):
        HMActionInternal.__init__(self, level = level, recovery_action = recovery_action)
        if error_code is not None:
            self.error_code = error_code # Error code, only for errors in 'USER' state.
            self.description = description # Description of the error, only for errors in 'USER' state.


class PartitionHMTableInternal(HMTableInternal):
    yaml_tag = '!IPartitionHMTable'

    # List of system states corresponded to partition
    partition_system_states = [
        'OS_PART',
        'ERROR_HANDLER',
        'USER'
    ]

    levels = ('PARTITION', 'PROCESS')

    @classmethod
    def from_normal(cls, partition_hm_table):
        actions = {}

        for state in cls.partition_system_states:
            error_ids_levels = partition_hm_table.level_selector.get(state, {})
            error_ids_map = {}

            if state == 'USER':
                user_level_codes = partition_hm_table.user_level_codes
            else:
                user_level_codes = {}

            for error_id in HMTableInternal.error_ids:
                level_val = error_ids_levels.get(error_id, 0)
                level = PartitionHMTableInternal.levels[level_val]

                user_code = user_level_codes.get(error_id, None)
                if user_code is not None:
                    error_code = user_code[0]
                    description = user_code[1]
                else:
                    error_code = None
                    description = None

                error_ids_map[error_id] = PartitionHMActionInternal (
                    level = level,
                    recovery_action = partition_hm_table.get_action(state, error_id),
                    error_code = error_code,
                    description = description
                )


            actions[state] = error_ids_map

        partition_hm_table_internal = PartitionHMTableInternal(
            actions = actions
        )

        return partition_hm_table_internal


class MemoryBlockInternal(SerializableObject):
    yaml_tag = '!IMemoryBlock'

    copy_slots = [
        "name", # Name of the block. Should be unique in partition.
        "size", # Size of the block. Should be non-negative.
        "vaddr", # Virtual address of the block, if required.
        "paddr", # Physical address of the block, if required.
        "cache_policy", # Enumeration.
        "is_coherent", # Modificator for some 'cache_policy' values.
        "is_guarded", # Modificator for some 'cache_policy' values.
        "is_writable", # Whether memory block can be written by the user.
        "is_contiguous", # Whether block should be *physically* contiguous.
        "align", # Alignment of the block. 4k by default.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

class TimeSlotInternal(SerializableObject):
    yaml_tag = '!ITimeSlot'

    copy_slots = [
        'offset',
        'duration',
        'slot_type', # 'SPARE', 'PARTITION', 'MONITOR', 'GDB'
         # If slot_type is 'PARTITION', this is index of linked partition.
         # Otherwise None.
        'partition_index',
         # Have a sence If slot_type is 'PARTITION', this is TRUE if periodic processes
         # are allowed to start in that slot.
         # Otherwise None.
        'periodic_processing_start'
    ]

    def __init__(self,
            offset,
            duration,
            slot_type, # 'SPARE', 'PARTITION', 'MONITOR', 'GDB'
            partition_index = None, # If slot_type is 'PARTITION', this is index of linked partition.
            periodic_processing_start = None # Have a sence only for 'PARTITION' slot type.
        ):

        self.offset = offset
        self.duration = duration
        self.slot_type = slot_type
        if partition_index is not None:
            self.partition_index = partition_index
            self.periodic_processing_start = periodic_processing_start


class ChannelConnectionInternal(SerializableObject):
    yaml_tag = '!IChannelConnection'

    copy_slots = [
        'partition_index',
        'port_index', # Index in the appropriate list of ports.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    @classmethod
    def from_normal(cls, localConnection):
        """
        Create object from LocalConnection object.
        NOTE: partitions should be processes before (so 'private_data' field for every partition is set).
        """
        return ChannelConnectionInternal(
            partition_index = localConnection.partition.private_data.part_internal.index,
            port_index = localConnection.port.private_data.index
        )

class ChannelQueueingInternal(SerializableObject):
    yaml_tag = '!IChannelQueueing'

    copy_slots = [
        'src',
        'dst',
        'index',
        'max_message_size',
        'max_nb_message_receive',
        'max_nb_message_send'
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

class ChannelSamplingInternal(SerializableObject):
    yaml_tag = '!IChannelSampling'

    copy_slots = [
        'src',
        'dst',
        'index',
        'max_message_size',
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


# Private data for 'Partition' object.
class PartitionPrivate:
    def __init__(self, part_internal):
        self.part_internal = part_internal
        # Counter for duration within major frame
        self.total_duration = 0
        # Whether there is at least one time slot for partition, which
        # has periodic_processing_start property.
        self.has_periodic_processing_start = False

# Compute requirements for ARINC heap.
def compute_arinc_requirements(part, types_requirements):
    part_internal = part.private_data.part_internal

    part_internal.arinc_heap_add_array(part.num_arinc653_buffers,
        types_requirements, 'struct arinc_buffer')
    part_internal.arinc_heap_add_array(part.num_arinc653_buffers,
        types_requirements, 'struct arinc_blackboard')
    part_internal.arinc_heap_add_array(part.num_arinc653_buffers,
        types_requirements, 'struct arinc_semaphore')
    part_internal.arinc_heap_add_array(part.num_arinc653_buffers,
        types_requirements, 'struct arinc_event')

    part_internal.arinc_heap_size += part.buffer_data_size
    part_internal.arinc_heap_size += part.blackboard_data_size

# Process configuration for time slots and fill 'slots' list.
# Return major time frame.
def process_time_slots(conf_internal, conf):
    # Actual major frame.
    major_frame = 0

    for slot in conf.slots:
        slot_offset = major_frame

        if slot.get_kind_constant() == "POK_SLOT_PARTITION":
            part = slot.partition
            part_private = part.private_data

            slot_internal = TimeSlotInternal(
                offset = major_frame,
                duration = slot.duration,
                slot_type = 'PARTITION',
                partition_index = part_private.part_internal.index,
                periodic_processing_start = slot.periodic_processing_start
            )

            part_private.total_duration += slot.duration
            part_private.has_periodic_processing_start = part_private.has_periodic_processing_start and slot.periodic_processing_start

        else: # Slot other than for partition
            slot_type = None
            if slot.get_kind_constant() == 'POK_SLOT_GDB':
                slot_type = 'GDB'
            elif slot.get_kind_constant() == 'POK_SLOT_MONITOR':
                slot_type = 'MONITOR'
            else:
                slot_type = 'SPARE'

            slot_internal = TimeSlotInternal(
                offset = major_frame,
                duration = slot.duration,
                slot_type = slot_type)

        major_frame += slot.duration

        conf_internal.slots.append(slot_internal)

    if conf.major_frame is not None:
        if conf.major_frame != major_frame:
            raise RuntimeError("Configuration sets major frame equal to %d, but sum of all time slots is %d." % [conf.major_frame, major_frame])

    conf_internal.major_frame = major_frame


    for part in conf.partitions:
        # Check partition's timing requirements.
        # If they are not filled, fill with some values.
        part_private = part.private_data

        #if part_private.total_duration > 0:
        #    if not part_private.has_periodic_processing_start:
        #        raise RuntimeError("Partition %s has time slot(s) assigned to it, but none of them has periodic processing start property." % part.name)

        if part.period is not None:
            if part.period % major_frame != 0:
                raise RuntimeError("Partition %s has period %d which is not divident of major time frame (%d)." % [part.name, part.period, major_frame])
            duration_max = part_private.total_duration * (part.period / major_frame)
            if part.duration is not None:
                if part.duration > duration_max:
                    raise RuntimeError("Partition %s has duration %d ns, but only %d ns is available to it during period." % [part.name, part.duration, duration_max])
            else:
                part.duration = duration_max
        else:
            part.period = major_frame
            part.duration = part_private.total_duration

        part_internal = part_private.part_internal

        part_internal.period = part.period
        part_internal.duration = part.duration

# Process single channel in configuration
def process_channel(conf_internal, channel):

    # Sum of types of ports. 1 - sampling, 2 - queueing
    types_sum = 0

    # Compute type of the channel.
    for conn in [channel.src, channel.dst]:
        if not isinstance(conn, chpok_configuration.LocalConnection):
            raise RuntimeError("Only local connections are supported for the channel.")
        port = conn.port

        if port.get_kind_constant() == "sampling":
            types_sum += 1
        else:
            types_sum += 2

    if types_sum % 2 == 1:
        raise RuntimeError("Sampling and queuing ports are connected to the single channel.")

    port_src = channel.src.port
    port_dst = channel.dst.port

    if not port_src.is_src():
        raise RuntimeError("Source port is used as destination in the channel.")

    if port_dst.is_src():
        raise RuntimeError("Destination port is used as source in the channel.")

    # Use max message size of the src port.
    max_message_size = port_src.max_message_size

    if port_dst.max_message_size < max_message_size:
        raise RuntimeError("In channel destination port has max message size less then source one.")

    src_connection = ChannelConnectionInternal.from_normal(channel.src)
    dst_connection = ChannelConnectionInternal.from_normal(channel.dst)

    if types_sum / 2 == 1:
        # Sampling channel
        sampling_channel_index = len(conf_internal.channels_sampling)

        channel_internal = ChannelSamplingInternal(
            src = src_connection,
            dst = dst_connection,
            index = sampling_channel_index,
            max_message_size = port_src.max_message_size
        )

        conf_internal.channels_sampling.append(channel_internal)

        port_src.private_data.sampling_channel_index = sampling_channel_index
        port_dst.private_data.sampling_channel_index = sampling_channel_index
    else:
        # Queuing channel
        queueing_channel_index = len(conf_internal.channels_queueing)

        channel_internal = ChannelQueueingInternal(
            src = src_connection,
            dst = dst_connection,
            index = queueing_channel_index,
            max_message_size = port_src.max_message_size,
            max_nb_message_receive = port_src.max_nb_message,
            max_nb_message_send = port_dst.max_nb_message,
        )

        conf_internal.channels_queueing.append(channel_internal)

        port_src.private_data.queueing_channel_index = queueing_channel_index
        port_dst.private_data.queueing_channel_index = queueing_channel_index

    channel.private_data = channel_internal
