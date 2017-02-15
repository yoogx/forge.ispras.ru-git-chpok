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
Simple allocator of memory blocks, which can be used by env['BOARD_DEPLOY'] function.

1. Define class for TLB entry, which describes mapping of memory region
from virtual space to physical one.

2. Extend 'PhysMemoryBlock', so it will reflect block in physical memory,
for which TLB entry can be created.

3. Extend 'PhysMemoryType' which defines possible cache policies for some
physical area and creation of 'PhysMemoryBlock' objects.

4. Declare regions of physical memory, in which memory blocks with non-fixed
physical addresses can be allocated.
For each such region 'PhysMemoryType' object should be binded.

5. For memory block constraints call allocateMemoryBlocksSimple().
This function will transform constraints into definitions and return
per-partition lists of TLB entries. (It returns list of 'PartitionTLBEntries' objects).
"""

from __future__ import division

import abc
import memory_definition

def align_val(val, align):
    return ((val + align - 1) // align) * align


class PhysMemoryBlock:
    """
    Memory block in physical memory, to which normal memory block of
    suitable size can be mapped.
    """
    __metaclass__ = abc.ABCMeta

    __slots__ = [
        'paddr',
        'size',
        'align', # minimal alignment for virtual address.
    ]

    def __init__(self, paddr, size, align):
        self.paddr = paddr
        self.size = size
        self.align = align

    @abc.abstractmethod
    def createTLBEntry(self, vaddr, access, cache_policy, space_id):
        """
        Create object which maps given virtual address to this physical
        block.

        'vaddr' is garanteed to be aligned on '.align'.

        If entry cannot be created, exception should raised.
        """
        pass

class PhysMemoryType:
    """
    Describe type of physical memory in how memory blocks can be created
    in it.
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def isCacheSupported(self, cache_policy):
        """
        Check whether given cache_policy is supported for this memory.
        """
        pass

    @abc.abstractmethod
    def allocPhysBlock(self, paddr, size):
        """
        Create physical memory block at the lowest physical address
        no less than 'paddr'.

        Return None if block cannot be allocated (because of parameters).
        """
        pass

class PhysSegment:
    """
    Range of addresses in physical memory of given type.
    """
    __slots__ = [
        'paddr_start', # Range is [paddr_start, paddr_end)
        'paddr_end',
        'memory_type', # Object of type 'PhysMemoryType'
    ]

    def __init__(self, paddr_start, paddr_end, memory_type):
        self.paddr_start = paddr_start
        self.paddr_end = paddr_end
        self.memory_type = memory_type

class PartitionTLBEntries:
    """
    TLB entries allocated for partition.
    """
    slots = [
        'name', # Name of the partition
        'entries' # List of TLB entries
    ]

    def __init__(self, name):
        self.name = name
        self.entries = []


