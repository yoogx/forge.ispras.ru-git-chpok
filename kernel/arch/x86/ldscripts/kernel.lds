OUTPUT_FORMAT("elf32-i386");
OUTPUT_ARCH("i386")
ENTRY(_core_entry)

__KERNBASE = 0xc0000000; /* Should be equal to KERNBASE in memlayout.h */
__SHIFT = 0x100000;
SECTIONS
{
    . = __KERNBASE + __SHIFT;

    .text : AT(__SHIFT)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)

        __archive_begin = .;
        *(.archive)
        __archive_end = .;
        __archive2_begin = .;
        *(.archive2)
        __archive2_end = .;
    }

    .bss :
    {
        *(.bss)
        *(COMMON)

       /* Allocate the main stack area.  Make sure it starts 16bytes aligned
          to accommodate any possible program expectation.  */
        . = ALIGN(16) + 0x1000;
        jet_entry_stack = .;

        jet_heap_start = . ;
        jet_heap_end = . + 0x40000; /* 256 KB for heap  */
        ASSERT (jet_heap_end < __KERNBASE + 0x800000, "ERROR: KERNEL SIZE > 8MB. If you want more then you should change this check and add pages in kernel_pgdir.c (you should update both entry_pgdir and pgdir_insert_kernel_mapping)");
    }

}
