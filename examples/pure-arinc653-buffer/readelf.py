#import unittest

import newdd
from create_partition_ld import *

import os.path
import subprocess
import function 
import sys
from jinja2 import FileSystemLoader,Template

from elftools.elf.elffile import ELFFile

#class MyTest(unittest.TestCase):
    
#   def test1(self):
#        for
#        self.assertTrue() 

def full_sizes(pid, flag):
        sizes = 0
        f = open(name_f(pid), 'rb')
        elffile = ELFFile(f)
        for section in elffile.iter_sections():
            if section['sh_flags'] == flag:
                sizes = sizes + section['sh_size']
        return sizes
        
E500MC_PGSIZE ={}

E500MC_PGSIZE[0] = 0
E500MC_PGSIZE[2**12] = 'E500MC_PGSIZE_4K'
E500MC_PGSIZE[2**14] = 'E500MC_PGSIZE_16K'
E500MC_PGSIZE[2**16] = 'E500MC_PGSIZE_64K'
E500MC_PGSIZE[2**18] = 'E500MC_PGSIZE_256K'
E500MC_PGSIZE[2**20] = 'E500MC_PGSIZE_1M'
E500MC_PGSIZE[2**22] = 'E500MC_PGSIZE_4M'
E500MC_PGSIZE[2**24] = 'E500MC_PGSIZE_16M'
E500MC_PGSIZE[2**26] = 'E500MC_PGSIZE_64M'
E500MC_PGSIZE[2**28] = 'E500MC_PGSIZE_256M'
E500MC_PGSIZE[2**30] = 'E500MC_PGSIZE_1G'
E500MC_PGSIZE[2**32] = 'E500MC_PGSIZE_4G'

E500MC_PGSIZE['E500MC_PGSIZE_4K']   = 2**12
E500MC_PGSIZE['E500MC_PGSIZE_16K']  = 2**14
E500MC_PGSIZE['E500MC_PGSIZE_64K']  = 2**16
E500MC_PGSIZE['E500MC_PGSIZE_256K'] = 2**18
E500MC_PGSIZE['E500MC_PGSIZE_1M']   = 2**20
E500MC_PGSIZE['E500MC_PGSIZE_4M']   = 2**22
E500MC_PGSIZE['E500MC_PGSIZE_16M']  = 2**24
E500MC_PGSIZE['E500MC_PGSIZE_64M']  = 2**26
E500MC_PGSIZE['E500MC_PGSIZE_256M'] = 2**28
E500MC_PGSIZE['E500MC_PGSIZE_1G']   = 2**30
E500MC_PGSIZE['E500MC_PGSIZE_4G']   = 2**32


MAS3_SR          =      0x00000001
MAS3_SX          =      0x00000010
MAS3_SW          =      0x00000004
MAS3_UR          =      0x00000002
MAS3_UX          =      0x00000020
MAS3_UW          =      0x00000008


FLAG_ALLOC = 1<<1
FLAG_WRITE = 1
FLAG_EXECUTE = 1<<2
ALL_FLAG = [FLAG_EXECUTE|FLAG_ALLOC, FLAG_WRITE|FLAG_ALLOC, FLAG_ALLOC]


flags={}
flags[FLAG_EXECUTE|FLAG_ALLOC]='MAS3_SX|MAS3_SR|MAS3_UX|MAS3_UR'
flags[FLAG_ALLOC]='MAS3_SR|MAS3_UR'
flags[FLAG_WRITE|FLAG_ALLOC]='MAS3_SW|MAS3_SR|MAS3_UW|MAS3_UR'

max_memory_TLB1_entries=2**30
max_number_TLB1_entries=64

START_PHYSICAL_ADDRESS=0x4000000


def name_f(pid):
    return "P"+str(pid)+"/build/e500mc/part.elf"


def divide_by_TLB_entry(pid,size_pid_for_flag):
    for flag in ALL_FLAG:
        size_area = full_sizes(pid, flag)
        size_pid_for_flag.append([
                min_size_only_TLB_entry_for_size(size_area),
                size_area,
                pid,
                flag])
    sort_entryies_wasted_memory(size_pid_for_flag)
    return size_pid_for_flag
    
    
def min_size_only_TLB_entry_for_size(size):
    if size<=0:
        return 0
    a=32
    while (2**(a-2)-size>0) & (a>0):
        a-=2
    if a<12:
        return 2**12
    return 2**a
    
    
def divided_by_flag_and_pid(nesessary_pid=0):
    size_pid_for_flag=[]
    pid=1
    if (nesessary_pid==0):
        while os.path.exists(name_f(pid)):
            call_ld(pid)
            size_pid_for_flag = divide_by_TLB_entry(pid, 
                                                    size_pid_for_flag)
            pid+=1
    else:
        call_ld(nesessary_pid)
        size_pid_for_flag = divide_by_TLB_entry(nesessary_pid,
                                                size_pid_for_flag)
        
    return size_pid_for_flag
    
def inspection(size_pid_for_flag):   
    if (size_all_TLB(size_pid_for_flag) <= max_memory_TLB1_entries) &\
            (len(size_pid_for_flag) <= max_number_TLB1_entries):
        return True
    else:
        return False

def number_pid():
    pid=1
    while os.path.exists(name_f(pid)):
        pid+=1
    return pid-1

