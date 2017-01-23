ENTRY(jet_start)

SECTIONS
{
    . = 0x8000;
/* . = 0x10000000; for i.mx6*/
    __text_start = .;
    .text :
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

        /* Allocate stack */
        . = ALIGN(16) + 0x1000;
        jet_entry_stack = .;
    }
    . = ALIGN(4);
    __bss_end = .;
    __end = .;
}
