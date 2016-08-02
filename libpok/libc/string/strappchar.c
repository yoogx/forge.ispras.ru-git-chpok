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

#include <libc/string.h>

/**
  * strappchar - Append one character to a string
  * @dest: The string to be appended to
  * @src: The character to append to the string
  */
__attribute__ ((weak))
void strappchar(char *dest, const char src)
{
    while (*dest)
        dest++;

    *dest++ = src;
}