def allocateMemoryBlocksSimple(memory_constraints, phys_segments,
        memory_type_default, vaddr_start_all):
    """
    Create TLB entries for given memory_constraints, allocating
    them from given list of physical segments.

    'memory_type_default' - PhysMemoryType object, which will be used
    for create physical blocks for memory blocks with fixed physical addresses.
    (these addresses should be outside of 'phys_segments').
    You may set this parameter to None, so fixed
    physical addresses will be rejected.

    'vaddr_start_all' - beginning of virtual address space for every partition.

    After the call 'memory_constrains' object will contain memory
    *definitions*.

    Return list of 'PartitionTLBEntries' objects with '.entries' filled.
    """
    pb_allocator = _PhysBlockAllocator(phys_segments, memory_type_default)
    # Memory blocks reserved by sharing.
    # Map (part_name) * (mb_name) => PhysMemoryBlock
    shared_mb = {pmc.name: {} for pmc in memory_constraints.partitions}

    # Process sharings and allocate physical memory for it.
    for i, mb_sharing in enumerate(memory_constraints.memory_block_sharings):
        first_mb_ref = mb_sharing.mb_refs[0]
        first_mb = memory_constraints.partitions[first_mb_ref.part_index].memory_blocks[first_mb_ref.mb_index]

        # Compute all information required for physical memory block creation.
        size = first_mb.size
        align = first_mb.align
        cache_policies = set()
        paddr = first_mb.paddr

        for mb_ref in mb_sharing.mb_refs:
            mb = memory_constraints.partitions[mb_ref.part_index].memory_blocks[mb_ref.mb_index]

            if mb.paddr != paddr:
                print "ERROR: Sharing %d has memory blocks of different physical address constraints." % i
                raise RuntimeError("Incorrect memory blocks sharing")

            if mb.size != size:
                print "ERROR: Sharing %d has memory blocks of different sizes." % i
                raise RuntimeError("Incorrect memory blocks sharing")

            if mb.align != align:
                print "ERROR: Sharing %d has memory blocks of different alignments." % i
                raise RuntimeError("Incorrect memory blocks sharing")

            cache_policies.add(mb.cache_policy)

        if size == 0:
            print "ERROR: Zero-sized memory blocks cannot be shared."
            raise RuntimeError("Incorrect memory blocks sharing")

        # Create physical memory block.
        if paddr is None:
            pb = pb_allocator.allocPhysBlock(size, align, cache_policies)
        else:
            pb = pb_allocator.allocPhysBlockFixed(paddr, size, cache_policies)

        # Mark all shared blocks as needed none physical allocation.
        for mb_ref in mb_sharing.mb_refs:
            shared_mb[mb_ref.part_name][mb_ref.mb_name] = pb

    partitionsTLB = [] # Returned list

    for pmc in memory_constraints.partitions:
        partitionTLB = PartitionTLBEntries(pmc.name)

        # Organize memory blocks into 6 groups:
        #
        # 0. size is 0.
        # 1. vaddr is not None, paddr is None, not shared
        # 2. vaddr is None, paddr is None, not shared
        # 3. vaddr is None, paddr is not None, not shared
        # 4. vaddr is None, paddr is None, shared
        # 5. vaddr is None, paddr is not None, shared
        #
        # (All other combinations of vaddr, paddr and sharing are prohibited).
        #
        # Group 0 doesn't require allocation.
        # Group 1 come into single physical block.
        # Groups 2,3,4,5 come into one physical block per memory block definition.
        mb_zero = []
        mb_vfixed = []
        mb_float = []
        mb_shared = []
        mb_pfixed = []
        mb_pfixed_shared = []

        # Create groups
        for mbd in pmc.memory_blocks:
            if mbd.size == 0:
                mb_zero.append(mbd)
            elif mbd.vaddr is not None:
                # Group 1
                if mbd.paddr is not None:
                    print "ERROR: Partition '%s' has memory block '%s' with fixed physical and virtual addresses. This feature is not supported." % (pmc.name, mbd.name)
                    raise RuntimeError("Unsupported memory block")

                if mbd.name in shared_mb[pmc.name]:
                    print "ERROR: Partition '%s' has memory block '%s' with fixed virtual address shared with other partitions. This feature is not supported." % (pmc.name, mbd.name)
                    raise RuntimeError("Unsupported memory block")

                if mbd.cache_policy != "DEFAULT":
                    print "ERROR: Partition '%s' has non-shared memory block '%s' without fixed physical address and non-default cache policy. This feature is not supported." % (pmc.name, mbd.name)
                    raise RuntimeError("Unsupported memory block")

                mb_vfixed.append(mbd)
            else:
                if mbd.paddr is None:
                    if not mbd.name in shared_mb[pmc.name]:
                        # Group 2
                        mb_float.append(mbd)
                    else:
                        # Group 3
                        mb_shared.append(mbd)
                else: # paddr is not None
                    if not mbd.name in shared_mb[pmc.name]:
                        # Group 4
                        mb_pfixed.append(mbd)
                    else:
                        # Group 5
                        mb_pfixed_shared.append(mbd)


        vb_allocator = _VirtBlockAllocator(vaddr_start_all)

        # Group 0 is the simplest:
        # Set its virtual and physical addresses to 0 (if required).
        for mbd in mb_zero:
            if mbd.vaddr is None:
                mbd.vaddr = 0
            if mbd.is_contiguous and mbd.paddr is None:
                mbd.paddr = 0
            mbd.kaddr = 0

        # Process group 1.
        if len(mb_vfixed) == 0:
            print "ERROR: There is no memory blocks with fixed virtual address. Where ELF should be loaded?"
            raise RuntimeError("Incorrect setup for memory blocks with virtual address fixed")

        align = 1
        for mbd in mb_vfixed:
            if mbd.align > align:
                align = mbd.align

        mb_vfixed.sort(key=lambda mb: mb.vaddr)

        mb_vfixed_first = mb_vfixed[0]
        mb_vfixed_last = mb_vfixed[-1]

        if mb_vfixed_first.vaddr < vaddr_start_all:
            print "ERROR: Partition '%s' has memory block '%s' with fixed virtual address %x less than minimal one %x." % (pmc.name, mbd.name, mbd.vaddr, vaddr_start)
            raise RuntimeError("Incorrect setup for memory blocks with virtual address fixed")

        vaddr_start = vaddr_start_all
        vaddr_end = mb_vfixed_last.vaddr + mb_vfixed_last.size

        # All memory blocks with fixed virtual address should have same cache policy.
        cache_policy = mb_vfixed_first.cache_policy

        for mbd in mb_vfixed:
            if mbd.cache_policy != cache_policy:
                print "ERROR: Partition '%s' has memory block '%s' with fixed virtual address and cache policy '%s', which differs from other blocks in that group. This feature is not supported." % (pmc.name, mbd.name, mbd.cache_policy)
                raise RuntimeError("Incorrect setup for memory blocks with virtual address fixed")

        pb = pb_allocator.allocPhysBlock(vaddr_end - vaddr_start, align, {cache_policy})

        if vaddr_start % pb.align:
            print "ERROR: Board start virtual address 0x%x doesn't satisfy alignment 0x%x." (vaddr_start, pb.align)
            print "HINT: Currently all memory blocks with fixed virtual addresses are organized into single physical memory block."
            raise RuntimeError("Incorrect setup for memory blocks with virtual address fixed")

        vb = vb_allocator.allocVirtBlockFixed(pb, vaddr_start)

        for mbd in mb_vfixed:
            vb.setup_memblock(mbd)

        te = pb.createTLBEntry(vb.vaddr, "RWX", cache_policy, pmc.space_id)
        partitionTLB.entries.append(te)

        # Process group 2.
        for mbd in mb_float:
            pb = pb_allocator.allocPhysBlock(mbd.size, mbd.align, {mbd.cache_policy})

            vb = vb_allocator.allocVirtBlock(pb, mbd.align)
            vb.setup_memblock(mbd)

            te = pb.createTLBEntry(vb.vaddr, mbd.access, mbd.cache_policy, pmc.space_id)
            partitionTLB.entries.append(te)

        # Process group 3.
        for mbd in mb_pfixed:
            pb = pb_allocator.allocPhysBlockFixed(mbd.paddr, mbd.size, {mbd.cache_policy})

            vb = vb_allocator.allocVirtBlock(pb, mbd.align)
            vb.setup_memblock(mbd)

            te = pb.createTLBEntry(vb.vaddr, mbd.access, mbd.cache_policy, pmc.space_id)
            partitionTLB.entries.append(te)

        # Process group 4 and 5: For both of them physical blocks are already created.
        for mbd in mb_shared + mb_pfixed_shared:
            pb = shared_mb[pmc.name][mbd.name]

            vb = vb_allocator.allocVirtBlock(pb, mbd.align)
            vb.setup_memblock(mbd)

            te = pb.createTLBEntry(vb.vaddr, mbd.access, mbd.cache_policy, pmc.space_id)
            partitionTLB.entries.append(te)

        partitionsTLB.append(partitionTLB)

    return partitionsTLB

