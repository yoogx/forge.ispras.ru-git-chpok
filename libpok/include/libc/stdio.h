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

#ifndef __POK_LIBC_STDIO_H__
#define __POK_LIBC_STDIO_H__

#include <stdarg.h>

int printf(const char *format, ...)__attribute__ ((format(printf, 1, 2)));
void hexdump (const void *addr, int len);
void snprintf(char *dst, unsigned size, const char *format, ...)__attribute__ ((format(printf, 3, 4)));;


#endif /* __POK_LIBC_STDIO_H_ */
