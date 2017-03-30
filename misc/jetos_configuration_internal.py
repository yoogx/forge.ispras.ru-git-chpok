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

import elf_info

class ConfigurationInternal(SerializableObject):
    yaml_tag = '!IConfiguration'

    copy_slots = [
        'module_hm_table',
        'partitions',
        'slots',
        'channels_sampling',
        'channels_queueing',
        'major_frame',
        'memory_block_sharings', # List of 'MemoryBlockSharingInternal' objects.
        'portal_types', # List of 'IPPCPortalTypeInternal' objects.
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
    def from_normal(cls, conf, types_requirements):
        """
        Create object of type 'ConfigurationInternal' from 'Configuration' one.

        'types_requirements' - object of type TypesRequirements.
        """

        conf_internal = ConfigurationInternal(
            module_hm_table = ModuleHMTableInternal.from_normal(conf.module_hm_table),
            partitions = [],
            slots = [],
            channels_sampling = [],
            channels_queueing = [],
            major_frame = 0,
            memory_block_sharings = [],
            portal_types = [],
        )

        for index, part in enumerate(conf.partitions):
            part_internal = PartitionInternal.from_normal(part, index)
            conf_internal.partitions.append(part_internal)

        process_time_slots(conf_internal, conf)

        for channel in conf.channels:
            process_channel(conf_internal, channel)

        for memory_block_sharing in conf.memory_block_sharings:
            mb_refs_internal = []

            for mb_ref in memory_block_sharing.mb_refs:
                part_internal = mb_ref.partition.private_data.part_internal
                memory_block_internal = mb_ref.memory_block.private_data

                mb_ref_internal = MemoryBlockRefInternal(
                    part_index = part_internal.index,
                    part_name = part_internal.name,
                    mb_index = memory_block_internal.index,
                    mb_name = memory_block_internal.name
                )

                mb_refs_internal.append(mb_ref_internal)

            memory_block_sharing_internal = MemoryBlockSharingInternal(
                mb_refs = mb_refs_internal)

            conf_internal.memory_block_sharings.append(memory_block_sharing_internal)

        for portal_type in conf.portal_types:
            server_part_internal = PartitionInternal.ref_from_normal(portal_type.server_part)
            portal_type_internal = IPPCPortalTypeInternal(
                portal_type_name = portal_type.name,
                portal_type_index = len(conf_internal.portal_types),
                server_part_name = server_part_internal.name,
                server_part_index = server_part_internal.index,
                portals = []
            )

            conf_internal.portal_types.append(portal_type_internal)
            portal_type_server_internal = IPPCPortalTypeServerInternal(
                portal_type_name = portal_type_internal.portal_type_name,
                portal_type_index = portal_type_internal.portal_type_index
            )
            server_part_internal.portal_types.append(portal_type_server_internal)

            for portal in portal_type.portals:
                client_part_internal = PartitionInternal.ref_from_normal(portal.client_part)
                portal_internal = IPPCPortalInternal(
                    portal_index = len(portal_type_internal.portals),
                    client_part_name = client_part_internal.name,
                    client_part_index =  client_part_internal.index,
                    connections_n = portal.connections_n,
                )

                portal_type_internal.portals.append(portal_internal)
                portal_client_internal = IPPCPortalClientInternal(
                    portal_type_name = portal_type_internal.portal_type_name,
                    portal_type_index = portal_type_internal.portal_type_index,
                    portal_index = portal_internal.portal_index,
                    connections_n = portal_internal.connections_n,
                )
                client_part_internal.portals.append(portal_client_internal)

        for part in conf.partitions:
            compute_arinc_requirements(part, types_requirements)

        return conf_internal

    def create_memory_constraints(self, env, phys_total):
        """
        Create object ModuleMemoryDefinition as memory constraints.
        """
        md = memory_definition.ModuleMemoryDefinition(
            partitions = [],
            memory_block_sharings = [],
            phys_total = phys_total,
        )

        for part in self.partitions:
            pmd = memory_definition.PartitionMemoryDefinition(
                name = part.name,
                space_id = part.space_id,
                memory_blocks = [memory_definition.MemoryBlockDefinition.from_config(m) for m in part.memory_blocks],
                memory_size = part.memory_size
            )

            # Memory blocks for ELF
            segments = elf_info.elf_read_segments(env, env['PARTITIONS_ELF_MAP'][pmd.name])

            for i, segment in enumerate(segments):
                elf_mbd = memory_definition.MemoryBlockDefinition(
                    name = '.ELF.' + str(i),
                    size = segment.MemSiz,
                    align = segment.Align,
                    vaddr = segment.VirtAddr,
                    access = segment.memory_block_access(),
                    init_source = "ELF",
                    init_stage = "PARTITION"
                )

                pmd.add_memory_block(elf_mbd)

            # Memory block for heap
            heap_mbd = memory_definition.MemoryBlockDefinition(
                name = ".HEAP",
                size = part.heap_size,
                align = part.heap_align
            )

            pmd.add_memory_block(heap_mbd)

            # Memory block for ARINC heap
            arinc_heap_mbd = memory_definition.MemoryBlockDefinition(
                name = ".ARINC_HEAP",
                size = part.arinc_heap_size,
                align = part.arinc_heap_align
            )

            pmd.add_memory_block(arinc_heap_mbd)

            # Kernel SHared Data
            kshd_mbd = memory_definition.MemoryBlockDefinition(
                name = ".KSHD",
                size = 4096, # TODO: Compute it somehow.
                align = 4096
            )

            pmd.add_memory_block(kshd_mbd)

            # Memory block for stacks
            stacks_mbd = memory_definition.MemoryBlockDefinition(
                name = ".STACKS",
                size = part.stack_size_all,
                align = 4096 # Hardcoded. TODO: Should it be arch-depended?
            )

            pmd.add_memory_block(stacks_mbd)

            md.partitions.append(pmd)

        for memory_block_sharing in self.memory_block_sharings:
            mb_refs_def = []
            for mb_ref in memory_block_sharing.mb_refs:
                mbrd = memory_definition.MemoryBlockRefDefinition(
                    part_name = mb_ref.part_name,
                    part_index = mb_ref.part_index,
                    mb_name = mb_ref.mb_name,
                    mb_index = mb_ref.mb_index,
                )
                mb_refs_def.append(mbrd)

            mbsd = memory_definition.MemoryBlockSharingDefinition(mb_refs = mb_refs_def)

            md.memory_block_sharings.append(mbsd)

        return md


class MemoryBlockSharingInternal(SerializableObject):
    """
    Sharing physical space by one or more memory blocks.
    """
    yaml_tag = '!IMemoryBlockSharing'

    copy_slots = [
        'mb_refs', # List of 'MemoryBlockRefInternal' objects
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


class MemoryBlockRefInternal(SerializableObject):
    """
    Reference to memory block from outsize of any partition.
    """
    yaml_tag = '!IMemoryBlockRef'

    copy_slots = [
        'part_name',
        'part_index',
        'mb_name',
        'mb_index',
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


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

class ModuleHMActionInternal(HMActionInternal):
    yaml_tag = '!IModuleHMAction'

    @classmethod
    def from_normal(cls, module_action):
        return cls(
            level = module_action.level,
            recovery_action = module_action.recovery_action
        )


class ModuleHMTableInternal(HMTableInternal):
    yaml_tag = '!IModuleHMTable'

    levels = ('MODULE', 'PARTITION')

    @classmethod
    def from_normal(cls, module_hm_table):

        actions_internal = {}

        for state in cls.system_states:
            error_ids_map = {}
            for error_id in cls.error_ids:
                action = module_hm_table.get_action(state, error_id)

                error_ids_map[error_id] = ModuleHMActionInternal.from_normal(action)

            actions_internal[state] = error_ids_map

        module_hm_table_internal = ModuleHMTableInternal(
            actions = actions_internal
        )

        return module_hm_table_internal

def align_val(val, align):
    return ((val + align - 1) / align) * align

class IPPCPortalClientInternal(SerializableObject):
    yaml_tag = '!IPPCPortalClient'

    copy_slots = [
        "portal_type_name", # Name of the portal type
        "portal_type_index", # Index of the portal type in global list.
        "portal_index", # Index of the portal in portal type.
        "connections_n", # Number of connections
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

class IPPCPortalTypeServerInternal(SerializableObject):
    yaml_tag = '!IPPCPortalTypeServer'

    copy_slots = [
        "portal_type_name", # Name of the portal type
        "portal_type_index", # Index of the portal type in global list.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)



class PartitionInternal(SerializableObject):
    yaml_tag = '!IPartition'

    copy_slots = [
        'name',
        'part_id',
        'index',

        'space_id', # Always 'index' + 1

        'is_system',

        'period',
        'duration',

        'memory_size',

        'heap_size',
        'heap_align',

        'num_threads',

        'num_threads_total', # Include main, error and server threads.

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

        'memory_blocks',

        'portal_types', # List of IPPCPortalTypeServerInternal objects, provided by given partition
        'portals', # List of IPPCPortalClientInternal objects, used by given partition
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    @classmethod
    def from_normal(cls, part, index):
        """
        Create internal partition's configuration from normal one.
        Valid 'index' is not required when configure single partition.
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

            memory_blocks = [],

            portal_types = [], # Will be set in global code
            portals = [], # Will be set in global code
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


        for i, memory_block in enumerate(part.memory_blocks):
            memory_block_internal = MemoryBlockInternal.from_normal(memory_block, i)

            memory_block.private_data = memory_block_internal

            part_internal.memory_blocks.append(memory_block_internal)

        return part_internal

    @classmethod
    def ref_from_normal(cls, part):
        """
        Extract reference to internal partition from normal one.
        """
        return part.private_data.part_internal

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

    def __init__(self, level, recovery_action, error_code = None, error_description = None):
        HMActionInternal.__init__(self, level = level, recovery_action = recovery_action)
        if error_code is not None:
            self.error_code = error_code # Error code, only for errors in 'USER' state.
            self.error_description = error_description # Description of the error, only for errors in 'USER' state.

    @classmethod
    def from_normal(cls, action):
        return cls(
            level = action.level,
            recovery_action = action.recovery_action,
            error_code = getattr(action, 'error_code', None),
            error_description = getattr(action, 'description', None),
        )


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
        actions_internal = {}

        for state in cls.partition_system_states:
            error_ids_map = {}
            for error_id in HMTableInternal.error_ids:
                action = partition_hm_table.get_action(state, error_id)
                error_ids_map[error_id] = PartitionHMActionInternal.from_normal(action)

            actions_internal[state] = error_ids_map

        partition_hm_table_internal = PartitionHMTableInternal(
            actions = actions_internal
        )

        return partition_hm_table_internal


class MemoryBlockInternal(SerializableObject):
    yaml_tag = '!IMemoryBlock'

    copy_slots = [
        "name", # Name of the block. Should be unique in partition.
        "size", # Size of the block. Should be non-negative.
        "align", # Alignment of the block.
        "vaddr", # Virtual address of the block, if required. Otherwise None.
        "paddr", # Physical address of the block, if required. Otherwise None.
        # Cache policy for given memory block. One of:
        #
        # - OFF
        # - COPY_BACK
        # - WRITE_THRU
        # - OFF+COHERENCY
        # - COPY_BACK+COHERENCY
        # - WRITE_THRU+COHERENCY
        # - OFF+GUARDED
        # - COPY_BACK+GUARDED
        # - WRITE_THRU+GUARDED
        # - OFF+GUARDED+COHERENCY
        # - COPY_BACK+GUARDED+COHERENCY
        # - WRITE_THRU+GUARDED+COHERENCY
        # - IO
        # - DEFAULT
        "cache_policy",
        "access", # Access to the memory block from the partition. String contained of 'R', 'W', 'X'.
        "is_contiguous", # Whether block should be *physically* contiguous.

        # Source, from which memory block should be initialized.
        # Supported values:
        #
        # - "ZERO" - fill with zeroes.
        # - "ELF" - fill from the ELF file, which partition is compiled to.
        #
        # If memory block doesn't required initialization, leave this as None.
        "init_source",

        # Stage, when memory block should be initialized:
        #
        # - MODULE - Every time when module is started or restarted
        # - PARTITION - Every time partition is started (either in COLD_START or in WARM_START mode).
        # - PARTITION:COLD - Every time when partition is started in COLD_START mode.
        #
        # If 'init_source' is None this should be None too.
        "init_stage",

        "index", # Index of the memory block in the list.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    @classmethod
    def from_normal(cls, memory_block, index):
        return cls(
            name = memory_block.name,
            size = memory_block.size,
            align = memory_block.align,
            vaddr = memory_block.vaddr,
            paddr = memory_block.paddr,
            cache_policy = memory_block.cache_policy,
            access = memory_block.access,
            is_contiguous = memory_block.is_contiguous,
            init_source = memory_block.init_source,
            init_stage = memory_block.init_stage,
            index = index
        )

class TimeSlotInternal(SerializableObject):
    yaml_tag = '!ITimeSlot'

    copy_slots = [
        'offset',
        'duration',
        'slot_type', # 'SPARE', 'PARTITION', 'MONITOR', 'GDB'
         # If slot_type is 'PARTITION', this is index of linked partition.
         # Otherwise None.
        'partition_index',
         # TRUE if periodic processes are allowed to start in that slot.
         # Have a sence If slot_type is 'PARTITION'. Otherwise None.
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
            partition_index = PartitionInternal.ref_from_normal(localConnection.partition).index,
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


class IPPCPortalTypeInternal(SerializableObject):
    yaml_tag = '!IIPPCPortalType'

    copy_slots = [
        'portal_type_name', # Name of portal type.
        'portal_type_index', # Index of the portal type.
        'server_part_name', # Name of the server partition.
        'server_part_index', # Index of the client partition.
        'portals', # List of portals
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


class IPPCPortalInternal(SerializableObject):
    yaml_tag = '!IIPPCPortal'

    copy_slots = [
        'portal_index', # Index of the portal in portal type.
        'client_part_name', # Name of the client partition.
        'client_part_index', # Index of the client partition.
        'connections_n', # Number of connections
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
