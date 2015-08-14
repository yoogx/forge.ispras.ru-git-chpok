#ifndef __POK_SYSNET_MEM_H__
#define __POK_SYSNET_MEM_H__
void *driver_mem_alloc (size_t sz);

void *driver_mem_alloc_aligned(size_t mem_size, size_t alignment);
#endif

