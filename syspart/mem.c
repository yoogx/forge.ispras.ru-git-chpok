#include <types.h>
#include <stdio.h>

#define MEMSIZE 0xa000 //should be enough for virtio

char start[MEMSIZE];

static char * end = NULL;

void *smalloc(size_t sz)
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

void *smalloc_aligned(size_t mem_size, size_t alignment)
{
    if (alignment == 4096) {
        return smalloc(mem_size);
    } else {
        printf("Unsuported alignment");
        return NULL;
    }
}
