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
Definition of the memory from the core(arch-independent) point of view.

There are two types of such definition:

1. In form of memory _constraints_.

2. Complete definition.

When some part isn't need to be filled in the first form
(memory constraints) it will be noted explicitely.
"""

from text_serialization import *

class MemoryBlockDefinition(SerializableObject):
    yaml_tag = '!MemoryBlockDef'

    copy_slots = [
        "name", # Name of the block. Should be unique in partition.
        "size", # Size of the block. Should be non-negative.
        "align", # Alignment of the block.

        "vaddr", # Virtual address of the block. May be absent in memory constraints.
        "is_contiguous", # Whether block should be *physically* contiguous.
        "paddr", # If block is *physically* contiguous, it is its physical address. May be absent in memory constraints.
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

        "kaddr", # Kernel address of the block. Do not set in memory constraints

        "index", # Index of the memory block in the list. May be set upon part.add_memory_block() call.
    ]

    default_slots = {
        "vaddr": None,
        "paddr": None,
        "cache_policy": "DEFAULT",
        "access": "RW",
        "is_contiguous": False,
        "init_source": None,
        "init_stage": None,
        "kaddr": None,
        "index": None,
    }

    def __init__(self, **kargs):
        copy_constructor_with_default(self, kargs)

    @classmethod
    def from_config(cls, memory_block):
        param_dict = {}
        for slot in cls.copy_slots:
            if slot == "kaddr":
                value = None
            else:
                value = getattr(memory_block, slot)
            param_dict[slot] = value
        return cls(**param_dict)

class PartitionMemoryDefinition(SerializableObject):
    yaml_tag = '!PartitionMemoryDef'

    copy_slots = [
        'name', # For error messages
        'space_id',
        'memory_blocks', # Array of memory blocks.

        'memory_size', # Temporary used instead of ELF parsing.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    def add_memory_block(self, memory_block):
        memory_block.index = len(self.memory_blocks)

        self.memory_blocks.append(memory_block)


class MemoryBlockSharingDefinition(SerializableObject):
    """
    Sharing physical space by one or more memory blocks.
    """
    yaml_tag = '!MemoryBlockSharingDef'

    copy_slots = [
        'mb_refs', # List of 'MemoryBlockRefInternal' objects
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


class MemoryBlockRefDefinition(SerializableObject):
    """
    Reference to memory block from outsize of any partition.
    """
    yaml_tag = '!MemoryBlockRefDef'

    copy_slots = [
        'part_name',
        'part_index',
        'mb_name',
        'mb_index',
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


class ModuleMemoryDefinition(SerializableObject):
    yaml_tag = '!ModuleMemoryDef'

    copy_slots = [
        'partitions', # Array of partitions

        'memory_block_sharings', # Array of MemoryBlockSharingDefinition objects

        'phys_total', # Total physical memory available.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    # Serialization/deserialization methods
    def save_to_file(self, filename):
        serialize_object_as_text(self, filename)

    @classmethod
    def load_from_file(cls, filename):
        return deserialize_object_from_text(cls, filename)
