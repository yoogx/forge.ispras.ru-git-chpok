/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 *
 */

/* Definitions which are used in deployment.c */

#ifndef __JET_X86_DEPLOYMENT_H__
#define __JET_X86_DEPLOYMENT_H__

#include <stdint.h>
#include <stddef.h>
//#include <asp/space.h> //TODO delete?

/*
 * Description of one page
 */
struct page {
    uint32_t vaddr;
    uint32_t paddr_and_flags;
    uint8_t is_big; //1 if page_size is 4MB
};

/*
 * Description of pages for one partition
 */
struct partition_pages {
    struct page *pages;
    size_t len;
};

/*
 * Array of partition_pages.
 * Element i corresponds to space_id i+1.
 */
extern struct partition_pages partitions_pages[];
extern size_t partitions_pages_nb;

#endif /* __JET_PPC_DEPLOYMENT_H__ */
