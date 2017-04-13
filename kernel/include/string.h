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

/* 
 * While all C99 "string.h" functions are declared here, only ones
 * declared in "libc.h" are defined actually.
 * 
 * Such situation is temporary.
 */

#ifndef __JET_STRING_H__
#define __JET_STRING_H__

#include <stddef.h>

void *memcpy(void * restrict dest, const void * restrict src, size_t n);

void *memmove(void* dest, const void* src, size_t n);

char *strcpy(char * restrict dest, const char * restrict src);
char *strncpy(char * restrict dest, const char * restrict src, size_t n);

char *strcat(char * restrict dest, const char * restrict src);
char *strncat(char * restrict dest, const char * restrict src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

void *memchr(const void *s, int c, size_t n);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

size_t strcspn(const char *s1, const char *s2);
size_t strspn(const char *s1, const char *s2);

char *strpbrk(const char *s1, const char *s2);

char *strstr(const char *s1, const char *s2);

void *memset(void *s, int c, size_t n);

size_t strlen(const char *s);

/* POSIX */
int strncasecmp(const char *s1, const char *s2, size_t n);

#endif /* __JET_STRING_H__ */
