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
        .ctors :
        {
          __PARTITION_CTOR_START__ = .;
          LONG((__PARTITION_CTOR_END__ - __PARTITION_CTOR_START__) / 4 - 2)
          *(SORT(.ctors))
          LONG(0)
          __PARTITION_CTOR_END__ = .;
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

	test_eom = 0x81000000 ;
}
