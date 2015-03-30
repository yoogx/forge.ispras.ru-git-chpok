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


#ifdef POK_NEEDS_PARTITIONS

#include <errno.h>
#include <types.h>
#include <libc.h>
#include <core/cpio.h>
#include <core/error.h>
#include <core/partition.h>
#include <core/debug.h>
#include <elf.h>

/**
 * Load an ELF file.
 *
 *  @param file
 */
pok_ret_t pok_loader_elf_load   (char* file,
                                 ptrdiff_t offset,
                                 uintptr_t* entry)
{
   Elf32_Ehdr*  elf_header;
   Elf32_Phdr*  elf_phdr;
   unsigned int i;
   char*        dest;

   elf_header = (Elf32_Ehdr*)file;

   if (elf_header->e_ident[0] != 0x7f ||
       elf_header->e_ident[1] != 'E' ||
       elf_header->e_ident[2] != 'L' ||
       elf_header->e_ident[3] != 'F')
   {
      return POK_ERRNO_NOTFOUND;
   }

   *entry = (uintptr_t) elf_header->e_entry;

   elf_phdr = (Elf32_Phdr*)(file + elf_header->e_phoff);

   for (i = 0; i < elf_header->e_phnum; ++i)
   {
      dest = (char *)elf_phdr[i].p_vaddr + offset;

      memcpy (dest, elf_phdr[i].p_offset + file, elf_phdr[i].p_filesz);
      memset (dest + elf_phdr[i].p_filesz, 0, elf_phdr[i].p_memsz - elf_phdr[i].p_filesz);
   }

   return POK_ERRNO_OK;
}

#endif /* POK_NEEDS_PARTITIONS */
