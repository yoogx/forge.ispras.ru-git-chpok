#include <types.h>
#include <core/loader.h>
#include <core/partition.h>

extern size_t pok_elf_sizes[];

void pok_arch_load_partition(pok_partition_id_t part_id, uintptr_t *entry)
{
    extern char __archive2_begin;
    size_t elf_offset, elf_size;
    pok_partition_t *part = &pok_partitions[part_id];

    elf_offset = 0;
    for (pok_partition_id_t i = 0; i < part_id; i++) {
        elf_offset += pok_elf_sizes[i];
    }

    elf_size = pok_elf_sizes[part_id];
    
    if (elf_size > pok_partitions[part_id].size)
    {
#ifdef POK_NEEDS_ERROR_HANDLING
        pok_error_raise_partition(part_id, POK_ERROR_KIND_PARTITION_CONFIGURATION);
#else
#ifdef POK_NEEDS_DEBUG
      /* We consider that even if errors are not raised, we must print an error
       * for such error
       * So, when we are using the debug mode, we print a fatal error.
       */

        printf("Declared size for partition %d : %d\n", part_id, pok_partitions[part_id].size);
        printf("Real size for partition %d     : %d\n", part_id, size);
        pok_fatal ("Partition size is not correct\n");
#endif
#endif
   }    

    /*
     * When we're running kernel code, 
     * we're basically running in physical address space. 
     *
     * Partition is visible, but under different address than
     * it will be running.
     *
     */
   
    pok_loader_elf_load((&__archive2_begin) + elf_offset, part->base_addr - part->base_vaddr, entry);
}
