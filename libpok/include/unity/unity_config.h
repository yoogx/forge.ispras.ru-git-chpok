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

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

#include <stdio.h>
#include <string.h>

#include <arinc653/partition.h>

#define UNITY_EXCLUDE_STDINT_H         1
#define UNITY_EXCLUDE_LIMITS_H         1
#define UNITY_EXCLUDE_SETJMP_H         1
#define UNITY_EXCLUDE_MATH_H           1
#define UNITY_EXCLUDE_STDLIB_MALLOC    1

//~ #define UNITY_OMIT_OUTPUT_CHAR_HEADER_DECLARATION 1
#define UNITY_OUTPUT_MAX_LENGTH 10000
char outstr[UNITY_OUTPUT_MAX_LENGTH];
unsigned int pos;
#define UNITY_OUTPUT_CHAR(a) outstr[pos++] = a

#define UNITY_OMIT_OUTPUT_FLUSH_HEADER_DECLARATION 1
#define UNITY_OUTPUT_FLUSH() {}

#define UNITY_OUTPUT_START()                        \
    do {                                            \
        pos = 0;                                    \
        memset(outstr, 0, UNITY_OUTPUT_MAX_LENGTH); \
    } while (0)

#define UNITY_OUTPUT_COMPLETE() printf("\n%s\n", outstr)

#endif /* UNITY_CONFIG_H */
