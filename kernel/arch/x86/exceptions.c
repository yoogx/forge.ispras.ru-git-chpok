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

/*#if defined(POK_NEEDS_ERROR_HANDLING)
static
void pok_error_from_exception(pok_error_kind_t error)
{
    POK_ERROR_CURRENT_THREAD(error);
    
    // call scheduler (which will then switch to error handler anyway (probably))
    // TODO: make a shortcut: switch to handler immediately (if it's created, ofc)
    pok_sched(); 
}
#endif*/

INTERRUPT_HANDLER (exception_divide_error)
{
  (void) frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise divide by zero error\n");
#endif
  pok_raise_error(POK_ERROR_ID_NUMERIC_ERROR,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);
#else
  pok_fatal ("Divide error");
#endif
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
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

   #ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] Raise exception NMI fault\n");
   #endif

   pok_raise_error (POK_ERROR_ID_ILLEGAL_REQUEST,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);
#else

#ifdef POK_NEEDS_DEBUG
   dump_registers(frame);
   pok_fatal ("NMI Interrupt");
#endif
#endif
}

extern void * pok_trap_addr;
pok_bool_t was_breakpoint=TRUE;

INTERRUPT_HANDLER (exception_breakpoint)
{
  (void)frame;
   printf("EXEPTION breakpoint\n");
   dump_registers(frame);
   frame->eip --;
//// pok_trap_addr = address of pok_trap in entry.S
   if (frame->eip == (unsigned) (&pok_trap_addr)){
       was_breakpoint=FALSE;
       handle_exception(17,(struct regs *) frame);
   }else{
        handle_exception(3,(struct regs *) frame);
    }   
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
    if (!was_breakpoint){
        frame->eip++;
    }
    was_breakpoint=TRUE;
    printf("Exit from GDBserver\n");
}

INTERRUPT_HANDLER (exception_overflow)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

   #ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] Raise exception overflow fault\n");
   #endif

  pok_raise_error (POK_ERROR_ID_STACK_OVERFLOW,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Overflow");
#endif
#endif
}

INTERRUPT_HANDLER (exception_boundrange)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] Raise exception bound range fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_STACK_OVERFLOW,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
      dump_registers(frame);
      pok_fatal ("Bound range exceded");
#endif
#endif
}

INTERRUPT_HANDLER (exception_invalidopcode)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] Raise exception invalid opcode fault, EIP: 0x%lx\n",
           frame->eip);
#endif

  pok_raise_error (POK_ERROR_ID_ILLEGAL_REQUEST,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);
#else
#ifdef POK_NEEDS_DEBUG
      dump_registers(frame);
      pok_fatal ("Invalid Opcode");
#endif
#endif
}

INTERRUPT_HANDLER (exception_nomath_coproc)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] Raise exception no math coprocessor fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_ILLEGAL_REQUEST,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else

#ifdef POK_NEEDS_DEBUG
      dump_registers(frame);
      pok_fatal ("Invalid No Math Coprocessor");
#endif

#endif
}

INTERRUPT_HANDLER_errorcode (exception_doublefault)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception double fault\n");
#endif

  // FIXME: does it make sense?
  pok_raise_error (POK_ERROR_ID_UNHANDLED_INT,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Double Fault");
#endif
#endif
}

INTERRUPT_HANDLER (exception_copseg_overrun)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception copseg overrun fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_MEMORY_VIOLATION,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Coprocessur Segment Overrun");
#endif
#endif
}

INTERRUPT_HANDLER_errorcode (exception_invalid_tss)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception invalid tss fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_MEMORY_VIOLATION,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Invalid TSS");
#endif
#endif
}

INTERRUPT_HANDLER_errorcode (exception_segment_not_present)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception segment not present fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_MEMORY_VIOLATION,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Segment Not Present");
#endif
#endif
}

INTERRUPT_HANDLER_errorcode (exception_stackseg_fault)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception stack segment fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_MEMORY_VIOLATION,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Stack-Segment Fault");
#endif
#endif
}

INTERRUPT_HANDLER_errorcode (exception_general_protection)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception general protection fault. EIP=0x%lx\n",
           frame->eip);
#endif

  pok_raise_error (POK_ERROR_ID_ILLEGAL_REQUEST,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("General Protection Fault");
#endif
#endif
}

INTERRUPT_HANDLER_errorcode (exception_pagefault)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)
#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception pagefault fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_MEMORY_VIOLATION,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);
#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Page Fault");
#endif
#endif
}

INTERRUPT_HANDLER (exception_fpu_fault)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception FPU fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_HARDWARE_FAULT,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Floating Point Exception");
#endif
#endif
}

INTERRUPT_HANDLER_errorcode (exception_alignement_check)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception alignment fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_HARDWARE_FAULT,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);
#else
#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("Bad alignement");
#endif
#endif
}

INTERRUPT_HANDLER (exception_machine_check)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
   printf ("[KERNEL] Raise exception machine check fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_HARDWARE_FAULT,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);

#else
#ifdef POK_NEEDS_DEBUG
  pok_fatal ("Machine check error");
  dump_registers(frame);
#endif
#endif
}

INTERRUPT_HANDLER (exception_simd_fault)
{
  (void)frame;
#if defined (POK_NEEDS_PARTITIONS) && defined (POK_NEEDS_ERROR_HANDLING)

#ifdef POK_NEEDS_DEBUG
  printf ("[KERNEL] Raise exception SIMD fault\n");
#endif

  pok_raise_error (POK_ERROR_ID_HARDWARE_FAULT,
    /*TODO: User or kernel*/TRUE,
    /*TODO: Failed address or kernel*/NULL);
#else

#ifdef POK_NEEDS_DEBUG
  dump_registers(frame);
  pok_fatal ("SIMD Fault");
#endif
#endif
}

#endif