def main():
    TLB_dict=create_TLB_dict()
    
    newdd.create_TLB(TLB_dict,0)


        


def create_TLB_dict():
    TLB_dict={}
    size_pid_for_flag = TLB_entries_for_pid()
    if (size_pid_for_flag != 0)&inspection(size_pid_for_flag):
        TLB_dict[0]=size_pid_for_flag
    else:
        pid=1
        while os.path.exists(name_f(pid)):
            size_pid_for_flag = TLB_entries_for_pid(pid)
            if inspection(size_pid_for_flag):
                TLB_dict[pid] = size_pid_for_flag
            else:
                print("-----------ERROR- TOO BIG SIZE PARTITION NUMBER"+pid+"-----------")
                sys.exit()
            pid+=1
    return TLB_dict

def find_allowable_separation(size_pid_for_flag):
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
        
        for i in range(size_area//(size_TLB_entry//4)):
            dif_size_pid_for_flag.append([
                       (size_TLB_entry//4),
                       (size_TLB_entry//4),
                       pid,
                       flag])
        rest_size_area = size_area%(size_TLB_entry//4)
        dif_size_pid_for_flag.append([
                    min_size_only_TLB_entry_for_size(size_area),
                    rest_size_area,
                    pid,
                    flag])
        size_pid_for_flag.extend(dif_size_pid_for_flag)
        sort_entryies_wasted_memory(size_pid_for_flag)
    return size_pid_for_flag

def TLB_entries_for_pid(nesessary_pid=0):

    size_pid_for_flag=[]

    size_pid_for_flag = divided_by_flag_and_pid(nesessary_pid)
    
    size_pid_for_flag = find_allowable_separation(size_pid_for_flag)
    
    #print size_pid_for_flag
    if not inspection(size_pid_for_flag):
        return size_pid_for_flag
    #create_partition_ld_skript(size_pid_for_flag)
        
    TLB_entries1=[]
    
    for entry in size_pid_for_flag:
        size_area = entry[0]
        flag = entry[3]
        pid = entry[2]
        virtual_addr = 0
        TLB_entries1.append([pid,
                             flag,
                             size_area,
                             virtual_addr])
    
    TLB_entries1.sort(reverse=True)

    for i in range(len(TLB_entries1)):
        size=0
        if (i>0) & (TLB_entries1[i][1]==TLB_entries1[i-1][1])&(TLB_entries1[i][0]==TLB_entries1[i-1][0]):
            size=size+TLB_entries1[i-1][2]
        else:
            size=0
        call_ld(TLB_entries1[i][0])
        TLB_entries1[i][3]=hex(find_virtual_addr(TLB_entries1[i][0],
                               TLB_entries1[i][1],size))
    TLB_entries=[]
    
    for TLB_entry in TLB_entries1:
        size_area = TLB_entry[2]
        flag = TLB_entry[1]
        pid = TLB_entry[0]
        virtual_addr = TLB_entry[3]
        TLB_entries.append([size_area,
                           pid,
                           flag,
                           virtual_addr])
    TLB_entries.sort(reverse=True)
    physical=START_PHYSICAL_ADDRESS
    TLB=[]
    for TLB_entry in TLB_entries:
        #print (TLB_entry[2],flags(TLB_entry[2]))
        TLB.append([E500MC_PGSIZE[TLB_entry[0]],
                    TLB_entry[3],
                    hex(physical),
                    flags[TLB_entry[2]],#'MAS3_SW | MAS3_SR | MAS3_UW | MAS3_UR | MAS3_UX',
                    0,
                    TLB_entry[1],
                    False])     #TLB_entry[2]   ---- flag
        physical+=TLB_entry[0]
    
    return TLB

def sort_entryies_wasted_memory(size_pid_for_flag):
    for entry in size_pid_for_flag:
        entry.insert(0,entry[0]-entry[1])
    size_pid_for_flag.sort(reverse=True)
    for entry in size_pid_for_flag:
        del entry[0]

def find_virtual_addr(pid, flag,size):
    #n=RO[flag]
    f = open(name_f(pid), 'rb')
    elffile = ELFFile(f)
    for section in elffile.iter_sections():
        if section['sh_flags'] == flag:
            #return (math.ceil(section['sh_addr']/float(n)))*n+size
            return section['sh_addr']+size
    print ("------------ERROR flag has a unknown value------------/n")
    return -1



def size_all_TLB(size_pid_for_flag):
    size=0
    for a in size_pid_for_flag:
        size+=min(E500MC_PGSIZE[a[0]], a[0])
    return size
    
def call_ld(pid):
    subprocess.call([
       "powerpc-elf-ld", "-o", name_f(pid), "-T",
       "/home/xubuntu/chpok/boards/e500mc/ldscripts/partition1.lds", 
       "-Map", 
       "/home/xubuntu/chpok/examples/"\
        "pure-arinc653-buffer-2-partitions"\
       "/P"+str(pid)+"/build/e500mc/part.elf.map", 
       "P"+str(pid)+"/build/e500mc/deployment.o", 
       "P"+str(pid)+"/build/e500mc/main.o", 
       "/home/xubuntu/chpok/build/e500mc/libpok/libpok.a", 
       "/opt/powerpc-elf/lib/gcc/powerpc-elf/4.9.1/libgcc.a"])
        
        
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
