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

import yaml

class PropertyTreeLeafNode(yaml.YAMLObject):
    """
    Base class for property tree leaf node.
    """

    @abc.abstractmethod
    def fill_node(self, node, prop_tree_impl):
        """
        Set value of the node in _PropertyTreeImpl object.
        """
        pass

class PropertyTreeNodeInt32(PropertyTreeLeafNode):
    """
    32-bit integer node in property tree.

    Can be given in YAML with '!int32' tag.
    """
    yaml_tag = '!int32'

    def __init__(self, value):
        self.value = value

    def fill_node(self, node, prop_tree_impl):
        node.value = _PropertyTreeNodeValueInteger32(self.value)

    @classmethod
    def from_yaml(cls, loader, node):
        value = int(loader.construct_scalar(node), 0)
        return cls(value)

    @classmethod
    def to_yaml(cls, dumper, data):
        return dumper.represent_scalar(cls.yaml_tag, "%d" % data.value)

class PropertyTreeNodeUInt32(PropertyTreeLeafNode):
    """
    32-bit unsigned integer node in property tree.

    Can be given in YAML with '!uint32' tag.
    """
    yaml_tag = '!uint32'

    def __init__(self, value):
        self.value = value

    def fill_node(self, node, prop_tree_impl):
        node.value = _PropertyTreeNodeValueUnsigned32(self.value)

    @classmethod
    def from_yaml(cls, loader, node):
        value = int(loader.construct_scalar(node), 0)
        return cls(value)

    @classmethod
    def to_yaml(cls, dumper, data):
        return dumper.represent_scalar(cls.yaml_tag, "%u" % data.value)

class PropertyTreeNodeInt64(PropertyTreeLeafNode):
    """
    64-bit integer node in property tree.

    Can be given in YAML with '!int64' tag.
    """
    yaml_tag = '!int64'

    def __init__(self, value):
        self.value = value

    def fill_node(self, node, prop_tree_impl):
        node.value = _PropertyTreeNodeValueInteger64(self.value)

    @classmethod
    def from_yaml(cls, loader, node):
        value = int(loader.construct_scalar(node), 0)
        return cls(value)
    @classmethod
    def to_yaml(cls, dumper, data):
        return dumper.represent_scalar(cls.yaml_tag, "%d" % data.value)

class PropertyTreeNodeUInt64(PropertyTreeLeafNode):
    """
    64-bit unsigned integer node in property tree.

    Can be given in YAML with '!uint64' tag.
    """
    yaml_tag = '!uint64'

    def __init__(self, value):
        self.value = value

    def fill_node(self, node, prop_tree_impl):
        node.value = _PropertyTreeNodeValueUnsigned64(self.value)

    @classmethod
    def from_yaml(cls, loader, node):
        value = int(loader.construct_scalar(node), 0)
        return cls(value)

    @classmethod
    def to_yaml(cls, dumper, data):
        return dumper.represent_scalar(cls.yaml_tag, "%u" % data.value)

class PropertyTreeNodeBinaryFile(PropertyTreeLeafNode):
    """
    Binary data node. Data are contained in the given file.

    Can be given in YAML with '!binary_file' tag.
    """
    yaml_tag = '!binary_file'

    def __init__(self, filename):
        self.filename = filename

    def fill_node(self, node, prop_tree_impl):
        binary_node = prop_tree_impl.alloc_binary(value = None, filename = self.filename)
        node.value = binary_node

    @classmethod
    def from_yaml(cls, loader, node):
        filename = loader.construct_scalar(node)
        return cls(filename)

    @classmethod
    def to_yaml(cls, dumper, data):
        return dumper.represent_scalar(cls.yaml_tag, data.filename)


