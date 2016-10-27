import os.path
import json
from elftools.elf.elffile import ELFFile
from jinja2 import FileSystemLoader,Template

FLAG_ALLOC = 1<<1
FLAG_WRITE = 1
FLAG_EXECUTE = 1<<2
ALL_FLAG = [FLAG_EXECUTE|FLAG_ALLOC, FLAG_WRITE|FLAG_ALLOC, FLAG_ALLOC]

flag = {}
flag[6] = 'RX'
flag[3] = 'RW'
flag[2] = 'RO'

def name_f(addr):
    return os.path.join(addr,"part.elf")

def full_sizes(flag,addr):
    sizes = 0
    f = open(name_f(addr), 'rb')
    elffile = ELFFile(f)
    for section in elffile.iter_sections():
        if section['sh_flags'] == flag:
            sizes = sizes + section['sh_size']
    return sizes

def divide_by_TLB_entry(pid,size_pid_for_flag,addr):
    for flag in ALL_FLAG:
        size_area = full_sizes(flag,addr)
        size_pid_for_flag.append([
                size_area,
                pid,
                flag])
    return size_pid_for_flag

def find_va(flag,addr):
    f = open(name_f(addr), 'rb')
    elffile = ELFFile(f)
    for section in elffile.iter_sections():
        if section['sh_flags'] == flag:
            return section['sh_addr']


def divided_by_flag_and_pid(addr):
    size_pid_for_flag=[]
    pid=1
    for i in addr:
    #while os.path.exists(name_f(pid,addr)):
        size_pid_for_flag = divide_by_TLB_entry(pid, 
                                                size_pid_for_flag,i)
        pid+=1
    return size_pid_for_flag
def create_json_entry_section(size_pid_for_flag,addr):
    entries={}
    for entry in size_pid_for_flag:
        entries[entry[1]] = []
    
    for entry in size_pid_for_flag:
        d = {"virtual_addres" : hex(find_va(entry[2],addr[entry[1]-1])), 
             "flag" : flag[entry[2]], 
             "size" : entry[0]}
        entries[entry[1]].append(d)
    f = open('entry1.json','w')
    f.write(json.dumps(entries , indent = 4))
    f.close()

def main(addr):
    size_pid_for_flag = divided_by_flag_and_pid(addr)
    create_json_entry_section(size_pid_for_flag,addr)

        
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
