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

#include <config.h>

#include <errno.h>
#include <kernel_shared_data.h>

int main();

int __pok_partition_start ()
{
   // Setup user-only fields of kernel shared data.
   kshd.main_thread_id = kshd.current_thread_id;

   main(); /* main loop from user */
   return (0);
}
