#include <types.h>
#include <stdio.h>

#define MEMSIZE 0x10000
char memory_for_driver[MEMSIZE];
static char *end = memory_for_driver;

void * driver_mem_alloc (size_t sz)
{
    char *res;

    res = (char *)(((unsigned) end + 4095) & ~4095);
    end = res + sz;
    if ((unsigned)end > (unsigned)memory_for_driver + MEMSIZE) {
        printf("Needs more memory!\n");
        return NULL;
    }
    return res;
}

void * driver_mem_alloc_aligned(size_t mem_size, size_t alignment) 
{
    if (alignment == 4096)
        return driver_mem_alloc(mem_size);
    else
        printf("Unsuported alignment");
        return NULL;
}
