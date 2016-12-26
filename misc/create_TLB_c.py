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

from jinja2 import FileSystemLoader,Template
import json
import os

#jetos_path = os.getcwd()

def readjson(name):
    with open(name) as f:
        a = f.read() 
    return json.loads(a)
def create_TLB(TLB_dict,pid,addr):
    
    t=Template("""
    #include <TLB.h>
    #include "../../../../kernel/arch/ppc/mmu.h"
    
    #define False  0
    #define True   1
    
    struct TLB_entry TLB_entries{{index}}[]={
    {%for TLB_entry in TLB_d:%}
         {
         .virtual={{ TLB_entry['virtual'] }},
         .physical={{ TLB_entry['physical'] }},
         .pgsize_enum={{ TLB_entry['pgsize_enum'] }},
         .permissions={{ TLB_entry['permissions'] }},
         .wimge={{ TLB_entry['wimge'] }},
         .pid={{ TLB_entry['pid'] }},
         .is_resident= {{ TLB_entry['is_resident'] }},
         },
    {% endfor %}
    {
         .virtual=0x80ff0000,
         .physical=0x4020000,
         .pgsize_enum=E500MC_PGSIZE_64K ,
         .permissions=MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX | MAS3_SX,
         .wimge=0,
         .pid=1,
         .is_resident= False,
         }
        };
        
        number_TLB_entry{{index}}={{number}};
        """)

    with open(os.path.join(addr, "TLB.c"),'w') as f:
        f.write(t.render(
            index='',
            TLB_d = TLB_dict['0'],
            number = len(TLB_dict['0'])+1
            ))
    
    for TLB in TLB_dict['0']:
        TLB['permissions']='MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX'
    
    with open(os.path.join(addr, "TLB1.c"),'w') as f:
        f.write(t.render(
            index=1,
            TLB_d = TLB_dict['0'],
            number = len(TLB_dict['0'])+1
            ))

def main(addr):
    d = readjson('entry2.json')
    create_TLB(d,0,addr)

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()

