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
import os
import subprocess


POK_PATH = 0
SCONSTRUCT_DIR = 0
PARTITIONS = 0

def create_partition_ld_skript(data):
    
    template = Template("""
OUTPUT_FORMAT("elf32-powerpc");
OUTPUT_ARCH("powerpc")
ENTRY(__pok_partition_start)

SECTIONS
{
	kshd = 0x80000000; /* This should coincide with ja_space_shared_data() in the kernel. */

  . = 0x80010000;

	__partition_begin = . ;
       
        .text           :
        {
           *(.text .text.*)
           __text_end = .;
        }
        . = ALIGN(0x{{"%x" % size2}});
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
        . = ALIGN(0x{{"%x" % size2}});
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
        .bss  :
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
    with open(os.path.join(POK_PATH, "kernel/arch/ppc/ldscripts/partition.lds"), "w") as f:
        f.write(template.render(data))

def call_ld(name_pid):
    subprocess.call([
       "powerpc-elf-ld", "-o", name_f(name_pid), "-T",
       os.path.join(POK_PATH, "kernel/arch/ppc/ldscripts/partition.lds"), 
       "-Map", 
       os.path.join(SCONSTRUCT_DIR,name_pid,"build/e500mc/part.elf.map"), 
       os.path.join(SCONSTRUCT_DIR,name_pid,"build/e500mc/deployment.o"), 
       os.path.join(SCONSTRUCT_DIR,name_pid,"build/e500mc/main.o"), 
       os.path.join(POK_PATH, "build/e500mc/libpok/libpok.a"), 
       "/opt/powerpc-elf/lib/gcc/powerpc-elf/4.9.1/libgcc.a"])
        
def write_const(env):
    global POK_PATH
    global SCONSTRUCT_DIR
    global PARTITIONS 
    POK_PATH = env['POK_PATH']
    SCONSTRUCT_DIR = env['SCONSTRUCT_DIR']
    PARTITIONS = env['PARTITIONS']

def main(env):
    write_const(env)
    data={'size1': 268435456,'size2': 268435456}
    create_partition_ld_skript(data)
    pid = 1
    for name_pid in PARTITIONS:
        call_ld(name_pid)
        pid += 1


def name_f(name_pid):
    return os.path.join(SCONSTRUCT_DIR,name_pid,"build/e500mc/part.elf")



#-------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
