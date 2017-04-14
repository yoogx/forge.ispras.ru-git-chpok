/*
 * Automaticaly generated file; DO NOT EDIT.
 */
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */
/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */
/* should be at least 1MB aligned */
/* KERNBASE_ values should be aligned by 1MB */
/* #define KERNBASE_PADDR 0x0 */
/* addr of interrupt vector table */
ENTRY(jet_start)
__KERNBASE_PADDR = 0x10000000;
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