class _VirtMemoryBlock:
    """
    Memory block in virtual memory, to which normal memory block of
    suitable size can be mapped.
    """

    __slots__ = [
        'vaddr',
        'pb', # Physical block to which virtual one corresponds
    ]

    def __init__(self, vaddr, pb):
        self.vaddr = vaddr
        self.pb = pb

    def setup_memblock(self, mb):
        """
        Perform needed setup for memory block constraint, so it will be
        memory block definition.
        """
        if mb.vaddr is None:
            mb.vaddr = self.vaddr
        else:
            assert mb.vaddr >= self.vaddr

        assert mb.vaddr + mb.size <= self.vaddr + self.pb.size

        # Assume kaddr is always same as vaddr
        mb.kaddr = mb.vaddr

        if not mb.is_contiguous:
            return

        if mb.paddr is None:
            mb.paddr = self.pb.paddr + (self.vaddr - mb.vaddr)



class _VirtBlockAllocator:
    """
    Simple allocator of virtual memory blocks.
    """
    __slots__ = [
        'vaddr_start', # Start of the virtual memory for partition.
        'vaddr_current'
    ]

    def __init__(self, vaddr_start):
        self.vaddr_start = vaddr_start
        self.vaddr_current = vaddr_start

    def allocVirtBlock(self, pb, align):
        """
        Alloc virtual block according to physical one
        """
        if align < pb.align:
            align = pb.align

        vaddr_block_start = align_val(self.vaddr_current, align)

        self.vaddr_current = vaddr_block_start + pb.size

        return _VirtMemoryBlock(vaddr_block_start, pb)

    def allocVirtBlockFixed(self, pb, vaddr):
        """
        Alloc virtual block according to physical one.
        Accept virtual address of the block created.
        """
        # In simple form, we require this function to be called before
        # any other allocations, and 'vaddr' should be equal to 'vaddr_start'.
        assert self.vaddr_start == self.vaddr_current
        assert self.vaddr_start == vaddr
        assert (vaddr % pb.align) == 0

        self.vaddr_current = vaddr + pb.size

        return _VirtMemoryBlock(vaddr, pb)


