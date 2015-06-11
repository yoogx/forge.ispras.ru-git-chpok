
#include <libc.h>
/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */
/**
 * strrchr - Find the last occurrence of a character in a string
 * @s: The string to be searched
 * @c: The character to search for
 */
char *strrchr(const char *s, int c)
{
        const char *last = NULL;
        do {
                if (*s == (char)c)
                        last = s;
        } while (*s++);
        return (char *)last;
}
