#include <types.h>
#include <stdio.h>

extern char dynamic_memory[];
extern unsigned dynamic_memory_size;
#define ALIGN_UP(addr,size) (((addr)+((size)-1))&(~((size)-1)))


static char * end = NULL;

void *smalloc_aligned(size_t sz, size_t alignment)
{
    char *res;
    if (!end)
        end = dynamic_memory;

    res = (char *) ALIGN_UP((uintptr_t)end, alignment);
    end = res + sz;
    if ((uintptr_t)end > (uintptr_t)dynamic_memory + dynamic_memory_size) {
        printf("Needs more memory!\n");
        return NULL;
    }
    return res;
}

void *smalloc(size_t sz)
{
    //in PPC every memory access must be 4-byte aligned
    return smalloc_aligned(sz, 4);
}