class _PhysSegmentBlockAllocator:
    """
    Allocator of memory blocks within single physical segment.
    """
    __slots__ = [
        'paddr_start', # Range is [paddr_start, paddr_end)
        'paddr_end',
        'memory_type', # Object of type 'PhysMemoryType'
        'paddr_current', # First unused address.
    ]

    def __init__(self, phys_segment):
        self.paddr_start = phys_segment.paddr_start
        self.paddr_end = phys_segment.paddr_end
        self.memory_type = phys_segment.memory_type
        self.paddr_current = phys_segment.paddr_start

    def allocPhysBlock(self, size, align):
        """
        Allocate block in physical memory. Return None if cannot.

        'align' is alignment in *both* physical and virtual memory.
        """
        phys_memblock = self.memory_type.allocPhysBlock(align_val(self.paddr_current, align), size)

        if phys_memblock is None:
            return None

        memblock_paddr_end = phys_memblock.paddr + phys_memblock.size
        if memblock_paddr_end > self.paddr_end:
            return None

        self.paddr_current = memblock_paddr_end

        return phys_memblock

    def isCacheSupported(self, cache_policy):
        """
        Return True if given cache policy is supported.
        """
        return self.memory_type.isCacheSupported(cache_policy)


    def isAllCacheSupported(self, cache_policies):
        """
        Return True if *all* cache policies in the given set are supported.
        """
        for cache_policy in cache_policies:
            if not self.isCacheSupported(cache_policy):
                return False

        return True


