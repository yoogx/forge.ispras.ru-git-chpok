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


#ifndef __POK_TYPES_H__
#define __POK_TYPES_H__

#include <uapi/types.h>

/* 
 * This function should be used for check (untrusted) time whether
 * it is infinity(special) or not.
 */
static inline pok_bool_t pok_time_is_infinity(pok_time_t t)
{
    return (t<0);
}

/* 
 * Compare names.
 * 
 * Name is a array of bytes, which is either of MAX_NAME_LENGTH size
 * without null-bytes, or have null-byte in the first MAX_NAME_LENGTH
 * bytes.
 */
int   strncasecmp(const char *s1, const char *s2, size_t size);
static inline int pok_compare_names(const char* name1, const char* name2)
{
    return strncasecmp(name1, name2, MAX_NAME_LENGTH);
}

#endif
