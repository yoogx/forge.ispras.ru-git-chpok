OUTPUT_FORMAT("elf32-i386");
OUTPUT_ARCH("i386")
ENTRY(__pok_partition_start)

SECTIONS
{
	. = 0x1000;

	__partition_begin = . ;

	.text :
	{
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data) *(.bss) *(COMMON)
	}

	.ctors :
    {
        __PARTITION_CTOR_START__ = .;
        LONG((__PARTITION_CTOR_END__ - __PARTITION_CTOR_START__) / 4 - 2)
        *(SORT(.ctors))
        LONG(0)
        __PARTITION_CTOR_END__ = .;
    }

	__partition_end = . ;
}
