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
Requirements for (C-language) types.
"""

import yaml

class TypesRequirements:
    """
    Accumulator for all type requirements.
    """
    def __init__(self):
        # Alignment *sufficient* for any type.
        self.align_max = None # Not set
        # Dictionary <type name> => TypesRequirementsSingle
        self.types = {}

    def load_from_yaml(self, filename):
        """
        Append requirements described in YAML-file to current ones.

        Format of the YAML:

        [align_max: <value>]

        [types:
            - name: <type name>
              align: <type align>
              size: <type size>
              [include: <type header>]
            - ...
        ]
        """
        with open(filename) as fd:
            yaml_dict = yaml.load(fd)

        if 'align_max' in yaml_dict:
            self.align_max = yaml_dict['align_max']

        for type_def in yaml_dict.get('types', []):
            type_name = type_def['name']
            type_align = type_def['align']
            type_size = type_def['size']
            type_include = type_def.get('include', None)

            self.types[type_name] = TypesRequirementsSingle(
                type_align,
                type_size,
                type_include)

class TypesRequirementsSingle:
    """
    Requirements for the single type.
    """
    def __init__(self, align, size, include_file = None):
        # Alignment *sufficient* for variable of this type
        self.align = align
        # Number of bytes *sufficient* for variable of this type
        self.size = size
        # Header, where the type is defined. (None if type is defined always).
        self.include_file = include_file
