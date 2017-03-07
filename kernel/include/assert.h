/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2017 ISPRAS
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

#ifndef __JET_KERNEL_ASSERT_H__
#define __JET_KERNEL_ASSERT_H__

#include <core/debug.h>

__attribute__((__noreturn__))
static inline void pok_assertion_fail(const char *expression, const char *file, int line)
{
    printf("Assertion failed (%s) in %s:%d\n", expression, file, line);
    pok_fatal("");
}

//never def?
#ifndef NDEBUG
    #define assert(expr) ( \
        (expr) ? \
        ((void) 0) : \
        pok_assertion_fail(#expr, __FILE__, __LINE__) \
    )
#else
    #define assert(expr) do {} while (0)
#endif

#define unreachable() assert(FALSE)

/* this is strange but we need 2 macros to stringify __LINE__ */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define STATIC_ASSERT(cond) _Static_assert(cond, AT)

#endif
