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


#include "interrupt.h"
#include "tss.h"
#include <gdb.h>
#include <libc.h>

extern void * pok_trap_addr;
pok_bool_t was_breakpoint=TRUE;


void update_tss (interrupt_frame* frame)
{
  if ((frame->cs & 0xffff) != 0x8)
  {
    pok_tss.esp0 = (uint32_t)frame + sizeof (interrupt_frame);
  }
}

#ifdef POK_NEEDS_GDB
void save_frame(interrupt_frame* frame)
{
  global_thread_stack = frame;
}
#endif /* POK_NEEDS_GDB */

void process_breakpoint(interrupt_frame* frame)
{
   (void)frame;
   printf("EXCEPTION breakpoint\n");
   frame->eip --;
//// pok_trap_addr = address of pok_trap in entry.S
   if (frame->eip == (unsigned) (&pok_trap_addr)){
       was_breakpoint=FALSE;
       handle_exception(17, frame);
   }else{
        handle_exception(3, frame);
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
