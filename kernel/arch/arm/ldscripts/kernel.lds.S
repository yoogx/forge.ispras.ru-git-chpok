#include <arch/memlayout.h>

ENTRY(jet_start)

__KERNBASE_PADDR = KERNBASE_PADDR;
__KERNBASE_VADDR = KERNBASE_VADDR;

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

        . = ALIGN(4096);
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
    }
    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
    }

    jet_heap_start = . ;
    jet_heap_end = . + 0x500000; /* 5 MB heap */
}
