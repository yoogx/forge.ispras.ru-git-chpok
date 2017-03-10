ENTRY(jet_start)


/* __KERNBASE_ values should be aligned by 1MB */
__KERNBASE_PADDR = 0x10000000;
/* __KERNBASE_PADDR = 0x8000; */

__KERNBASE_VADDR = 0xc0000000;

SECTIONS
{
    . = __KERNBASE_VADDR;
    .text : AT(__KERNBASE_PADDR)
    {
        KEEP(*(.text.boot))
        *(.text)
    }
    . = ALIGN(4);

    .rodata :
    {
        *(.rodata)

        /* space for .archive2 section in sizes.o file */
        __archive2_begin = .;
        *(.archive2)
        __archive2_end = .;
    }
    . = ALIGN(4);

    __vector_table_start = .;
    .vector_table :
    {
        *(.vector_table)
    }
    . = ALIGN(4);
    __vector_table_end = .;


    .data :
    {
        *(.data)
    }
    . = ALIGN(4);

    .bss :
    {
       *(.bss)
       *(COMMON)

        /* Allocate aligned stack */
        . = ALIGN(16) + 0x1000;
        jet_entry_stack = .;

        jet_heap_start = . ;
        jet_heap_end = . + 0x40000; /* 256 KB for heap  */
    }
    . = ALIGN(4);
}
