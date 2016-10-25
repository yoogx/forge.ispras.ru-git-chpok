import json
from collections import OrderedDict

from target_descripter import *

f = open(name)
a = f.read() 
d = json.loads(a)


def readjson(name):
    f = open(name)
    a = f.read() 
    return json.loads(a)

def writejson(dict,name):
    f = open(name,'w')
    f.write(json.dumps(dict , indent = 4))
    f.close()

def min_size_only_TLB_entry_for_size(size):
    if size<=0:
        return 0
    for size_TLB in TLB_size:
        if size<=size_TLB
            return size_TLB
    return TLB_size[0]

def sort_entryies_wasted_memory(size_pid_for_flag):
    for entry in size_pid_for_flag:
        entry.insert(0,entry[0]-entry[1])
    size_pid_for_flag.sort(reverse=True)
    for entry in size_pid_for_flag:
        del entry[0]

def find_previous_size_tlb(size):
    for i in range(len(TLB_size)):
        if TLB_size[i] = size:
            return TLB_size[i-1]

def find_allowable_separation(size_pid_for_flag):
    copy_size_pid_for_flag = size_pid_for_flag
    sort_entryies_wasted_memory(size_pid_for_flag)
    while not inspection(size_pid_for_flag):
        if size_pid_for_flag[0][0]-size_pid_for_flag[0][1]<2**12:
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
    else:
        return copy_size_pid_for_flag

def size_all_TLB(size_pid_for_flag):
    size=0
    for entry in size_pid_for_flag:
        size+= entry[0]
    return size

def inspection(size_pid_for_flag):   
    if (size_all_TLB(size_pid_for_flag) <= max_memory_TLB1_entries) &\
            (len(size_pid_for_flag) <= max_number_TLB1_entries):
        return True
    else:
        return False

def create_d_TLB_entry(size_pid_for_flag):
    size_pid_for_flag.sort(reverse=True)
    d = {}
    d[0] = []
    physical_addres = START_PHYSICAL_ADDRESS
    for entry in size_pid_for_flag:
        dd={}
        dd['virtual']=entry[4]
        dd['physical']=hex(physical_addres)
        dd['pgsize_enum']=E500MC_PGSIZE[entry[0]]
        dd['permissions']=flags[entry[3]]
        dd['wimge']=0
        dd['pid']=entry[2]
        dd['is_resident'] = False
        d[0].append(dd)
        physical_addres += entry[0]
    return d

def create_size_pid_for_flag_from_dict(dict):
    size_pid_for_flag = []
    for pid in dict.keys():
        for d in dict[pid]:
            size_area = d['size']
            va = d['virtual_addres']
            flag = d['flag']
            size_pid_for_flag.append([
                min_size_only_TLB_entry_for_size(size_area),
                size_area,
                int(pid),
                flag,
                hex(int(va,16))])
    return size_pid_for_flag
    
    
def main():
    name = 'entry1.json'
    s = readjson(name)
    size_pid_for_flag = create_size_pid_for_flag_from_dict(s)
    
    size_pid_for_flag = find_allowable_separation(size_pid_for_flag)
    
    d = create_d_TLB_entry(size_pid_for_flag)
    name = 'entry2.json'
    writejson(d,name)
        
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
