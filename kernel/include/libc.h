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
#include <stdarg.h>
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
int   strncasecmp(const char *s1, const char *s2, size_t size);

char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

void *memchr(const void *s, int c, size_t n);

typedef void (*t_putc)(int val, void *out);
// Generic printf-like function.
void vprintf(t_putc putc, void *out, const char* format, va_list *args) __attribute__ ((format(printf, 3, 0)));

#if defined (POK_NEEDS_CONSOLE) || defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_INSTRUMENTATION) || defined (POK_NEEDS_COVERAGE_INFOS)

int printf(const char *format, ...)__attribute__ ((format(printf, 1, 2)));

/* 
 * Unlike to standard C function, this returns number of characters which are ACTUALLY written.
 * (but terminating '\0' doesn't count, as usual).
 * 
 * Linux kernel has 'scnprintf' function with exactly that semantic.
 * 
 * TODO: Probably, rename this function for prevent result misconfusion?
 */
int snprintf(char *dst, unsigned size, const char *format, ...) __attribute__ ((format(printf, 3, 4)));
char * readline(const char *prompt);
int getchar(void);
int getchar2(void);
void monitor();

/*
struct  regs{
#ifdef __PPC__
    uint32_t r1;
    uint32_t offset1;
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
    uint32_t r14;
    uint32_t r15;
    uint32_t r16;
    uint32_t r17;
    uint32_t r18;
    uint32_t r19;
    uint32_t r20;
    uint32_t r21;
    uint32_t r22;
    uint32_t r23;
    uint32_t r24;
    uint32_t r25;
    uint32_t r26;
    uint32_t r27;
    uint32_t r28;
    uint32_t r29;
    uint32_t r30;
    uint32_t r31;
    uint32_t offset2;
    uint32_t offset3;
    uint32_t offset4;
    uint32_t offset5;
    uint32_t offset6;
    uint32_t lr;
#endif
#ifdef __i386__
  uint32_t es;
  uint32_t ds;
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t __esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t error;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t esp;
  uint32_t ss;
#endif  
};
*/

struct T_breakpoint{
    /*
     *  Number of thread 
     */
    int T_num;
    /*
     *  Number of partition 
     */
    int P_num;
    /*
     *  Number of breakpoint 
     */
    int B_num;
    /*
     *  Address of breakpoint 
     */
    uintptr_t addr;
    /*
     *  Address of breakpoint (set and used only in GDB client) 
     */
    uintptr_t C_addr;
    /*
     * Reason of breakpoint 
     * if 1, it was Partition breakpoint
     * if 2, it was simple breakpoint
     */
    int Reason;
    /*
     * Instruction which was on this adress
     */
#ifdef __PPC__
    char Instr[8];
#endif
#ifdef __i386__
    char Instr[2];
#endif
};

struct jet_interrupt_context;

void handle_exception (int exceptionVector, struct jet_interrupt_context* ea);

void pok_monitor_thread(void);
void pok_monitor_thread_init(void);
void pok_gdb_thread(void);
void pok_gdb_thread_init(void);
#endif /* NEEDS_CONSOLE or NEEDS_DEBUG */

#endif
