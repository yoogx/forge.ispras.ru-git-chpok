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
 * \file kernel/arch/x86/syscalls.c
 * \brief This file implement system-calls for x86 platform
 * \author Julian Pidancet
 * \author Julien Delange
 * \author Laurent Lec
 */

#include <core/debug.h>
#include <core/syscall.h>
#include <core/uaccess.h>
#include <interrupt.h>

void process_syscall(interrupt_frame* frame)
{
   jet_ret_t            syscall_ret;
   pok_syscall_args_t*  syscall_args;
   pok_syscall_id_t     syscall_id;

   /*
    * Get the syscall id in the eax register
    */
   syscall_id = (pok_syscall_id_t) frame->eax;

   // Address of the array of args is passed through ebx register.
   pok_syscall_args_t* __user syscall_args_user = (void*)frame->ebx;
   // Get kernel address of them.
   syscall_args = jet_user_to_kernel_typed(syscall_args_user);
   if(syscall_args == NULL)
   {
         syscall_ret = EINVAL;
   }
   else
   {
      /*
       * Perform the syscall baby !
       */
      syscall_ret = pok_core_syscall (syscall_id, syscall_args);
   }

   /*
    * And finally, put the return value in eax register
    */
   frame->eax = syscall_ret;
}
