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

#ifndef __JET_ASP_CSWITCH_H__
#define __JET_ASP_CSWITCH_H__

/**
 * Initialize `context` on the given stack.
 * 
 * Return stack pointer, which can be used by pok_context_switch() to
 * jump into given entry with given stack.
 */
uint32_t ja_context_init(uint32_t sp, void (*entry)(void));

/**
 * Switch to context, stored in @new_sp.
 * 
 * Pointer to the current context will stored in @old_sp.
 */
void ja_context_switch (uint32_t* old_sp, uint32_t new_sp);

#endif /* __JET_ASP_CSWITCH_H__ */
