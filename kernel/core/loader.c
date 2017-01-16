/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
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

#include <types.h>
#include <libc.h>
#include <core/error.h>
#include <core/partition.h>
#include <elf.h>

#include <core/loader.h>

#include <core/memblocks.h>
#include <core/uaccess.h>

extern size_t pok_elf_sizes[];
extern char __archive2_begin;

void jet_loader_elf_load   (uint8_t elf_id,
                                 pok_partition_t* part,
                                 void (** entry)(void))
{
    // Determine offset of required elf in the file.
    size_t elf_offset = 0;

    for (uint8_t i = 0; i < elf_id; i++) {
        elf_offset += pok_elf_sizes[i];
    }

    const char* elf_start = &__archive2_begin + elf_offset;

    Elf32_Ehdr*  elf_header;
    Elf32_Phdr*  elf_phdr;

    elf_header = (Elf32_Ehdr*)elf_start;

    if (elf_header->e_ident[0] != 0x7f ||
         elf_header->e_ident[1] != 'E' ||
         elf_header->e_ident[2] != 'L' ||
         elf_header->e_ident[3] != 'F')
    {
        printf("Partition's ELF has incorrect format");
        pok_raise_error(POK_ERROR_ID_PARTLOAD_ERROR, FALSE, NULL);
    }

    *entry = (void (*)(void)) elf_header->e_entry;

    elf_phdr = (Elf32_Phdr*)(elf_start + elf_header->e_phoff);

    // Iterate over ELF segments.
    for (int i = 0; i < elf_header->e_phnum; ++i)
    {
        char* __user vstart = (char * __user)elf_phdr[i].p_vaddr;
        size_t memsz = elf_phdr[i].p_memsz;

        char* __kuser kstart = jet_user_to_kernel_fill_local(vstart, memsz);

        if(kstart == NULL) {
            printf("ELF segment %d of partition '%s' isn't contained in any memory block.\n",
                i, part->name);
            pok_raise_error(POK_ERROR_ID_PARTLOAD_ERROR, FALSE, NULL);
        }

        size_t filesz = elf_phdr[i].p_filesz;

        if(filesz) {
            memcpy(kstart, (const char*)elf_start + elf_phdr[i].p_offset,
                filesz);
        }
        if(memsz > filesz) {
            memset(kstart + filesz, 0, memsz - filesz);
        }
    }
}
