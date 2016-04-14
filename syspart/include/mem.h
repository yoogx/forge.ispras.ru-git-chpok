#ifndef __POK_SYSNET_MEM_H__
#define __POK_SYSNET_MEM_H__
/* simple(stupid) malloc */
void *smalloc (size_t sz);

void *smalloc_aligned(size_t mem_size, size_t alignment);
#endif