class PropertyTreeNodeBinaryData(PropertyTreeLeafNode):
    """
    Binary data node. Data are given directly.

    Can be given in YAML with '!binary_data' tag.
    """
    yaml_tag = '!binary_data'

    def __init__(self, value):
        self.value = value

    def fill_node(self, node, prop_tree_impl):
        binary_node = prop_tree_impl.alloc_binary(value = self.value, filename = None)
        node.value = binary_node

    @classmethod
    def from_yaml(cls, loader, node):
        value = loader.construct_scalar(node)
        return cls(value)

    @classmethod
    def to_yaml(cls, dumper, data):
        return dumper.represent_scalar(cls.yaml_tag, data.value)


def property_tree_write(obj, filename, binary_data_align):
    """
    Write property tree, corresponded to given object, into the binary file.
    """
    impl = _PropertyTreeImpl(binary_data_align)

    impl.load(obj)

    impl.write(filename)


def property_tree_serialize(obj, filename):
    """
    Serialize property tree into the file.
    """
    with open(filename, "w") as yaml_file:
        yaml.dump({'root': obj}, yaml_file)

def property_tree_deserialize(filename):
    """
    Deserialize property tree from the file.

    Return property tree extracted.
    """
    with open(filename, "r") as yaml_file:
        yaml_content = yaml.load(yaml_file)

    return yaml_content['root']

############################# Implementation ###########################
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
    def get_value_bytes(self):
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

    struct_II = struct.Struct("!II")

    def __init__(self, string_record):
        self.string_record = string_record

    def get_type(self):
        return self.JET_PT_STRING

    def get_value_bytes(self):
        return self.struct_II.pack(self.string_record.offset, 0)

class _PropertyTreeNodeValueInteger32(_PropertyTreeNodeValueBase):
    __slots__ = [
        'value',
    ]

    struct_iI = struct.Struct("!iI")

    def __init__(self, value):
        self.value = value

    def get_type(self):
        return self.JET_PT_INTEGER32

    def get_value_bytes(self):
        return self.struct_iI.pack(self.value, 0)

class _PropertyTreeNodeValueUnsigned32(_PropertyTreeNodeValueBase):
    __slots__ = [
        'value',
    ]

    struct_II = struct.Struct("!II")

    def __init__(self, value):
        self.value = value

    def get_type(self):
        return self.JET_PT_UNSIGNED32

    def get_value_bytes(self):
        return self.struct_II.pack(self.value, 0)

class _PropertyTreeNodeValueInteger64(_PropertyTreeNodeValueBase):
    __slots__ = [
        'value',
    ]

    struct_q = struct.Struct("!q")

    def __init__(self, value):
        self.value = value

    def get_type(self):
        return self.JET_PT_INTEGER64

    def get_value_bytes(self):
        return self.struct_q.pack(self.value)

class _PropertyTreeNodeValueUnsigned64(_PropertyTreeNodeValueBase):
    __slots__ = [
        'value',
    ]

    struct_Q = struct.Struct("!Q")

    def __init__(self, value):
        self.value = value

    def get_type(self):
        return self.JET_PT_UNSIGNED64

    def get_value_bytes(self):
        return self.struct_Q.pack(self.value)


class _PropertyTreeBinaryRecord:
    __slots__ = [
        'value', # Immediate value, if given.
        'filename', # File with value, if given.
        'size', # Size of the value

        'offset', # Offset in the block of binary data.
    ]

    def __init__(self, value, filename, offset):
        self.value = value
        self.filename = filename
        self.offset = offset

        if self.value is not None:
            self.size = len(self.value)
        else:
            self.size = os.path.getsize(self.filename)

class _PropertyTreeNodeValueBinary(_PropertyTreeNodeValueBase):
    __slots__ = [
        'binary_record',
    ]

    struct_II = struct.Struct("!II")

    def __init__(self, binary_record):
        self.binary_record = binary_record

    def get_type(self):
        return self.JET_PT_BINARY

    def get_value_bytes(self):
        return self.struct_II.pack(self.binary_record.offset, self.binary_record.size)

