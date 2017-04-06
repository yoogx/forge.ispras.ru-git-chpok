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
 * \file    core/boot.c
 * \author  Julien Delange
 * \brief   Boot function to start the kernel
 * \date    2008-2009
 */

#include <config.h>

#include <cons.h>

#include <core/time.h>
#include <core/thread.h>
#include <core/sched.h>
#include <core/partition.h>
#include <core/partition_arinc.h>
#include <core/channel.h>
#include <asp/entries.h>
#include <libc.h>

#ifdef POK_NEEDS_GDB
#include <gdb.h>
#endif

#ifdef KERNEL_UNITTESTS
#include <unity_fixture.h>
void RunAllTests(void);
#endif

void jet_boot (void)
{
   kernel_state = POK_SYSTEM_STATE_OS_MOD; // TODO: is this choice for state right?

   pok_partition_arinc_init_all();
#ifdef POK_NEEDS_MONITOR
   pok_monitor_thread_init();
#endif
#ifdef POK_NEEDS_GDB
   pok_gdb_thread_init();
#endif

   pok_sched_init ();

   pok_channels_init_all ();

   printf("POK kernel initialized\n");

#if defined(POK_NEEDS_GDB) && defined(POK_NEEDS_WAIT_FOR_GDB)
  printf("Waiting for GDB connection ...\n");
  printf("\n");
  pok_trap();
#endif

#ifdef KERNEL_UNITTESTS
  const char* argv[] = {"kernel", "-v"};
  UnityMain(2, argv, RunAllTests);
#else
  pok_sched_start();
#endif
}
