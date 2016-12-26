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

import json

def readjson(name):
    with open(name) as f:
        a = f.read() 
    return json.loads(a)

def add_dict_in_dict(fromdict, indict):
    for entry in fromdict:
        indict[str(entry.pop('pid'))].append(entry)
    return indict

def main():
    entries = readjson('entry1.json')
    Memconfig = readjson('Memconfig.json')
    entries = add_dict_in_dict(Memconfig, entries)
    with open('entry1_5.json','w') as f:
        f.write(json.dumps(entries , indent = 4))

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
