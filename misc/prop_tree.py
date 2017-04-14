#******************************************************************
#
# Institute for System Programming of the Russian Academy of Sciences
# Copyright (C) 2017 ISPRAS
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
Serialization and writting of the property tree based on python data.
"""

from __future__ import division
import abc
import collections
import struct
import os
import shutil

from text_serialization import *

class PropertyTreeBinaryNode(SerializableObject):
    yaml_tag = '!BinaryNode'

    copy_slots = [
        'value', # Immediate value of the node, if given.
        'filename', # File with value of the node, if given.
        'align', # Alignment of the value
    ]

    default_slots = {
        'value': None,
        'filename': None
    }

    def __init__(self, **kargs):
        copy_constructor_with_default(self, kargs)

        # At least one of 'value' and 'filename' should be given
        assert(self.value is not None or self.filename is not None)


class _PropertyTreeWriter:
    __slots__ = [
        'is_big_endian', # True for big endian False for little endian
        'byte_order_format', # Format character corresponded to byte order.
    ]

    def __init__(self, is_big_endian):
        self.is_big_endian = is_big_endian
        if self.is_big_endian:
            self.byte_order_format = '>'
        else:
            self.byte_order_format = '<'

class _PropertyTreeNodeValueBase:

    # Types constants.
    JET_PT_STRING = 0
    JET_PT_INTEGER32 = 1
    JET_PT_INTEGER64 = 2
    JET_PT_UNSIGNED32 = 3
    JET_PT_UNSIGNED64 = 4
    JET_PT_FLOAT = 5
    JET_PT_DOUBLE = 6
    JET_PT_BINARY = 7
    JET_PT_TREE = 8

    JET_PT_INVALID_NODE = 0xffffffff # (-1) as uint32_t

    @abc.abstractmethod
    def get_type(self):
        """
        Return value type.
        """
        pass

    @abc.abstractmethod
    def get_value_bytes(self, tree_writer):
        """
        Return 8 bytes represented the value.
        """
        pass


class _PropertyTreeStringRecord:
    __slots__ = [
        'value', # String
        'offset', # Offset in the block of strings.
    ]

    def __init__(self, value, offset):
        self.value = value
        self.offset = offset

class _PropertyTreeNodeValueString(_PropertyTreeNodeValueBase):
    __slots__ = [
        'string_record',
    ]

    def __init__(self, string_record):
        self.string_record = string_record

    def get_type(self):
        return self.JET_PT_STRING

    def get_value_bytes(self, tree_writer):
        return struct.pack(tree_writer.byte_order_format + "II", self.string_record.offset, 0)

class _PropertyTreeBinaryRecord:
    __slots__ = [
        'value', # Immediate value, if given.
        'filename', # File with value, if given.
        'size', # Size of the value

        'offset', # Offset in the block of strings.
    ]

    def __init__(self, binary_node, offset):
        self.value = binary_node.value
        self.filename = binary_node.filename
        self.offset = offset

        if self.value is not None:
            self.size = len(self.value)
        else:
            self.size = os.path.getsize(self.filename)

class _PropertyTreeNodeValueBinary(_PropertyTreeNodeValueBase):
    __slots__ = [
        'binary_record',
    ]

    def __init__(self, binary_record):
        self.binary_record = binary_record

    def get_type(self):
        return self.JET_PT_BINARY

    def get_value_bytes(self, tree_writer):
        return struct.pack(tree_writer.byte_order_format + "II", self.binary_record.offset, self.binary_record.size)

class _PropertyTreeNodeValueTree(_PropertyTreeNodeValueBase):
    __slots__ = [
        'n_children',
    ]

    def __init__(self, n_children, first_child):
        self.n_children = n_children
        self.first_child = first_child

    def get_type(self):
        return self.JET_PT_TREE

    def get_value_bytes(self, tree_writer):
        if self.first_child is not None:
            first_child_index = self.first_child.index
        else:
            first_child_index = JET_PT_INVALID_NODE
        return struct.pack(tree_writer.byte_order_format + "II", self.n_children, first_child_index)

def align_val(val, align):
    return ((val + align - 1) // align) * align

class _PropertyTreeNodeRecord:
    FLAG_HAS_NEXT_SIBLING = 1

    __slots__ = [
        'index', # Index in the block of nodes.
        'name_record', # String record described node's name.
        'has_next_sibling', # Whether more siblings exists after the node.
        'value', # Type-dependent value
    ]

    def __init__(self, index, name_record):
        self.index = index
        self.name_record = name_record
        self.has_next_sibling = False # May be set later
        self.value = None # Should be set later

    def get_bytes(self, tree_writer):
        node_type = self.value.get_type()
        flags = 0

        if self.has_next_sibling:
            flags = flags | self.FLAG_HAS_NEXT_SIBLING

        name_int = self.name_record.offset

        value_bytes = self.value.get_value_bytes(tree_writer)

        return struct.pack(tree_writer.byte_order_format + "HHI", node_type, flags, name_int) + value_bytes

class _PropertyTreeImpl:


    __slots__ = [
        'nodes', # List of nodes
        'string_records', # List of string records
        'strings_size', # Number of bytes used for strings.
        'binary_records', # List of binary records
        'binary_data_size', # Number of bytes used for binary data
        'binary_data_align', # Total alignment of binary data.

        'string_records_dict', # Dictionary of existed string records, for reuse them.

        'tree_writer',
    ]

    def __init__(self):
        self.nodes = []
        self.string_records = []
        self.strings_size = 0
        self.binary_records = []
        self.binary_data_size = 0
        self.binary_data_align = 1

        self.string_records_dict = {}

    def load(self, obj):
        """
        Load property tree from object.
        """
        node = self.alloc_node("root")
        self.load_node(node, obj)

    def write(self, filename, tree_writer):
        """
        Write property tree into the file in binary format.
        """
        nodes_offset = 4 * 8

        strings_offset = nodes_offset + 16 * len(self.nodes)

        binary_offset = align_val(strings_offset + self.strings_size, self.binary_data_align)

        offset = 0
        with open(filename, "wb") as fd:
            # Write header
            header_bytes = struct.pack(tree_writer.byte_order_format + "LLLLLLLL",
                0x50725472,
                nodes_offset, len(self.nodes),
                strings_offset, self.strings_size,
                binary_offset, self.binary_data_size,
                0
            )

            fd.write(header_bytes)
            offset += len(header_bytes)

            # Write nodes
            assert(offset == nodes_offset)
            offset += self.write_nodes(fd, tree_writer)

            # Write strings
            assert(offset == strings_offset)
            offset += self.write_strings(fd)

            # Write binary data
            offset += self.write_pad(offset, binary_offset, fd)
            offset += self.write_binaries(fd)



    def write_pad(self, offset, required_offset, fd):
        if offset != required_offset:
            n_bytes = required_offset - offset
            assert(n_bytes)

            fd.write("\n" * n_bytes)
            return n_bytes

        return 0

    def write_nodes(self, fd, tree_writer):
        for node in self.nodes:
            fd.write(node.get_bytes(tree_writer))

        return len(self.nodes) * 16

    def write_strings(self, fd):
        offset = 0
        for string_record in self.string_records:
            fd.write(string_record.value + "\0")
            offset += len(string_record.value) + 1

        return offset

    def write_binaries(self, fd):
        offset = 0

        for binary_record in self.binary_records:
            self.write_pad(offset, binary_record.offset, fd)

            if binary_record.value is not None:
                fd.write(binary_record.value)
            else:
                with open(binary_record.filename, 'rb') as binary_file:
                    shutil.copyfileobj(binary_file, fd)

            offset += binary_record.size

        return offset

    def load_node(self, node, obj):
        """
        Having the node allocated, fill it correspondigly to the object.
        """
        if isinstance(obj, str):
            # print "Interpret object '%s' as string" % str(obj)
            string_record = self.alloc_string(obj)
            node.value = _PropertyTreeNodeValueString(string_record)
        elif isinstance(obj, PropertyTreeBinaryNode):
            # print "Interpret object '%s' as binary node" % str(obj)
            binary_node = self.alloc_binary(obj)
            node.value = binary_node
        elif isinstance(obj, collections.Mapping):
            # print "Interpret object '%s' as mapping" % str(obj)
            children_nodes = []
            children_objs = []
            for name, child_obj in obj.items():
                child_node = self.alloc_node(name)
                children_nodes.append(child_node)
                children_objs.append(child_obj)

            self.process_children(node, children_nodes, children_objs)

        elif isinstance(obj, collections.Sequence):
            # print "Interpret object '%s' as sequence" % str(obj)
            children_nodes = []
            children_objs = []
            for index, child_obj in enumerate(obj):
                child_node = self.alloc_node(str(index))
                children_nodes.append(child_node)
                children_objs.append(child_obj)

            self.process_children(node, children_nodes, children_objs)
        else:
            raise RuntimeError("Unknown node type: %s" % str(obj))

    def process_children(self, node, children_nodes, children_objs):
        n_children = len(children_nodes)

        if n_children != 0:
            first_child = children_nodes[0]
        else:
            first_child = None

        node.value = _PropertyTreeNodeValueTree(n_children, first_child)

        if n_children == 0:
            return

        node.first_child_record = children_nodes[0]
        for i in range(0, n_children - 1):
            children_nodes[i].has_next_sibling = True

        for child_node, child_obj in zip(children_nodes, children_objs):
            self.load_node(child_node, child_obj)

    def alloc_node(self, name):
        name_record = self.alloc_string(name)
        node = _PropertyTreeNodeRecord(len(self.nodes), name_record)
        self.nodes.append(node)

        return node

    def alloc_string(self, value):
        string_record = self.string_records_dict.get(value)
        if string_record is None:
            string_record = _PropertyTreeStringRecord(value, self.strings_size)
            self.strings_size += len(value) + 1 # Do not forget terminated null symbol
            self.string_records.append(string_record)

        return string_record

    def alloc_binary(self, binary_node):
        if self.binary_data_align < binary_node.align:
            self.binary_data_align = binary_node.align

        self.binary_data_size = align_val(self.binary_data_size, binary_node.align)

        binary_record = _PropertyTreeBinaryRecord(binary_node, self.binary_data_size)

        self.binary_records.append(binary_record)
        self.binary_data_size += binary_record.size

        return _PropertyTreeNodeValueBinary(binary_record)

def property_tree_write(obj, filename, is_big_endian):
    """
    Write property tree, corresponded to given object, into the binary file.
    """
    impl = _PropertyTreeImpl()

    impl.load(obj)

    tree_writer = _PropertyTreeWriter(is_big_endian)

    impl.write(filename, tree_writer)
