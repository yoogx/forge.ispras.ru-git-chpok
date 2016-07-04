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

/**
 * \file       arch/x86/exceptions.c
 * \author     Julian Pidancet
 */

#include <config.h>

#if defined (POK_NEEDS_DEBUG) || defined (POK_NEEDS_ERROR_HANDLING)

#include <errno.h>
#include <arch.h>
#include <core/debug.h>
#include <core/error.h>
#include <core/partition.h>
#include <libc.h>
#include "event.h"

void		exception_divide_error();
void		exception_debug();
void		exception_nmi();
void		exception_breakpoint();
void		exception_overflow();
void		exception_boundrange();
void		exception_invalidopcode();
void		exception_nomath_coproc();
void		exception_doublefault();
void		exception_copseg_overrun();
void		exception_invalid_tss();
void		exception_segment_not_present();
void		exception_stackseg_fault();
void		exception_general_protection();
void		exception_pagefault();
void		exception_fpu_fault();
void		exception_alignement_check();
void		exception_machine_check();
void		exception_simd_fault();

static const struct
{
  uint16_t	vector;
  void		(*handler)(void);
}
exception_list[] =
{
  { EXCEPTION_DIVIDE_ERROR, exception_divide_error},
  { EXCEPTION_DEBUG, exception_debug},
  { EXCEPTION_NMI, exception_nmi},
  { EXCEPTION_BREAKPOINT, exception_breakpoint},
  { EXCEPTION_OVERFLOW, exception_overflow},
  { EXCEPTION_BOUNDRANGE, exception_boundrange},
  { EXCEPTION_INVALIDOPCODE, exception_invalidopcode},
  { EXCEPTION_NOMATH_COPROC, exception_nomath_coproc},
  { EXCEPTION_DOUBLEFAULT, exception_doublefault},
  { EXCEPTION_COPSEG_OVERRUN, exception_copseg_overrun},
  { EXCEPTION_INVALID_TSS, exception_invalid_tss},
  { EXCEPTION_SEGMENT_NOT_PRESENT, exception_segment_not_present},
  { EXCEPTION_STACKSEG_FAULT, exception_stackseg_fault},
  { EXCEPTION_GENERAL_PROTECTION, exception_general_protection},
  { EXCEPTION_PAGEFAULT, exception_pagefault},
  { EXCEPTION_FPU_FAULT, exception_fpu_fault},
  { EXCEPTION_ALIGNEMENT_CHECK, exception_alignement_check},
  { EXCEPTION_MACHINE_CHECK, exception_machine_check},
  { EXCEPTION_SIMD_FAULT, exception_simd_fault},
  { 0, NULL}
};

pok_ret_t pok_exception_init()
{
  int i;

  for (i = 0; exception_list[i].handler != NULL; ++i)
  {
    pok_idt_set_gate (exception_list[i].vector,
		                GDT_CORE_CODE_SEGMENT << 3,
                      (uint32_t) exception_list[i].handler,
                      IDTE_INTERRUPT,
                      3);
  }

  return (POK_ERRNO_OK);
}

__attribute__((unused))
static void dump_registers (interrupt_frame *frame)
{
  printf ("ES: %lx, DS: %lx\n",  frame->es, frame->ds);
  printf ("CS: %lx, SS: %lx\n",  frame->cs, frame->ss);
  printf ("EDI: %lx, ESI: %lx\n", frame->edi, frame->esi);
  printf ("EBP: %lx, ESP: %lx\n", frame->ebp, frame->esp);
  printf ("EAX: %lx, ECX: %lx\n", frame->eax, frame->ecx);
  printf ("EDX: %lx, EBX: %lx\n", frame->edx, frame->ebx);
  printf ("EIP: %lx, ErrorCode: %lx\n", frame->eip, frame->error);
  printf ("EFLAGS: %lx\n\n", frame->eflags);
}

/* 
 * Raise error from interrupt handler.
 * 
 * If error handler is used, call pok_raise_error() with parameters
 * error_id, user_space, failed_addr passed to it and output formatted
 * message, given as rest parameters, with printf_debug().
 * 
 * Otherwise output short_message describes interrupt reason with pok_fatal().
 * 
 * Interrupt frame is ignored in any case.
 */
