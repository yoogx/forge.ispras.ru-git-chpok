import json


max_memory_TLB_entries=2**31
max_number_TLB1_entries=64

max_number_TLB0_entries=512

E500MC_PGSIZE ={}

E500MC_PGSIZE[hex(2**12)] = 'E500MC_PGSIZE_4K'
E500MC_PGSIZE[hex(2**14)] = 'E500MC_PGSIZE_16K'
E500MC_PGSIZE[hex(2**16)] = 'E500MC_PGSIZE_64K'
E500MC_PGSIZE[hex(2**18)] = 'E500MC_PGSIZE_256K'
E500MC_PGSIZE[hex(2**20)] = 'E500MC_PGSIZE_1M'
E500MC_PGSIZE[hex(2**22)] = 'E500MC_PGSIZE_4M'
E500MC_PGSIZE[hex(2**24)] = 'E500MC_PGSIZE_16M'
E500MC_PGSIZE[hex(2**26)] = 'E500MC_PGSIZE_64M'
E500MC_PGSIZE[hex(2**28)] = 'E500MC_PGSIZE_256M'
E500MC_PGSIZE[hex(2**30)] = 'E500MC_PGSIZE_1G'
E500MC_PGSIZE[hex(2**32)] = 'E500MC_PGSIZE_4G'

all_TLB_sizes=[hex(4**x*2**12) for x in range(11)]

ALL_FLAG = ['RX', 'RW', 'RO']
TLBs = {}
TLBs['TLB1'] = {'number': max_number_TLB1_entries, 'TLB_sizes': all_TLB_sizes}
TLBs['TLB0'] = {'number': max_number_TLB0_entries, 'TLB_sizes': [hex(2**12)]}




flags={}
flags['RX']='MAS3_SX|MAS3_SR|MAS3_UX|MAS3_UR|MAS3_SW'
flags['RO']='MAS3_SR|MAS3_UR|MAS3_SW'
flags['RW']='MAS3_SW|MAS3_SR|MAS3_UW|MAS3_UR'

all_TLB_sizes=[hex(4**x*2**12) for x in range(11)]

START_PHYSICAL_ADDRESS=0x4000000

def main():
    d={}
    d['def'] = {}
    d['def']['START_PHYSICAL_ADDRESS'] = hex(START_PHYSICAL_ADDRESS)
    d['def']['max_memory_TLB_entries'] = max_memory_TLB_entries

    
    d['flags'] = flags
    d['TLBs'] = TLBs
    d['all_TLB_sizes'] = all_TLB_sizes
    
    d['dict E500MC_PGSIZE'] = E500MC_PGSIZE
    
    f = open('target.json','w')
    f.write(json.dumps(d , indent = 4))
    f.close()
    
#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
