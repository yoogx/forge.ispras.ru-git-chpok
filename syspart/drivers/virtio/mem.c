#include <types.h>
#include <stdio.h>
#include <memory.h>

#define MEMSIZE 0xa000 //should be enough for virtio

char start[MEMSIZE];
//static void * start = (void *)0x4000000;
//static void * start = (void *)0x80010000;
static char * end = NULL;

void * driver_mem_alloc (size_t sz)
{
    char *res;

    if (!end)
        end = start;

    res = (char *)(((unsigned) end + 4095) & ~4095);
    end = res + sz;
    if ((unsigned)end > (unsigned)start + MEMSIZE) {
        printf("Needs more memory!\n");
        return NULL;
    }
    return res;
}


void * driver_mem_alloc_phys_aligned(size_t sz)
{
    char *res;

    if (!end)
        end = start;

    res = (char *)(((unsigned) end + 4095) & ~4095);
    uintptr_t phys = pok_virt_to_phys(res);
    uintptr_t phys_aligned = ((uintptr_t) phys + 4095) & ~4095;
    uintptr_t diff = phys_aligned - phys;

    res += diff;
    end = res + sz;
    if ((unsigned)end > (unsigned)start + MEMSIZE) {
        printf("Needs more memory!\n");
        return NULL;
    }
    //printf("allocating %p\n", res);
    return res;
}

void * driver_mem_alloc_aligned(size_t mem_size, size_t alignment) 
{
    if (alignment == 4096)
        return driver_mem_alloc_phys_aligned(mem_size);
    else
        printf("Unsuported alignment");
        return NULL;
}
