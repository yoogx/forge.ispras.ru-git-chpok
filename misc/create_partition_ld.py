from jinja2 import FileSystemLoader,Template
import os
import subprocess

FLAG_ALLOC = 1<<1
FLAG_WRITE = 1
FLAG_EXECUTE = 1<<2
jetos_path = os.environ['POK_PATH']

def rouding_size_virtual_address(size_pid_for_flag):
    size1=0
    size2=0
    for TLB_entry in size_pid_for_flag:
        #print(TLB_entry)
        if TLB_entry[3] == FLAG_EXECUTE|FLAG_ALLOC:
            size1 = max(size1,TLB_entry[0])      
        if TLB_entry[3] == FLAG_WRITE|FLAG_ALLOC:
            size2 = max(size2,TLB_entry[0]) 
    return (size1,size2)

def create_partition_ld_skript(data):
    
    template = Template("""
OUTPUT_FORMAT("elf32-powerpc");
OUTPUT_ARCH("powerpc")
ENTRY(__pok_partition_start)

SECTIONS
{
	. = 0x80000000;

	__partition_begin = . ;
       
        .text           :
        {
           *(.text .text.*)
           . = ALIGN(4);
           __text_end = .;
        }
        . = ALIGN({{size1}});
        .rodata :
        {
          *(.rodata .rodata.*)
          *(.rodata1)
          . = ALIGN(4);
          __rodata_end = .;
        }
        .sdata2 :
        {
          __sdata2_start = .;
          PROVIDE (_SDA2_BASE_ = 32768);
          *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
          . = ALIGN(4);
          __sdata2_end = .;
        }
        .sbss2 :
        {
          __sbss2_start = .;
          *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
          . = ALIGN(4);
          __sbss2_end = .;
        }
        .data :
        {
          __data_start = .;
          *(.data .data.* .gnu.linkonce.d.*)
          *(.data1)
          PROVIDE (_SDA_BASE_ = 32768);
          *(.sdata .sdata.* .gnu.linkonce.s.*)
          . = ALIGN(4);
          __data_end = .;
        }
        .sbss :
        {
          __sbss_start = .;
          *(.dynsbss)
          *(.sbss .sbss.* .gnu.linkonce.sb.*)
          *(.scommon)
          __sbss_end = .;
        }
        . = ALIGN(0x{{"%x" % size2}});
        .bss :
        {
          __bss_start = .;
          *(.dynbss)
          *(.bss .bss.* .gnu.linkonce.b.*)
          *(COMMON)
          . = ALIGN(4);
          __bss_end = .;

          _end = .;
        }
       
	__partition_end = . ;
}
""")
    #(size1,size2)=rouding_size_virtual_address(size_pid_for_flag)
    with open(os.path.join(jetos_path, "boards/e500mc/ldscripts/partition.lds"), "w") as f:
        f.write(template.render(data))

def call_ld(name_pid,addr):
    subprocess.call([
       "powerpc-elf-ld", "-o", name_f(name_pid), "-T",
       os.path.join(jetos_path, "boards/e500mc/ldscripts/partition.lds"), 
       "-Map", 
       os.path.join(addr,name_pid,"build/e500mc/part.elf.map"), 
       os.path.join(addr,name_pid,"build/e500mc/deployment.o"), 
       os.path.join(addr,name_pid,"build/e500mc/main.o"), 
       os.path.join(jetos_path, "build/e500mc/libpok/libpok.a"), 
       "/opt/powerpc-elf/lib/gcc/powerpc-elf/4.9.1/libgcc.a"])
        
        
def main(addr):
    data={'size1': 268435456,'size2': 268435456}
    create_partition_ld_skript(data)
    pid = 1
    #while os.path.exists(name_f(pid,addr)):
    for name_pid in partitions:
        call_ld(name_pid,addr)
        pid += 1


def name_f(name_pid,addr):
    return os.path.join(addr,name_pid,"/build/e500mc/part.elf")



#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
