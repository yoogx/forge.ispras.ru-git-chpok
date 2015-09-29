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

int   strcmp (const char *s1, const char *s2);
int   strncmp(const char *s1, const char *s2, size_t size);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

void *memchr(const void *s, int c, size_t n);



#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

int printf(const char *format, ...)__attribute__ ((format(printf, 1, 2)));
void snprintf(char *dst, unsigned size, const char *format, ...) __attribute__ ((format(printf, 3, 4)));
char * readline(const char *prompt);
char * readline2(const char *prompt);
int getchar(void);
int getchar2(void);
void monitor();
struct  regs{
    uint32_t r1;
    uint32_t offset2;
    uint32_t cr;
    uint32_t r0;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t r13;
    uint32_t ctr;
    uint32_t xer;
    uint32_t srr0; // == pc
    uint32_t srr1;
    uint32_t offset3;
    uint32_t offset4;
    uint32_t offset5;
    uint32_t offset6;
    uint32_t offset7;
    uint32_t lr;
    uint32_t offset8;
    uint32_t offset9;
    uint32_t offset10;
    uint32_t offset11;
    uint32_t offset12;
    uint32_t offset13;
    uint32_t offset14;
    uint32_t offset15;
};

void handle_exception (int exceptionVector, struct regs * ea);

void pok_monitor_thread(void);
void pok_monitor_thread_init();
void pok_gdb_thread(void);
void pok_gdb_thread_init();
#endif /* NEEDS_CONSOLE or NEEDS_DEBUG */

#endif
