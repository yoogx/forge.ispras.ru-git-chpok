/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

#ifndef __POK_KERNEL_LIBC_H__
#define __POK_KERNEL_LIBC_H__

#include <config.h>

#include <types.h>

void  *memcpy(void * to, const void * from, size_t n);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *, const void *, size_t n);

/* avoid errors for windows */
/*__attribute__ ((weak))*/
void  *memset(void *dest, unsigned char val, size_t count);

int   strlen (const char* str);
int   strnlen (const char* str, size_t n);

char* strncpy(char* dest, const char* src, size_t n);

int   strcmp (const char *s1, const char *s2);
int   strncmp(const char *s1, const char *s2, size_t size);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

void *memchr(const void *s, int c, size_t n);



#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

int printf(const char *format, ...)__attribute__ ((format(printf, 1, 2)));
void snprintf(char *dst, unsigned size, const char *format, ...) __attribute__ ((format(printf, 3, 4)));
char * readline(const char *prompt);
int getchar(void);
void monitor();
void pok_monitor_thread(void);
void pok_monitor_thread_init();
#endif /* NEEDS_CONSOLE or NEEDS_DEBUG */

#endif
