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

import ipaddr
import chpok_configuration

def parse_bool(s):
    # this follows xsd:boolean
    if s in ("true", "1"):
        return True
    if s in ("false", "0"):
        return False

    raise ValueError(s)

def parse_bytes(s):
    multiplier = 1
    if s.endswith("K"):
        multiplier = 2 ** 10
        s = s[:-1]
    elif s.endswith("M"):
        multiplier = 2 ** 20
        s = s[:-1]

    return int(s, 0) * multiplier

def parse_time(s):
    # note: CHPOK uses ms internally

    if s.endswith("ns"):
        ns = int(s[:-2])
        # see the end of the function
    elif s.endswith("ms"):
        ns = int(s[:-2]) * (10 ** 6)
    elif s.endswith("s"):
        ns = int(s[:-1]) * (10 ** 9)
    else:
        # assume nanoseconds
        ns = int(s)

    return ns

class ArincConfigParser:
    # Static map: error code (without prefix) -> error description
    error_description_table = {
        'ILLEGAL_REQUEST': 'Illegal Request',
        'APPLICATION_ERROR': 'Application Error',
        'NUMERIC_ERROR': 'Numeric Error',
        'MEMORY_VIOLATION': 'Memory Violation',
        'DEADLINE_MISSED': 'Deadline Missed',
        'HARDWARE_FAULT': 'Hardware Fault',
        'POWER_FAIL': 'Power Fail',
        'STACK_OVERFLOW': 'Stack Overflow',
        'PARTITION_CONFIGURATION': 'Config Error',
    }

    def parse(self, root):
        """
        Returns chpok_configuration.Configuration object.
        """

        conf = chpok_configuration.Configuration()

        conf_private = dict()
        # Mapping name => partition.
        partitions_by_name = dict()

        conf_private['partitions_by_name'] = partitions_by_name
        conf.private_data = conf_private

        for index, part_root in enumerate(root.find("Partitions").findall("Partition")):
            part = self.parse_partition(part_root, str(index))
            part_name = part.name
            conf.partitions.append(part)
            partitions_by_name[part_name] = part

        self.parse_schedule(conf, root.find("Schedule"))

        connection_table = root.find("Connection_Table")
        if connection_table is not None:
            self.parse_channels(conf, connection_table)

        conf.module_hm_table.default_action = chpok_configuration.ModuleHMAction("MODULE", "SHUTDOWN")
        # Use some default action for module HM table for partition-level errors.
        action_partition = chpok_configuration.ModuleHMAction("PARTITION", "SHUTDOWN")

        module_hm_actions_per_state = {error_id: action_partition for error_id in chpok_configuration.HMTable.error_ids }
        for s in ['ERROR_HANDLER', 'USER']:
            conf.module_hm_table.actions[s] = module_hm_actions_per_state

        self.parse_shared_memory_blocks(conf, root.find("Shared_Memory_Blocks"))

        portal_types_root = root.find("Portal_Types")

        if portal_types_root is not None:
            for portal_type_entry in portal_types_root.findall("Portal_Type"):
                portal_type = self.__class__.parse_portal_type(conf, portal_type_entry)
                conf.portal_types.append(portal_type)

        return conf

    def parse_partition(self, part_root, part_id = "0"):
        """
        Return chpok_configuration.Partition object

        'part_id' meaningfull only when configure the whole module.
        """

        part_name = part_root.find("Definition").attrib["Name"]

        part = chpok_configuration.Partition(part_id, part_name)

        part.memory_size = parse_bytes(part_root.find("Memory").attrib["Bytes"])
        part.heap_size = parse_bytes(part_root.find("Memory").attrib.get('Heap', default='0'))

        # FIXME support partition period, which is simply a fixed attribute
        #       with no real meaning (except it can be introspected)
        part.is_system = False
        if "System" in part_root.find("Definition").attrib:
            part.is_system = parse_bool(part_root.find("Definition").attrib["System"])

        # FIXME support partition period, which is simply a fixed attribute
        #       with no real meaning (except it can be introspected)

        part.num_threads = int(part_root.find("Threads").attrib["Count"])

        # Reserve 8K stack for every thread, including main and error ones
        part.stack_size_all = (part.num_threads + 2) * 8096

        part.num_arinc653_buffers = int(part_root.find("ARINC653_Buffers").attrib["Count"])
        part.num_arinc653_blackboards = int(part_root.find("ARINC653_Blackboards").attrib["Count"])
        part.num_arinc653_events = int(part_root.find("ARINC653_Events").attrib["Count"])
        part.num_arinc653_semaphores = int(part_root.find("ARINC653_Semaphores").attrib["Count"])

        part.buffer_data_size = parse_bytes(part_root.find("ARINC653_Buffers").attrib["Data_Size"])
        part.blackboard_data_size = parse_bytes(part_root.find("ARINC653_Blackboards").attrib["Data_Size"])

        part_private = dict()

        # Map name=>port.
        part_private['ports_by_name'] = dict()
        # Map name=>memory_block.
        part_private['mb_by_name'] = dict()
        part.private_data = part_private

        self.parse_ports(part, part_root.find("ARINC653_Ports"))

        self.parse_hm(part.hm_table, part_root.find("HM_Table"))

        self.parse_partition_memory_blocks(part, part_root.find("Memory_Blocks"))

        return part

    def parse_schedule(self, conf, slot_root):
        for x in slot_root.findall("Slot"):
            slot_type = x.attrib["Type"]

            slot_duration = parse_time(x.attrib["Duration"])
            slot = None

            if slot_type == "Spare":
                slot = chpok_configuration.TimeSlotSpare(slot_duration)
            elif slot_type == "Partition":
                slot_partition = conf.private_data['partitions_by_name'][x.attrib["PartitionNameRef"]]
                slot_periodic_processing_start = parse_bool(x.attrib["PeriodicProcessingStart"])

                slot = chpok_configuration.TimeSlotPartition(slot_duration,
                    slot_partition, slot_periodic_processing_start)
            elif slot_type == "Network":
                slot = chpok_configuration.TimeSlotNetwork(slot_duration)
            elif slot_type == "Monitor":
                slot = chpok_configuration.TimeSlotMonitor(slot_duration)
            elif slot_type == "GDB":
                slot = chpok_configuration.TimeSlotGDB(slot_duration)
            else:
                raise ValueError("unknown slot type %r" % slot_type)

            conf.slots.append(slot)

    def parse_ports(self, part, ports_root):
        part_private = part.private_data
        ports_by_name = part_private['ports_by_name']

        for qp in ports_root.findall("Queueing_Port"):
            port_name = qp.attrib["Name"]
            port_direction = qp.attrib["Direction"]
            port_max_message_size = parse_bytes(qp.attrib["MaxMessageSize"])
            port_max_nb_messages = int(qp.attrib["MaxNbMessage"])

            port = chpok_configuration.QueueingPort(port_name, port_direction,
                port_max_message_size, port_max_nb_messages)

            part.ports_queueing.append(port)

            ports_by_name[port_name] = port

        for sp in ports_root.findall("Sampling_Port"):
            port_name = sp.attrib["Name"]
            port_direction = sp.attrib["Direction"]
            port_max_message_size = parse_bytes(sp.attrib["MaxMessageSize"])
            port_refresh = parse_time(sp.attrib["Refresh"])

            port = chpok_configuration.SamplingPort(port_name, port_direction,
                port_max_message_size, port_refresh)

            part.ports_sampling.append(port)

            ports_by_name[port_name] = port

    def parse_channels(self, conf, channels_root):
        for ch in channels_root.findall("Channel"):

            src = self.parse_connection(conf, ch.find("Source")[0])
            dst = self.parse_connection(conf, ch.find("Destination")[0])

            channel = chpok_configuration.Channel(src, dst)

            conf.channels.append(channel)

    def parse_connection(self, conf, connection_root):
        if connection_root.tag == "Standard_Partition":
            part_name = connection_root.attrib["PartitionName"]
            part = conf.private_data['partitions_by_name'][part_name]
            port_name = connection_root.attrib["PortName"]

            port = part.private_data['ports_by_name'][port_name]

            return chpok_configuration.LocalConnection(part, port)

        else:
            raise RuntimeError("unknown connection tag name %r" % connection_root.tag)

    def parse_partition_memory_blocks(self, part, memory_blocks_xml):
        if memory_blocks_xml is None:
            return
        for memory_block_xml in memory_blocks_xml.findall("Memory_Block"):
            memory_block = self.parse_memory_block(memory_block_xml)
            part.memory_blocks.append(memory_block)
            part.private_data['mb_by_name'][memory_block.name] = memory_block

    def parse_memory_block(self, mroot):
        name = mroot.attrib["Name"]
        size = parse_bytes(mroot.attrib["Size"])

        mblock = chpok_configuration.MemoryBlock(name, size)

        mblock.access = mroot.attrib.get("Access", "RW")

        vaddr = mroot.attrib.get("VirtualAddress", None)
        if vaddr is not None:
            mblock.vaddr = int(vaddr, 0)

        is_contiguous = mroot.attrib.get("Contiguous", None)
        if is_contiguous is not None:
            mblock.is_contiguous = parse_bool(is_contiguous)

        paddr = mroot.attrib.get("PhysicalAddress", None)
        if paddr is not None:
            mblock.paddr = int(paddr, 0)
            # Enforce block to be contiguous
            mblock.is_contiguous = True

        cache_policy = mroot.attrib.get("CachePolicy", None)
        if cache_policy is not None:
            mblock.cache_policy = cache_policy

        init_source = mroot.attrib.get("InitSource", None)
        init_stage = mroot.attrib.get("InitStage", None)

        if init_source is not None:
            if not init_source in mblock.init_source_values:
                print "ERROR: Memory block '%s' has incorrect attribute 'InitSource': '%s'." % (name, init_source)
                print "HINT: Possible values are: " + ", ".join(mblock.init_source_values) + "."
                raise RuntimeError("Incorrect memory block definition.")

            mblock.init_source = init_source

            if init_stage is None:
                print "Memory block '%s' has attribute 'InitSource' without 'InitStage' one." % (name)
                raise RuntimeError("Incorrect memory block definition.")

            if not init_stage in mblock.init_stage_values:
                print "ERROR: Memory block '%s' has incorrect attribute 'InitStage': '%s'" % (name, init_stage)
                print "HINT: Possible values are: " + ", ".join(mblock.init_stage_values) + "."
                raise RuntimeError("Incorrect memory block definition.")

            mblock.init_stage = init_stage
        else:
            if init_stage is not None:
                print "Memory block '%s' has attribute 'InitStage' without 'InitSource' one." % (name)
                raise RuntimeError("Incorrect memory block definition.")

        # TODO: Parse other attributes.

        return mblock

    def parse_shared_memory_blocks(self, conf, sroot):
        if sroot is None:
            return

        for share_entry in sroot.findall('Shared_Memory_Blocks_Entry'):
            mb_sharing = chpok_configuration.MemoryBlockSharing()
            for mb_ref_xml in share_entry.findall('Memory_Block_Ref'):
                mb_sharing.mb_refs.append(self.parse_memory_block_ref(conf, mb_ref_xml))

            conf.memory_block_sharings.append(mb_sharing)

    def parse_memory_block_ref(self, conf, ref_entry):
        part_name = ref_entry.attrib["PartitionNameRef"]
        part = conf.private_data['partitions_by_name'][part_name]
        mb_name = ref_entry.attrib["NameRef"]
        memory_block = part.private_data['mb_by_name'][mb_name]

        return chpok_configuration.MemoryBlockRef(part, memory_block)

    def parse_hm(self, table, root):
        table.default_action = chpok_configuration.PartitionHMAction("PARTITION", "IDLE")

        if root is None:
            return

        table.actions['USER'] = {}

        for x in root.findall("Error"):
            # Assume "ErrorCode" to be error id
            error_id = x.attrib["ErrorCode"]

            level = x.attrib["Level"]
            recovery_action = x.attrib['Action']

            error_code = x.attrib["Code"].replace('POK_ERROR_KIND_', '')
            description = self.error_description_table[error_code]

            action = chpok_configuration.PartitionHMAction(level, recovery_action, error_code, description)

            table.actions['USER'][error_id] = action

    @classmethod
    def parse_portal_type(cls, conf, portal_type_entry):
        portal_type_name = portal_type_entry.attrib["PortalTypeName"]
        server_part_name = portal_type_entry.attrib["ServerPartitionName"]
        server_part = conf.private_data['partitions_by_name'][server_part_name]

        portal_type = chpok_configuration.IPPCPortalType(portal_type_name, server_part)

        for portal_entry in portal_type_entry.findall("Portal"):
            portal = cls.parse_portal(conf, portal_entry)
            portal_type.portals.append(portal)

        return portal_type

    @classmethod
    def parse_portal(cls, conf, portal_entry):
        client_part_name = portal_entry.attrib["ClientPartitionName"]
        client_part = conf.private_data['partitions_by_name'][client_part_name]
        connections_n = int(portal_entry.attrib["ConnectionsNumber"])

        portal = chpok_configuration.IPPCPortal(client_part, connections_n)

        return portal