#ifdef POK_NEEDS_ERROR_HANDLING
#define raise_error_from_interrupt(error_id, user_space, failed_addr, short_message, ... /* message format and its args */) \
   do { (void)frame; printf_debug(__VA_ARGS__); \
   pok_raise_error(error_id, user_space, failed_addr); } while(0)
#else
#define raise_error_from_interrupt(error_id, user_space, failed_addr, short_message, ... /* message format and its args */) \
   do { (void)frame; pok_fatal(short_message); } while(0)
#endif

/* Helper. dump_registers() in debug mode, otherwise argument is ignored. */
#ifdef POK_NEEDS_DEBUG
#define dump_registers_debug(frame) dump_registers(frame)
#else
#define dump_registers_debug(frame) (void)frame
#endif

/* 
 * Raise error from interrupt handler and dump registers.
 * 
 * If error handler is used, call pok_raise_error() with parameters
 * error_id, user_space, failed_addr passed to it and output formatted
 * message, given as rest parameters, with printf_debug().
 * 
 * Otherwise output short_message describes interrupt reason with pok_fatal().
 * 
 * In all cases, if debugging is enabled then dump_registers() is called.
 */
#ifdef POK_NEEDS_ERROR_HANDLING
#define raise_error_from_interrupt_dump(error_id, user_space, failed_addr, short_message, ... /* message format and its args */) \
   do { dump_registers_debug(frame); printf_debug(__VA_ARGS__); \
   pok_raise_error(error_id, user_space, failed_addr); } while(0)
#else
#define raise_error_from_interrupt(error_id, user_space, failed_addr, short_message, ... /* message format and its args */) \
   do { dump_registers_debug(frame); pok_fatal(short_message); } while(0)
#endif


INTERRUPT_HANDLER (exception_divide_error)
{
   raise_error_from_interrupt(POK_ERROR_ID_NUMERIC_ERROR,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Divide error",
      "[KERNEL] Raise divide by zero error\n"
      );
}

INTERRUPT_HANDLER (exception_debug)
{
  (void)frame;
//~ #if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)
//~ 
   //~ #ifdef POK_NEEDS_DEBUG
  //~ printf ("[KERNEL] Raise debug fault\n");
   //~ #endif
//~ 
  //~ pok_error_from_exception (POK_ERROR_KIND_ILLEGAL_REQUEST);
//~ #else
//~ 
   //~ #ifdef POK_NEEDS_DEBUG
  //~ dump_registers(frame);
  //~ pok_fatal ("Debug fault");
   //~ #endif
//~ #endif
    handle_exception(3,(struct regs *)frame);
}

INTERRUPT_HANDLER (exception_nmi)
{
   raise_error_from_interrupt_dump(POK_ERROR_ID_ILLEGAL_REQUEST,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "NMI Interrupt",
      "[KERNEL] Raise exception NMI fault\n"
   );
}

extern void * pok_trap_addr;
pok_bool_t was_breakpoint=TRUE;

INTERRUPT_HANDLER (exception_breakpoint)
{
  (void)frame;
#ifdef DEBUG_GDB
   printf("EXEPTION breakpoint\n");
   dump_registers(frame);
#endif
   frame->eip --;
//// pok_trap_addr = address of pok_trap in entry.S
   if (frame->eip == (unsigned) (&pok_trap_addr)){
       was_breakpoint=FALSE;
       handle_exception(17,(struct regs *) frame);
   }else{
        handle_exception(3,(struct regs *) frame);
    }   
#ifdef DEBUG_GDB
    printf("es = 0x%lx\n",frame->es);
    printf("ds = 0x%lx\n",frame->ds);
    printf("edi = 0x%lx\n",frame->edi);
    printf("esi = 0x%lx\n",frame->esi);
    printf("ebp = 0x%lx\n",frame->ebp);
    printf("__esp = 0x%lx\n",frame->__esp);
    printf("ebx = 0x%lx\n",frame->ebx);
    printf("edx = 0x%lx\n",frame->edx);
    printf("ecx = 0x%lx\n",frame->ecx);
    printf("eax = 0x%lx\n",frame->eax);
    printf("error = 0x%lx\n",frame->error);
    printf("eip = 0x%lx\n",frame->eip);
    printf("cs = 0x%lx\n",frame->cs);
    printf("eflags = 0x%lx\n",frame->eflags);
    printf("esp = 0x%lx\n",frame->esp);
    printf("ss = 0x%lx\n",frame->ss);
#endif
    if (!was_breakpoint){
        frame->eip++;
    }
    was_breakpoint=TRUE;
#ifdef DEBUG_GDB
    printf("Exit from GDBserver\n");
#endif
}

