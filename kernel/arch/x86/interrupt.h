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


#ifndef __POK_INTERRUPT_H__
#define __POK_INTERRUPT_H__


#include <types.h>
#include <arch/interrupt_frame.h>

/* Descriptor for concrete exception. */
struct exception_descriptor
{
  uint16_t	vector;
  void      (*handler)(void);
};

/* 
 * List of exception for register.
 * 
 * The last element in the list has NULL as handler field.
 */
extern const struct exception_descriptor exception_list[];

/*
 * If we returns to user space, restore esp0 field of tss.
 * This field is responsible for kernel stack processing interrupts
 * from user space.
 * 
 * Should be called when returning from interrupt.
 */
void update_tss (interrupt_frame* frame);

#ifdef POK_NEEDS_GDB
/* 
 * Save pointer to the frame, so it can be used in GDB.
 * 
 * Should be called when enter to interrupt.
 */
void save_frame(interrupt_frame* frame);
#endif /* POK_NEEDS_GDB */

void process_breakpoint(interrupt_frame* frame);
void process_syscall(interrupt_frame* frame);

#endif /* !__POK_INTERRUPT_H__ */