class _PhysBlockAllocator:
    """
    Simple allocator of physical memory blocks.
    """

    __slots__ = [
        'segment_allocators', # List of '_PhysSegmentBlockAllocator' objects.
        'memory_type_default', # Memory allocator for regions outside of segments.
    ]

    def __init__(self, phys_segments, memory_type_default):
        self.segment_allocators = [_PhysSegmentBlockAllocator(segment) for segment in phys_segments]
        self.memory_type_default = memory_type_default

    def isCacheSupported(self, cache_policy):
        """
        Check that at least one physical segment may have memory blocks
        with given cache policy.
        """
        for segment_allocator in self.segment_allocators:
            if segment_allocator.isCacheSupported(cache_policy):
                return True

            return False

    def allocPhysBlock(self, size, align, cache_policies):
        """
        Allocate physical memory block of given parameters suitable for
        any cache policy in the array.

        If block cannot be allocated, exception will be raised.

        'align' is alignment in *both* physical and virtual memory.
        """
        # Whether given combination of cache policies is supported at least by one memory region.
        caches_are_supported = False
        for segment_allocator in self.segment_allocators:
            if not segment_allocator.isAllCacheSupported(cache_policies):
                continue

            caches_are_supported = True
            phys_memblock = segment_allocator.allocPhysBlock(size, align)
            if not phys_memblock:
                continue

            return phys_memblock

        # Try to detect reason of failure
        if not caches_are_supported:
            for cache_policy in cache_policies:
                if not self.isCacheSupported(cache_policy):
                    print "ERROR: Cache policy '%s' is not supported on given board." % cache_policy
                    raise RuntimeError('Unsupported cache policy')
            print "ERROR: None memory region supports combination of cache policies: '" + "', '".join(cache_policies) + "'."
            raise RuntimeError('Unsupported cache policies combination')

        print "ERROR: Cannot allocate memory block of size '%d'." % size
        print "HINT: Probably, you need more physical memory for that."
        raise RuntimeError('Cannot allocate memory block')

    def allocPhysBlockFixed(self, paddr, size, cache_policies):
        """
        Allocate physical memory block *at given address* with given
        parameters suitable for any cache policy in the set.

        If block cannot be allocated at given address, exception will be raised.
        """
        # Only blocks outside of physical regions are supported.
        for segment_allocator in self.segment_allocators:
            if segment_allocator.paddr_end <= paddr:
                continue
            if segment_allocator.paddr_start >= paddr + size:
                continue
            print "ERROR: Cannot create memory block [0x%x; 0x%x) which *intercepts* range [0x%x; 0x%x]."
            raise RuntimeError("Physically-fixed memory blocks cannot be created in addresses suitable for allocate non-fixed blocks.")

        if self.memory_type_default is None:
            print "ERROR: Memory blocks with fixed addresses are not supported for given board."
            raise RuntimeError("Physically-fixed memory blocks are not supported")

        # Allocate block using default memory type
        for cache_policy in cache_policies:
            if not self.memory_type_default.isCacheSupported(cache_policy):
                print "ERROR: Cache policy '%s' is not supported for memory blocks with fixed physical addresss."
                raise RuntimeError("Cannot create memory block at fixed physical address")

        pb = self.memory_type_default.allocPhysBlock(paddr, size)
        if pb is None:
            print "ERROR: Cannot create memory block with physical address 0x%x." % paddr
            raise RuntimeError("Cannot create memory block at fixed physical address")

        if pb.paddr != paddr:
            print "ERROR: Cannot create memory block of size %x *exactly* at requested physical address 0x%x." % (size, paddr)
            raise RuntimeError("Cannot create memory block at fixed physical address")

        if pb.size != size:
            print "ERROR: Cannot create memory block with physical address 0x%x *exactly* of requested size 0x%x." % (paddr, size)
            raise RuntimeError("Cannot create memory block at fixed physical address")

        return pb