class _PropertyTreeNodeValueTree(_PropertyTreeNodeValueBase):
    __slots__ = [
        'n_children',
    ]

    struct_II = struct.Struct("!II")

    def __init__(self, n_children, first_child):
        self.n_children = n_children
        self.first_child = first_child

    def get_type(self):
        return self.JET_PT_TREE

    def get_value_bytes(self):
        if self.first_child is not None:
            first_child_index = self.first_child.index
        else:
            first_child_index = JET_PT_INVALID_NODE
        return self.struct_II.pack(self.n_children, first_child_index)

def align_val(val, align):
    return ((val + align - 1) // align) * align

class _PropertyTreeNodeRecord:
    FLAG_HAS_NEXT_SIBLING = 1

    struct_HHI = struct.Struct("!HHI")

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

    def get_bytes(self):
        node_type = self.value.get_type()
        flags = 0

        if self.has_next_sibling:
            flags = flags | self.FLAG_HAS_NEXT_SIBLING

        name_int = self.name_record.offset

        value_bytes = self.value.get_value_bytes()

        return self.struct_HHI.pack(node_type, flags, name_int) + value_bytes

class _PropertyTreeImpl:
    __slots__ = [
        'nodes', # List of nodes
        'string_records', # List of string records
        'strings_size', # Number of bytes used for strings.
        'binary_records', # List of binary records
        'binary_data_size', # Number of bytes used for binary data
        'binary_data_align', # Total alignment of binary data.

        'string_records_dict', # Dictionary of existed string records, for reuse them.
    ]

    def __init__(self, binary_data_align):
        self.nodes = []
        self.string_records = []
        self.strings_size = 0
        self.binary_records = []
        self.binary_data_size = 0
        self.binary_data_align = binary_data_align

        self.string_records_dict = {}

    def load(self, obj):
        """
        Load property tree from object.
        """
        node = self.alloc_node("root")
        self.load_node(node, obj)

    def write(self, filename):
        """
        Write property tree into the file in binary format.
        """
        nodes_offset = 4 * 8

        strings_offset = nodes_offset + 16 * len(self.nodes)

        binary_offset = align_val(strings_offset + self.strings_size, self.binary_data_align)

        offset = 0
        with open(filename, "wb") as fd:
            # Write header
            header_bytes = struct.pack("!LLLLLLLL",
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
            offset += self.write_nodes(fd)

            # Write strings
            assert(offset == strings_offset)
            offset += self.write_strings(fd)

            if self.binary_data_size > 0:
                # Write binary data
                offset += self.write_pad(offset, binary_offset, fd)
                offset += self.write_binaries(fd)

    def write_pad(self, offset, required_offset, fd):
        if offset != required_offset:
            n_bytes = required_offset - offset
            assert(n_bytes)

            fd.write("\0" * n_bytes)

            return n_bytes

        return 0

    def write_nodes(self, fd):
        for node in self.nodes:
            fd.write(node.get_bytes())

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
            offset += self.write_pad(offset, binary_record.offset, fd)

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
        elif isinstance(obj, PropertyTreeLeafNode):
            # print "Interpret object '%s' as leaf node" % str(obj)
            obj.fill_node(node, self)
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
            self.string_records_dict[value] = string_record

        return string_record

    def alloc_binary(self, value, filename):
        self.binary_data_size = align_val(self.binary_data_size, self.binary_data_align)

        binary_record = _PropertyTreeBinaryRecord(value, filename, self.binary_data_size)

        self.binary_records.append(binary_record)
        self.binary_data_size += binary_record.size

        return _PropertyTreeNodeValueBinary(binary_record)

if __name__ == '__main__':

    import sys
    import yaml
    if len(sys.argv) <= 2:
        print ("Usage: prop_tree.py <input-yaml-file> <output-bin-file>")
        exit(1)

    input_yaml_file = sys.argv[1]
    output_bin_file = sys.argv[2]

    with open(input_yaml_file, "r") as source_file:
        yaml_content = yaml.load(source_file)

    property_tree_write(yaml_content, output_bin_file, 16)

    print "Property tree has been written into '%s'" % output_bin_file
