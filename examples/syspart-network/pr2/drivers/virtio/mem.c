#include <types.h>
#include <stdio.h>

#define MEMSIZE 0x10000

//char memory_for_driver[MEMSIZE];
static void * start = (void *)0x4000000;
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
    printf("Virtio: allocated 0x%zx bytes: %p\n", sz, res);
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
