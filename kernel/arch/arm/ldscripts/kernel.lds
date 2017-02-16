ENTRY(jet_start)


__KERNBASE_PADDR = 0x10000000;
/* __KERNBASE_PADDR = 0x8000; */

__KERNBASE_VADDR = __KERNBASE_PADDR;
/*
__KERNBASE_VADDR = 0xc0000000;
*/

SECTIONS
{
    . = __KERNBASE_VADDR;
    __text_start = .;
    .text : AT(__KERNBASE_PADDR)
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    . = ALIGN(4);
    __text_end = .;

    __rodata_start = .;
    .rodata :
    {
        *(.rodata)

        /* space for .archive2 section in sizes.o file */
        __archive2_begin = .;
        *(.archive2)
        __archive2_end = .;
    }
    . = ALIGN(4);
    __rodata_end = .;

    __vector_table_start = .;
    .vector_table :
    {
        *(.vector_table)
    }
    . = ALIGN(4);
    __vector_table_end = .;


    __data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(4);
    __data_end = .;

    __bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)

        /* Allocate aligned stack */
        . = ALIGN(16) + 0x1000;
        jet_entry_stack = .;
    }
    . = ALIGN(4);
    __bss_end = .;
    __end = .;
}
