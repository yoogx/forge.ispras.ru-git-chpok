OUTPUT_FORMAT("elf32-powerpc");
OUTPUT_ARCH("powerpc")
ENTRY(__pok_partition_start)

SECTIONS
{
	kshd = 0x80000000; /* This should coincide with ja_space_shared_data() in the kernel. */

  . = 0x80001000;

	__partition_begin = . ;
       
        .text           :
        {
           *(.text .text.*)
           __text_end = .;
        }
        .rodata :
        {
          *(.rodata .rodata.*)
          *(.rodata1)
          . = ALIGN(4);
          __rodata_end = .;
        }

        .sdata2 :
        {
          __SDATA2_START__ = .;
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
          __SBSS2_END__ = .;
        }
        .data :
        {
          __data_start = .;
          *(.data .data.* .gnu.linkonce.d.*)
          *(.data1)
          PROVIDE (_SDA_BASE_ = 32768);
          __SDATA_START__ = .;
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
          __SBSS_END__ = .;
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
