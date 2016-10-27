#import tool
import json
TLBs = 0
E500MC_PGSIZE = 0
flags = 0
START_PHYSICAL_ADDRESS = 0
max_memory_TLB_entries = 0
all_TLB_sizes = 0

def readtarget():
    f = open('target.json')
    a = f.read() 
    d = json.loads(a)
    global TLBs
    global E500MC_PGSIZE
    global flags
    global START_PHYSICAL_ADDRESS
    global max_memory_TLB_entries
    global all_TLB_sizes
    all_TLB_sizes = d['all_TLB_sizes']
    TLBs = d['TLBs']
    E500MC_PGSIZE = d['dict E500MC_PGSIZE']
    for a in E500MC_PGSIZE.keys():
        E500MC_PGSIZE[E500MC_PGSIZE[a]]=a
    flags = d['flags']
    START_PHYSICAL_ADDRESS = d['def']['START_PHYSICAL_ADDRESS']
    max_memory_TLB_entries = d['def']['max_memory_TLB_entries']
    all_TLB_sizes = d['all_TLB_sizes']
    
def check_sum_size_TLB(entries):
    size = 0
    for entry in entries:
        size += int(E500MC_PGSIZE[entry['pgsize_enum']],16)
    if size+int(START_PHYSICAL_ADDRESS,16) <= max_memory_TLB_entries:
         print('Total memory TLB: Correct')
    else:
         print('Total memory TLB: Incorrect')

def check_size_TLB(entries):
    for entry in entries:
        if entry['name_TLB'] in TLBs.keys():
            if TLBs[entry['name_TLB']]['TLB_sizes'].count(E500MC_PGSIZE[entry['pgsize_enum']]) == 0 :
                print('Size TLB: Correct\n')
                print('Size TLB ='+entry['pgsize_enum']+'\nTarget have not this size for name_TLB = '+entry['name_TLB'])
                break
    else:
        print('Size TLB: Correct')

def find_number_TLB_with_name(entries,name):
    number_tlb = 0
    for entry in entries:
        if entry['name_TLB']==name:
            number_tlb+=1
    return number_tlb

def check_number_TLB(entries):
    for tlb in TLBs.keys():
        if find_number_TLB_with_name(entries,tlb)<TLBs[tlb]['number']:
            print('Number TLB with name_TLB ='+tlb+': Correct')
        else:
            print('Number '+tlb+' = '+str(find_number_TLB_with_name(entries,tlb))+'\nNumber TLB with name_TLB ='+tlb+': Correct')

def check_name_tlb(entries):
    
    for entry in entries:
        if entry['name_TLB'] not in TLBs.keys():
            print('Name TLB = '+entry['name_TLB']+'\nName TLB: Incorrect')
            break
    else:
        print('Name TLB: Correct')

def check_physical_TLB(entries):
    verdict = 'Correct'
    for entry in entries:
        for entry1 in entries:
            if (not(( int(entry['physical'],16)+int(E500MC_PGSIZE[entry['pgsize_enum']],16)<=int(entry1['physical'],16)) |\
             ( int(entry1['physical'],16)+int(E500MC_PGSIZE[entry1['pgsize_enum']],16)<=int(entry['physical'],16))))&\
             (not(entry==entry1)):
                verdict = 'Incorrect'
                print('TLB intersect physical address\n')
                print('TLB with physical address = ' + entry['physical']+' and ' + entry1['physical'])
                break
        else:
            continue
        break
    print('Physical address: '+verdict)
    
def check_virtual_TLB(entries):
    verdict = 'Correct'
    for entry in entries:
        for entry1 in entries:
            if (not(( int(entry['virtual'],16)+int(E500MC_PGSIZE[entry['pgsize_enum']],16)<=int(entry1['virtual'],16)) |\
             ( int(entry1['virtual'],16)+int(E500MC_PGSIZE[entry1['pgsize_enum']],16)<=int(entry['virtual'],16))))&\
             (not(entry['pid']==entry1['pid'])):
                verdict = 'Incorrect' 
                print('TLB intersect virtual adress\n')
                print('TLB with physical address = ' + entry['physical']+' and ' + entry1['physical'])
                break
        else:
            continue
        break
    print('Virtual address: '+verdict)

def readjson(name):
    f = open(name)
    a = f.read() 
    return json.loads(a)

def main():
    readtarget()
    d = readjson('entry2.json')
    for i in d.keys():
        print('--------------'+i+'-------------\n')
        check_name_tlb(d[i])
        check_sum_size_TLB(d[i])
        check_size_TLB(d[i])
        check_number_TLB(d[i])
        check_physical_TLB(d[i])
        check_virtual_TLB(d[i])
    
    
    #create_TLB_entries_from_dict(d)

#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
