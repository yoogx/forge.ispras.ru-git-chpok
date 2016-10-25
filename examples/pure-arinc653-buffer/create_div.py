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

def name_f(pid):
    return "P"+str(pid)+"/build/e500mc/part.elf"

def full_sizes(pid, flag):
    sizes = 0
    f = open(name_f(pid), 'rb')
    elffile = ELFFile(f)
    for section in elffile.iter_sections():
        if section['sh_flags'] == flag:
            sizes = sizes + section['sh_size']
    return sizes

def divide_by_TLB_entry(pid,size_pid_for_flag):
    for flag in ALL_FLAG:
        size_area = full_sizes(pid, flag)
        size_pid_for_flag.append([
                size_area,
                pid,
                flag])
    return size_pid_for_flag

def find_va(pid,flag):
    f = open(name_f(pid), 'rb')
    elffile = ELFFile(f)
    for section in elffile.iter_sections():
        if section['sh_flags'] == flag:
            return section['sh_addr']


def divided_by_flag_and_pid():
    size_pid_for_flag=[]
    pid=1
    while os.path.exists(name_f(pid)):
        size_pid_for_flag = divide_by_TLB_entry(pid, 
                                                size_pid_for_flag)
        pid+=1
    return size_pid_for_flag
def create_xml_entry_section(size_pid_for_flag):
    entries={}
    for entry in size_pid_for_flag:
        entries[entry[1]] = []
    
    for entry in size_pid_for_flag:
        d = {"virtual_addres" : hex(find_va(entry[1],entry[2])), 
             "flag" : flag[entry[2]], 
             "size" : entry[0]}
        entries[entry[1]].append(d)
    f = open('entry1.json','w')
    f.write(json.dumps(entries , indent = 4))
    f.close()

def main():
    size_pid_for_flag = divided_by_flag_and_pid()
    create_xml_entry_section(size_pid_for_flag)

        
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
