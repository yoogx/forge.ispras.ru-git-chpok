/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

/**
 * \file loader.c
 * \author Julian Pidancet
 * \author Julien Delange
 * \date 2008-2009
 *
 * Contains all needed stuff to load partitions (elf files).
 * This needs the partitioning service (POK_NEEDS_PARTITIONS must
 * be defined) to work.
 */

#include <config.h>

#ifdef POK_NEEDS_PARTITIONS

#include <errno.h>
#include <types.h>
#include <libc.h>
#include <core/cpio.h>
#include <core/error.h>
#include <core/partition.h>
#include <core/debug.h>
#include <elf.h>

#include <core/space.h>

extern size_t pok_elf_sizes[];
extern char __archive2_begin;

/**
 * Load an ELF file.
 *
 *  @param file
 */
void jet_loader_elf_load   (uint8_t elf_id,
                                 jet_space_id space_id,
                                 size_t heap_size,
                                 void (** entry)(void))
{
    size_t elf_offset, elf_size;

    struct jet_space_layout space_layout;
    ja_space_layout_get(space_id, &space_layout);

    elf_offset = 0;
    for (uint8_t i = 0; i < elf_id; i++) {
        elf_offset += pok_elf_sizes[i];
    }

    elf_size = pok_elf_sizes[elf_id];

    if (elf_size > space_layout.size)
    {
         printf("Attempt to load elf %u of size %zx into space of size %zx.\n",
            elf_id, elf_size, space_layout.size);
         pok_raise_error(POK_ERROR_ID_CONFIG_ERROR, FALSE, NULL);
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

    // First user space address after the elf.
    char* elf_end_user = space_layout.user_addr;

    for (int i = 0; i < elf_header->e_phnum; ++i)
    {
        char* user_dest = (char *)elf_phdr[i].p_vaddr;
        size_t filesz = elf_phdr[i].p_filesz;
        size_t memsz = elf_phdr[i].p_memsz;

        assert((user_dest >= space_layout.user_addr)
               && (user_dest + filesz <= space_layout.user_addr + space_layout.size)
               && (user_dest + memsz <= space_layout.user_addr + space_layout.size));

        char* kernel_dest = space_layout.kernel_addr + (user_dest - space_layout.user_addr);

        memcpy (kernel_dest, elf_phdr[i].p_offset + elf_start, filesz);
        memset (kernel_dest + filesz, 0, memsz - filesz);

        // Where given section ends(in user space).
        char* elf_section_end_user = user_dest + memsz;

        if(elf_end_user < elf_section_end_user) {
           elf_end_user = elf_section_end_user;
        }
    }

    struct jet_kernel_shared_data* __kuser kshd = ja_space_shared_data(space_id);

    if(heap_size > 0) {
       // Allocate heap after the elf.
       char* heap_start = (char*)ALIGN_VAL((unsigned long)elf_end_user,
         ja_user_space_maximum_alignment);
       char* heap_end = heap_start + heap_size;
       if(heap_end > (space_layout.user_addr + space_layout.size))
       {
           printf("Insufficient space for partition's heap.");
           pok_raise_error(POK_ERROR_ID_CONFIG_ERROR, FALSE, NULL);
       }

       kshd->heap_start = heap_start;
       kshd->heap_end = heap_end;
   }
   else {
      // No heap requested.
      kshd->heap_start = NULL;
      kshd->heap_end = NULL;
   }
}

#endif /* POK_NEEDS_PARTITIONS */
