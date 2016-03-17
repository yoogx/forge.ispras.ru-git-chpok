#include <config.h>

#include <errno.h>
#include <types.h>
#include <libc.h>
#include <core/loader.h>
#include <core/partition_arinc.h>
#include <core/error_arinc.h>

#include "mmu.h"
#include "reg.h"


extern size_t pok_elf_sizes[];

void pok_arch_load_partition(pok_partition_arinc_t* part,
    uint8_t elf_id,
    uint8_t space_id,
    uintptr_t *entry)
{
    extern char __archive2_begin;
    size_t elf_offset, elf_size;

    elf_offset = 0;
    for (uint8_t i = 0; i < elf_id; i++) {
        elf_offset += pok_elf_sizes[i];
    }

    elf_size = pok_elf_sizes[elf_id];
    
    if (elf_size > part->size)
    {
		printf("Declared size for partition %d : %ld\n", part->partition_id, part->size);
        printf("Real size for partition %d     : %d\n", part->partition_id, elf_size);
//TODO: How to emit partition's error?
//#ifdef POK_NEEDS_ERROR_HANDLING
//        pok_error_raise_partition(part_id, POK_ERROR_KIND_PARTITION_CONFIGURATION);
//#else
#ifdef POK_NEEDS_DEBUG
      /* We consider that even if errors are not raised, we must print an error
       * for such error
       * So, when we are using the debug mode, we print a fatal error.
       */
        pok_fatal ("Partition size is not correct\n");
#endif
//#endif
    }    
    /*
     * In order to load partition, we basically pretend that 
     * we're running in its context, which "activates" corresponding
     * TLB mappings.
     *
     * So partition is visible in kernel space, under the same addresses
     * as it runs itself. We don't need to adjust addresses or anything.
     * That's why second argument - offset - is zero;
     *
     * XXX maliciously linked ELF life may overwrite kernel data
     */

    mtspr(SPRN_PID, space_id + 1);
    pok_loader_elf_load((&__archive2_begin) + elf_offset, 0, entry);
    mtspr(SPRN_PID, 0); 
}
