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
Parser for file with properties (name = value pairs).
"""
# Currently, it supports syntax of Java properties file but without escaping (e.g., '\=') and Unicode (\uxxxx).

import re

def properties_load_from_file(result_dict, filename):
    """
    Enrich dictionary with properties contained in the given file.
    """
    parser = _PropertiesParser(filename, result_dict)
    parser.parse()

class _PropertiesParser:
    re_spaces = re.compile(r'[ \t]+')
    re_line = re.compile(r'[^\n]*')
    re_key = re.compile(r'[^ \t\n=:]+')
    re_spaces_or_assing = re.compile(r'[ \t]*[=:]?[ \t]*')
    re_value = re.compile(r'[^\n]*')

    def __init__(self, filename, result_dict):
        with open(filename, "r") as fd:
            self.content = fd.read()

        self.pos = 0
        self.result_dict = result_dict

    def parse(self):
        while not self.is_end():
            self.parse_next()

    def parse_next(self):
        self.skip_spaces()
        if self.is_end():
            return

        if self.current() == '\n':
            self.pos = self.pos + 1
            return

        if(self.current() == '#'):
            self.pos = self.pos + 1
            self.skip_line()
            return

        key = self.parse_key()

        if key is None:
            if not self.is_end():
                assert(self.current() == '\n')
                self.pos = self.pos + 1
            return

        self.skip_spaces_or_assing()

        value = self.parse_value()

        self.result_dict[key] = value

        if self.is_end():
            return

        assert(self.current() == '\n')
        self.pos = self.pos + 1


    def current(self):
        return self.content[self.pos]

    def is_end(self):
        return len(self.content) == self.pos

    def skip_spaces(self):
        m = self.__class__.re_spaces.match(self.content, self.pos)
        if m is not None:
            self.pos = m.end()

    def skip_line(self):
        m = self.__class__.re_line.match(self.content, self.pos)
        assert(m is not None)
        self.pos = m.end()

    def parse_key(self):
        m = self.__class__.re_key.match(self.content, self.pos)
        if m is None:
            return None

        key = m.group(0)
        self.pos = m.end()

        return key

    def skip_spaces_or_assing(self):
        m = self.__class__.re_spaces_or_assing.match(self.content, self.pos)
        if m is not None:
            self.pos = m.end()

    def parse_value(self):
        m = self.__class__.re_value.match(self.content, self.pos)
        assert(m is not None)

        value = m.group(0)
        self.pos = m.end()

        return value
