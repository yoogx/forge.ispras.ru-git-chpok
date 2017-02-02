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
 */

#ifndef __LIBJET_ALLOC_H__
#define __LIBJET_ALLOC_H__

#include <types.h>
#include <asp/alloc.h>

/* 
 * Allocate memory region of given size and alignment.
 * 
 * May be used only in init mode.
 */
void* smalloc_aligned(size_t size, size_t alignment);

/* 
 * Allocate memory region of given size and alignment, suitable for any
 * object of size less-or-equal than given one.
 * 
 * May be used only in init mode.
 */
void* smalloc(size_t size);

/* 
 * Allocate memory region for array with given number of elements,
 * which element has given size and alignment, suitable for any
 * object of size less-or-equal than given one.
 * 
 * May be used only in init mode.
 */
void* scalloc(size_t nmemb, size_t size);


// free() is not supported.

#define libjet_mem_get_alignment libja_mem_get_alignment

/**** Simple allocator operated with memory region in memory block. ****/
struct jet_sallocator
{
    /* Pointer to the first unused byte in the region. */
    char* addr_current;
    /* Pointer after the last byte in the region. */
    char* addr_end;
    /* Maximum alignment which can be requested. */
    size_t max_alignment;
};

struct _jet_memory_block_status;

/*
 * Initialize allocator.
 */
void jet_sallocator_init(struct jet_sallocator* sallocator,
    void* addr, size_t size, size_t max_alignment);

/*
 * Initialize allocator from memory block.
 */
void jet_sallocator_init_from_memblock(struct jet_sallocator* sallocator,
    struct _jet_memory_block_status* mb_status);

/*
 * Allocate memory of given size and alignment, suitable for any
 * object of size less-or-equal than given one.
 * 
 * On fail return NULL.
 * 
 * If computed alignment is greater than allocator's parameter
 * 'max_alignment', print a message and abort().
 * 
 * May be used only in init mode.
 */
void* jet_sallocator_alloc(struct jet_sallocator* sallocator, size_t size);

/* 
 * Allocate memory of given size and alignment.
 * 
 * On fail return NULL.
 * 
 * If 'alignment' is greater than allocator's parameter 'max_alignment',
 * print a message and abort().
 *
 * May be used only in init mode.
 */
void* jet_sallocator_alloc_aligned(struct jet_sallocator* sallocator, size_t size,
    size_t alignment);

/* 
 * Allocate memory for array with given number of elements, which
 * element has given size and alignment, suitable for any object of size
 * less-or-equal than given one.
 * 
 * On fail return NULL.
 * 
 * If computed alignment is greater than allocator's parameter
 * 'max_alignment', print a message and abort().
 * 
 * May be used only in init mode.
 */
void* jet_sallocator_alloc_array(struct jet_sallocator* sallocator,
    size_t n_elems, size_t size);


/* Return current state of the allocator. */
void* jet_sallocator_get_state(struct jet_sallocator* sallocator);

/*
 * Set state for the allocator.
 * 
 * 'state' should be obtained via previous jet_sallocator_get_state().
 */
void jet_sallocator_set_state(struct jet_sallocator* sallocator,
    void* state);

#endif /* __LIBJET_ALLOC_H__ */

