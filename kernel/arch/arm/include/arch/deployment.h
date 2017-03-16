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

#ifndef __JET_ARM_DEPLOYMENT_H__
#define __JET_ARM_DEPLOYMENT_H__

#include <stdint.h>
#include <stddef.h>

enum page_size {
    PAGE_SIZE_16M,
    PAGE_SIZE_1M,
    PAGE_SIZE_64K,
    PAGE_SIZE_4K
};

/*
 * Page description
 */
struct page {
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t flags; //access rights and cache policy
    enum page_size size;
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
extern struct partition_pages ja_partitions_pages[];
extern size_t ja_partitions_pages_nb;

#endif /* __JET_ARM_DEPLOYMENT_H__ */
