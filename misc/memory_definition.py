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
        "align", # Alignment of the block. 4k by default.

        "vaddr", # Virtual address of the block. May be absent in memory constraints.
        "is_contiguous", # Whether block should be *physically* contiguous.
        "paddr", # If block is *physically* contiguous, it is its physical address. May be absent in memory constraints.
        "cache_policy", # Enumeration.
        "is_coherent", # Modificator for some 'cache_policy' values.
        "is_guarded", # Modificator for some 'cache_policy' values.
        "access", # Access to the memory block. Combination of 'X', 'W', 'R'.

        "is_shared", # Whether memory block is shared between partitions.

        "kaddr", # Kernel address of the block. Do not set in memory constraints
    ]

    default_slots = {
        "vaddr": None,
        "paddr": None,
        "cache_policy": "DEFAULT",
        "is_coherent": False,
        "is_guarded": False,
        "access": "RW",
        "is_contiguous": False,
        "is_elf": False,
        "is_shared": False,
        "kaddr": None,
    }

    def __init__(self, **kargs):
        copy_constructor_with_default(self, kargs)

class PartitionMemoryDefinition(SerializableObject):
    yaml_tag = '!PartitionMemoryDef'

    copy_slots = [
        'name', # For error messages
        'space_id',
        'memory_blocks', # Array of memory blocks.

        'part_elf',

        'elf_size', # Temporary used instead of ELF parsing.
    ]

    def __init__(self, **kargs):
        copy_constructor(self, kargs)


class ModuleMemoryDefinition(SerializableObject):
    yaml_tag = '!ModuleMemoryDef'

    copy_slots = [
        'partitions', # Array of partitions

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