INTERRUPT_HANDLER (exception_overflow)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_STACK_OVERFLOW,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Overflow",
      "[KERNEL] Raise exception overflow fault\n"
   );
}

INTERRUPT_HANDLER (exception_boundrange)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_STACK_OVERFLOW,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Bound range exceded",
      "[KERNEL] Raise exception bound range fault\n"
   );
}

INTERRUPT_HANDLER (exception_invalidopcode)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_ILLEGAL_REQUEST,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Invalid Opcode",
      "[KERNEL] Raise exception invalid opcode fault, EIP: 0x%lx\n",
           frame->eip
      );
}

INTERRUPT_HANDLER (exception_nomath_coproc)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_ILLEGAL_REQUEST,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "No Math Coprocessor",
      "[KERNEL] Raise exception no math coprocessor fault\n"
      );
}

INTERRUPT_HANDLER_errorcode (exception_doublefault)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_UNHANDLED_INT, // FIXME: does it make sense?
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Double Fault",
      "[KERNEL] Raise exception double fault\n"
      );
}

INTERRUPT_HANDLER (exception_copseg_overrun)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_MEMORY_VIOLATION,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Coprocessur Segment Overrun",
      "[KERNEL] Raise exception copseg overrun fault\n"
   );
}

INTERRUPT_HANDLER_errorcode (exception_invalid_tss)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_MEMORY_VIOLATION,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Invalid TSS",
      "[KERNEL] Raise exception invalid tss fault\n"
   );
}

INTERRUPT_HANDLER_errorcode (exception_segment_not_present)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_MEMORY_VIOLATION,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Segment Not Present",
      "[KERNEL] Raise exception segment not present fault\n"
   );
}

INTERRUPT_HANDLER_errorcode (exception_stackseg_fault)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_MEMORY_VIOLATION,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Stack-Segment Fault",
      "[KERNEL] Raise exception stack segment fault\n"
   );
}

INTERRUPT_HANDLER_errorcode (exception_general_protection)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_ILLEGAL_REQUEST,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "General Protection Fault",
      "[KERNEL] Raise exception general protection fault. EIP=0x%lx\n",
           frame->eip
      );
}

INTERRUPT_HANDLER_errorcode (exception_pagefault)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_MEMORY_VIOLATION,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Page Fault",
      "[KERNEL] Raise exception pagefault fault\n"
      );
}

INTERRUPT_HANDLER (exception_fpu_fault)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_HARDWARE_FAULT,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Floating Point Exception",
      "[KERNEL] Raise exception FPU fault\n"
   );
}

INTERRUPT_HANDLER_errorcode (exception_alignement_check)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_HARDWARE_FAULT,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Bad alignement",
      "[KERNEL] Raise exception alignment fault\n"
   );
}

INTERRUPT_HANDLER (exception_machine_check)
{
   raise_error_from_interrupt_dump (POK_ERROR_ID_HARDWARE_FAULT,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "Machine check error",
      "[KERNEL] Raise exception machine check fault\n"
   );
}

INTERRUPT_HANDLER (exception_simd_fault)
{
  raise_error_from_interrupt_dump (POK_ERROR_ID_HARDWARE_FAULT,
      /*TODO: User or kernel*/TRUE,
      /*TODO: Failed address*/NULL,
      "SIMD Fault",
      "[KERNEL] Raise exception SIMD fault\n"
   );
}

#endif

