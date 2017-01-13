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
Useful things for parse elf information.

Note: this uses SCons environment 'env'.
"""

import subprocess
import re

class ElfSegment:
    """
    Elf segment as shown by 'readelf -l'.

    All fields listed in 'integer_slots' have integer type.
    """
    __slots__ = [
        'Type',
        'Offset',
        'VirtAddr',
        'PhysAddr',
        'FileSiz',
        'MemSiz',
        'Align',
        'Flg',
    ]

    integer_slots = [
        'Offset',
        'VirtAddr',
        'PhysAddr',
        'FileSiz',
        'MemSiz',
        'Align',
    ]

    def memory_block_access(self):
        """
        Transform segment flags to 'access' string used for memory block.
        """
        mb_access = ""
        if 'R' in self.Flg:
            mb_access += 'R'
        if 'W' in self.Flg:
            mb_access += 'W'
        if 'E' in self.Flg:
            mb_access += 'X'

        return mb_access


    def __repr__(self):
        res = []

        for s in self.__class__.__slots__:
            value = getattr(self, s)
            if s in self.__class__.integer_slots:
                value = hex(value)
            res.append("%s: %s" % (s, value))

        return ", ".join(res)

    # Callback 'field_setter' for _parse_output_as_table() call (see below).
    def field_setter(self, column_name, value):
        if column_name in self.__class__.integer_slots:
            value = int(value, 0)

        setattr(self, column_name, value)

    # Callback 'column_filter' for _parse_output_as_table() call (see below).
    @classmethod
    def column_filter(cls, column_name):
        if not column_name in cls.__slots__:
            print "Warning: Unexpected column '%s' in 'readelf -l' output" % column_name
            return False # Ignore unexpected column

        return True


def elf_read_segments(env, filename):
    """
    Read list of segments from the ELF file.
    """
    readelf_process = subprocess.Popen(
        args = (env['PREFIX'] + "readelf", "-lW", filename),
        stdout = subprocess.PIPE,
        stderr = subprocess.STDOUT,
    )

    (readelf_stdout, readelf_stderr_unused) = readelf_process.communicate()

    if readelf_process.returncode != 0:
        print "-- Output starts --" + readelf_stdout
        print "-- Output end --"
        raise RuntimeError("readelf terminates with error")

    return _parse_output_as_table(readelf_stdout,
        matcher_before = r'^\s*Program Headers:',
        matcher_after = r'^\s*$',
        entry_type = ElfSegment,
        column_filter = ElfSegment.column_filter,
        field_setter = ElfSegment.field_setter
    )


def _parse_output_as_table(output, matcher_before, matcher_after,
    entry_type = None, column_filter = None, field_setter = None):
    """
    Parse output of some ELF utility as table.

    Selected lines from output are interpreted as textual representation
    of the table with columns delimited with r'\s+'.

    The first line is assumed to contain column names.

    - 'matcher_before', 'matcher_after' - regex-like strings for match
    lines before the table and after the table correspondigly.

    - 'entry_type' - type which will have every entry.
    Should be default-constructible. Default is dictionary.

    - 'column_filter' - function which accepts column name and returns
    True-like value if column should be processed. May raise exception
    if column is unexpected. By default all columns are accepted.

    - 'field_setter' - function which is called as

        field_setter(entry, column_name, value)

    when need to set value for entry. Default is

        entry[column_name] = value
    """
    if field_setter is None:
        field_setter = _default_field_setter

    if column_filter is None:
        column_filter = lambda column_name: True

    if entry_type is None:
        entry_type = dict

    entries = []

    # State of the output scanning:
    #
    # - 0 - before the table
    # - 1 - table's title
    # - 2 - table's elements
    scan_state = 0

    for l in output.splitlines():
        if scan_state == 0:
            if re.match(matcher_before, l):
                scan_state = 1
        elif scan_state == 1:
            column_names = re.split(r'\s+', l)
            column_indicies = []
            for i, column_name in enumerate(column_names):
                if column_name != "" and column_filter(column_name):
                    column_indicies.append(i)
            scan_state = 2
        else: #scan_state == 2
            if re.match(matcher_after, l):
                break
            entry_values = re.split(r'\s+', l)

            entry = entry_type()

            for i in column_indicies:
                field_setter(entry, column_names[i], entry_values[i])

            entries.append(entry)

    return entries

# Default 'field_setter' callback for _parse_output_as_table().
def _default_field_setter(entry, column_name, value):
    entry[column_name] = value
