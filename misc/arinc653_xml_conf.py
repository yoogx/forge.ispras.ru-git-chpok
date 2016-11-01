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

    return int(s) * multiplier

def parse_time(s):
    # note: CHPOK uses ms internally

    if s.endswith("ns"):
        ns = int(s[:-2])
        # see the end of the function
    elif s.endswith("ms"):
        return int(s[:-2])
    elif s.endswith("s"):
        return int(s[:-1]) * (10 ** 3)
    else:
        # assume nanoseconds
        ns = int(s)

    if ns < (10 ** 6):
            raise ValueError("specified time less than 1ms (which won't work due to 1ms timer precision)")
    return ns // (10 ** 6)

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

    def __init__(self, arch):
        self.arch = arch

    def parse_layout(self, root):
        """
        Minimal parsing for extract layout of the module.

        Return list of 'part_layout' objects.
        """
        partitions_layout = []

        for part_root in root.find("Partitions").findall("Partition"):
            part_name = part_root.find("Definition").attrib["Name"]
            part_is_system = False
            if "System" in part_root.find("Definition").attrib:
                part_is_system = parse_bool(part_root.find("Definition").attrib["System"])

            part_layout = chpok_configuration.PartitionLayout(part_name, part_is_system)
            partitions_layout.append(part_layout)

        return partitions_layout

    def parse(self, root):
        """
        Returns chpok_configuration.Configuration object.
        """

        conf = chpok_configuration.Configuration(self.arch)

        for part_root in root.find("Partitions").findall("Partition"):
            self.parse_partition(conf, part_root)

        self.parse_schedule(conf, root.find("Schedule"))

        connection_table = root.find("Connection_Table")
        if connection_table is not None:
            self.parse_channels(conf, connection_table)

        conf.network = self.parse_network(root.find("Network"))

        # Use some default value for module HM table.
        module_error_level_selector_per_state = {error_id: 1 for error_id in conf.error_ids_all }
        for s in ['ERROR_HANDLER', 'USER']:
            conf.module_hm_table.level_selector[s] = module_error_level_selector_per_state

        conf.validate()

        return conf

    def parse_partition(self, conf, part_root):
        part_name = part_root.find("Definition").attrib["Name"]

        part_size = parse_bytes(part_root.find("Memory").attrib["Bytes"])

        # FIXME support partition period, which is simply a fixed attribute
        #       with no real meaning (except it can be introspected)
        part = conf.add_partition(part_name, part_size)

        part.is_system = False
        if "System" in part_root.find("Definition").attrib:
            part.is_system = parse_bool(part_root.find("Definition").attrib["System"])

        # FIXME support partition period, which is simply a fixed attribute
        #       with no real meaning (except it can be introspected)

        part.num_threads = int(part_root.find("Threads").attrib["Count"])

        part.num_arinc653_buffers = int(part_root.find("ARINC653_Buffers").attrib["Count"])
        part.num_arinc653_blackboards = int(part_root.find("ARINC653_Blackboards").attrib["Count"])
        part.num_arinc653_events = int(part_root.find("ARINC653_Events").attrib["Count"])
        part.num_arinc653_semaphores = int(part_root.find("ARINC653_Semaphores").attrib["Count"])

        part.buffer_data_size = parse_bytes(part_root.find("ARINC653_Buffers").attrib["Data_Size"])
        part.blackboard_data_size = parse_bytes(part_root.find("ARINC653_Blackboards").attrib["Data_Size"])

        self.parse_ports(part, part_root.find("ARINC653_Ports"))

        self.parse_hm(part.hm_table, part_root.find("HM_Table"))

    def parse_schedule(self, conf, slot_root):
        for x in slot_root.findall("Slot"):
            slot_type = x.attrib["Type"]

            slot_duration = parse_time(x.attrib["Duration"])
            slot = None

            if slot_type == "Spare":
                slot = chpok_configuration.TimeSlotSpare(slot_duration)
            elif slot_type == "Partition":
                slot_partition = conf.get_partition_by_name(x.attrib["PartitionNameRef"])
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

            conf.add_time_slot(slot)

    def parse_ports(self, part, ports_root):
        for qp in ports_root.findall("Queueing_Port"):
            port_name = qp.attrib["Name"]
            port_direction = qp.attrib["Direction"]
            port_max_message_size = parse_bytes(qp.attrib["MaxMessageSize"])
            port_max_nb_messages = int(qp.attrib["MaxNbMessage"])

            port = chpok_configuration.QueueingPort(port_name, port_direction,
                port_max_message_size, port_max_nb_messages)

            if "Protocol" in qp.attrib:
                port.protocol = qp.attrib["Protocol"]
            else:
                port.protocol = None

            part.add_port_queueing(port)

        for sp in ports_root.findall("Sampling_Port"):
            port_name = sp.attrib["Name"]
            port_direction = sp.attrib["Direction"]
            port_max_message_size = parse_bytes(sp.attrib["MaxMessageSize"])
            port_refresh = parse_time(sp.attrib["Refresh"])

            port = chpok_configuration.SamplingPort(port_name, port_direction,
                port_max_message_size, port_refresh)

            if "Protocol" in sp.attrib:
                port.protocol = sp.attrib["Protocol"]
            else:
                port.protocol = None

            part.add_port_sampling(port)

    def parse_channels(self, conf, channels_root):
        for ch in channels_root.findall("Channel"):

            src = self.parse_connection(conf, ch.find("Source")[0])
            dst = self.parse_connection(conf, ch.find("Destination")[0])

            conf.add_channel(src, dst)

    def parse_connection(self, conf, connection_root):
        if connection_root.tag == "Standard_Partition":
            connection_port = conf.get_port_by_partition_and_name(
                connection_root.attrib["PartitionName"],
                connection_root.attrib["PortName"])

            return chpok_configuration.LocalConnection(connection_port)

        elif connection_root.tag == "UDP":
            res = chpok_configuration.UDPConnection()

            #print('ipaddr: ', root.attrib['IP'])
            res.host = ipaddr.IPAddress(root.attrib["IP"])
            res.port = int(root.attrib["Port"])

            return res
        else:
            raise RuntimeError("unknown connection tag name %r" % connection_root.tag)

    def parse_network(self, root):
        if root is None:
            return None

        res = chpok_configuration.NetworkConfiguration()

        res.ip = ipaddr.IPAddress(root.attrib["IP"])

        #if "MAC" in root.attrib:
        #    res.mac = bytes(int(x, 16) for x in root.attrib["MAC"].split(":"))
        #else:
        #    res.mac = None

        return res

    def parse_hm(self, table, root):
        if root is None:
            return

        table.level_selector['USER'] = {}
        table.actions['USER'] = {}

        for x in root.findall("Error"):
            # Assume "ErrorCode" to be error id
            error_id = x.attrib["ErrorCode"]

            if x.attrib["Level"] == 'PROCESS':
                table.level_selector['USER'][error_id] = 1

            table.actions['USER'][error_id] = x.attrib['Action']

            error_code = x.attrib["Code"].replace('POK_ERROR_KIND_', '')
            error_description = self.error_description_table[error_code]

            table.user_level_codes[error_id] = (error_code, error_description)
