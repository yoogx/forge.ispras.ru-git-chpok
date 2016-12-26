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
import math
from collections import OrderedDict

TLBs = 0
E500MC_PGSIZE = 0
flags = 0
START_PHYSICAL_ADDRESS = 0
max_memory_TLB_entries = 0
all_TLB_sizes = 0

def readtarget():
    with open('target.json') as f:
        a = f.read() 
    d = json.loads(a)
    global TLBs
    TLBs = d['TLBs']
    global E500MC_PGSIZE
    E500MC_PGSIZE = d['dict E500MC_PGSIZE']
    global flags
    flags = d['flags']
    global START_PHYSICAL_ADDRESS
    START_PHYSICAL_ADDRESS = d['def']['START_PHYSICAL_ADDRESS']
    global max_memory_TLB_entries
    max_memory_TLB_entries = d['def']['max_memory_TLB_entries']
    global all_TLB_sizes
    all_TLB_sizes = d['all_TLB_sizes']

def readjson(name):
    with open(name) as f:
        a = f.read() 
    return json.loads(a)

def writejson(dict,name):
    with open(name,'w') as f:
        f.write(json.dumps(dict , indent = 4))

def min_size_only_TLB_entry_for_size(size):
    if size<=0:
        return 0
    for size_TLB in all_TLB_sizes:
        if size<=int(size_TLB,16):
            return int(size_TLB,16)
    return int(all_TLB_sizes[0],16)

def wasted_m(a):
    return a[0]-a[1]

def sort_entryies_wasted_memory(size_pid_for_flag):
    
    size_pid_for_flag.sort(reverse=True,key = wasted_m)

def find_previous_size_tlb(size):
    for i in range(len(all_TLB_sizes)):
        if (int(all_TLB_sizes[i],16) == size) & (i!=0):
            return int(all_TLB_sizes[i-1],16)
    return int(all_TLB_sizes[0],16)
        

def find_allowable_separation(size_pid_for_flag):
    copy_size_pid_for_flag = size_pid_for_flag
    sort_entryies_wasted_memory(size_pid_for_flag)
    
    while not inspection(size_pid_for_flag):
        if size_pid_for_flag[0][0]-size_pid_for_flag[0][1]<int(all_TLB_sizes[0],16):
            print("---------ERROR___TOO_BIG_SIZE_PARTITIONS-----/n")
            break

        entry_with_max_wasted_memory = size_pid_for_flag[0]
        size_pid_for_flag.remove(entry_with_max_wasted_memory)
        dif_size_pid_for_flag=[]
        pid = entry_with_max_wasted_memory[2]
        flag = entry_with_max_wasted_memory[3]
        size_area = entry_with_max_wasted_memory[1]
        size_TLB_entry = entry_with_max_wasted_memory[0]
        va = entry_with_max_wasted_memory[4]
        previous_size_tlb = find_previous_size_tlb(size_TLB_entry)
        for i in range(size_area//previous_size_tlb):
            dif_size_pid_for_flag.append([
                       previous_size_tlb,
                       previous_size_tlb,
                       pid,
                       flag,
                       va])
            va = hex(int(va,16)+previous_size_tlb)
        
        rest_size_area = size_area%previous_size_tlb
        dif_size_pid_for_flag.append([
                    min_size_only_TLB_entry_for_size(size_area),
                    rest_size_area,
                    pid,
                    flag,
                    va])
        size_pid_for_flag.extend(dif_size_pid_for_flag)
        sort_entryies_wasted_memory(size_pid_for_flag)
    if inspection(size_pid_for_flag):
        return size_pid_for_flag
    return copy_size_pid_for_flag

def size_all_TLB(size_pid_for_flag):
    #size = 0
    maxpa = 0
    for i in range(len(size_pid_for_flag)-1):
        maxpa = max(maxpa,size_pid_for_flag[i][0] +int(size_pid_for_flag[i][6],16 ))
        #size+= int(ceil(size_pid_for_flag[i][0]/size_pid_for_flag[i+1][0]))*size_pid_for_flag[i+1][0]
    return maxpa  #size +size_pid_for_flag[-1][0]

def inspection(size_pid_for_flag):   
    if (size_all_TLB(size_pid_for_flag)+int(START_PHYSICAL_ADDRESS,16) <= max_memory_TLB_entries):
        return True
    else:
        return False

def create_d_TLB_entry(size_pid_for_flag):
    #size_pid_for_flag.sort(reverse=True)
    d = {}
    d[0] = []
    #physical_addres = START_PHYSICAL_ADDRESS
    for entry in size_pid_for_flag:
        dd={}
        dd['virtual']=entry[4]
        dd['physical']=entry[6]
        dd['pgsize_enum']=E500MC_PGSIZE[hex(entry[0])]
        dd['permissions']=flags[entry[3]]
        dd['wimge']=0
        dd['pid']=entry[2]
        dd['is_resident'] = False
        dd['name_TLB'] = 'TLB1'
        d[0].append(dd)
        #physical_addres = hex(entry[0]+int(physical_addres,16))
    return d
def ph(a):
    return [a[6],a[5]]

def create_size_pid_for_flag_from_dict(dict):
    size_pid_for_flag = []
    for pid in dict.keys():
        for d in dict[pid]:
            size_area = d['size']
            va = d['virtual_addres']
            flag = d['flag']
            cont_flag = d['flag_continuous']
            pa = d.get('physical_addres','0')
            pf = 1 if pa=='0' else 0
            size_pid_for_flag.append([
                min_size_only_TLB_entry_for_size(size_area),
                size_area,
                int(pid),
                flag,
                hex(int(va,16)),
                cont_flag,
                pa,
                pf])
    size_pid_for_flag.sort(reverse=True)
    physical_addres = START_PHYSICAL_ADDRESS
    for entry in size_pid_for_flag:
        if entry[7]:
            entry[6] = physical_addres
            physical_addres = hex(int(physical_addres,16) + entry[0])
    #print(size_pid_for_flag)
    size_pid_for_flag.sort(key = ph)
    print(size_pid_for_flag)
    count = len(size_pid_for_flag)
    for i in range(count):
        if size_pid_for_flag[i][7]==0:
            print('----'+str(size_pid_for_flag[i][7])+'--------\n')
            if i:
                size_pid_for_flag.insert(i-1,size_pid_for_flag.pop(i))
            for j in range(i,count):
                if size_pid_for_flag[j][7]:
                    size_pid_for_flag[j][6] = hex(int(math.ceil(float(int(size_pid_for_flag[j-1][6],16)+size_pid_for_flag[j-1][0])/size_pid_for_flag[j][0]))*size_pid_for_flag[j][0])
            size_pid_for_flag.sort(key = ph)
    
    return size_pid_for_flag
    
def main():
    readtarget()
    name = 'entry1_5.json'
    s = readjson(name)
    size_pid_for_flag = create_size_pid_for_flag_from_dict(s)
    
    #size_pid_for_flag = find_allowable_separation(size_pid_for_flag)
    
    
    d = create_d_TLB_entry(size_pid_for_flag)
    name = 'entry2.json'
    writejson(d,name)
        
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()

