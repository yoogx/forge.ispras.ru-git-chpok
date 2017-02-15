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
Support for serialization/deserialization of python objects in textual form.
Such a way, by reading the file one may easily check object's content.


For make object serializable into YAML file and deserializable from it:

1. Derive object's class from 'SerializableObject'.

2. In the class, define field 'yaml_tag' in form '!<namespace>.<class-name>'.
    Note, that value of yaml_tag should be unique for all serializable objects.

3. In the class, define field 'copy_slots' as list of all object's fields
    intended for deserialization.

4. Implement object's constructor as:

    def __init__(self, **kargs):
        copy_constructor(self, kargs)

    Such a way all fields listed in 'copy_slots' will be initialized from
    named parameters (and name arguments are required for all such fields).

4a. Alternatively, you may define field 'default_slots' with mapping
     of default values for some fields. Then constructor implementation

    def __init__(self, **kargs):
        copy_constructor_with_default(self, kargs)

    would allow to omit some parameters (for which exists mapping in
    'default_slots') on object's initialization.

    Note, that default values should be of immutable types (that is,
    lists and maps will not work as expected).

5. Make sure that object's fields are serializable:

    a) has simple type: int, str, etc.,
    b) object of serializable type,
    c) map or map-like object, which keys and values are serializable,
    d) list or list-like object, which values are serializable.

NOTE: Derivation of serializable classes is not supported.
"""

import yaml

SerializableObject = yaml.YAMLObject

def copy_constructor(self, args_dict):
    """
    Fill all object's fields, listed in class'es field 'copy_slots',
    with values from dictionary given as second argument.

    The dictionary should contain all fields listed in 'copy_slots'.
    """
    for arg in self.__class__.copy_slots:
        setattr(self, arg, args_dict[arg])

def copy_constructor_with_default(self, args_dict):
    """
    Fill all object's fields, listed in class'es field 'copy_slots',
    with values from dictionary given as second argument.

    Some fields may be absent in dictionary. In that case corresponded
    object fields will be initialized by default, in accordance with
    'default_slots' class'es mapping.
    """
    for arg in self.__class__.copy_slots:
        if arg in self.__class__.default_slots:
            setattr(self, arg, args_dict.get(arg, self.__class__.default_slots[arg]))
        else:
            setattr(self, arg, args_dict[arg])

def serialize_object_as_text(serializableObject, filename):
    """
    Serialize object into the file.
    Object should be of serializable type which have 'copy_slots' field.
    """
    m = {k: getattr(serializableObject, k) for k in serializableObject.__class__.copy_slots}
    with open(filename, "w") as target_file:
        yaml.dump(m, target_file)

def deserialize_object_from_text(serializableClass, filename):
    """
    Deserialize object of given class from the file.
    Class should be serializable and have 'copy_slots' field.

    Return object extracted.
    """
    with open(filename, "r") as source_file:
        yaml_content = yaml.load(source_file)

    return serializableClass(**yaml_content)


def serialize_as_text(self, filename, topname):
    """
    Serialize object into the file, using 'topname' as a key for the object.
    (It is required for YAML file to have mapping at top-level).
    """
    with open(filename, "w") as target_file:
        yaml.dump({topname: self}, target_file)

def deserialize_from_text(filename, topname):
    """
    Deserialize object from the file, using 'topname' as a key for the object.
    (It is required for YAML file to have mapping at top-level).

    Return object extracted.

    Value of 'topname' should be the same as in the previous call to
    serialize_as_text().
    """
    with open(filename, "r") as source_file:
        yaml_content = yaml.load(source_file)

    return yaml_content[topname]
