/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <config.h>
#include <core/partition_arinc.h>

#ifdef POK_NEEDS_MONITOR
extern pok_partition_t partition_monitor;
#endif /* POK_NEEDS_MONITOR */

#ifdef POK_NEEDS_GDB
extern pok_partition_t partition_gdb;
#endif /* POK_NEEDS_GDB */


void for_each_partition(void (*f)(pok_partition_t* part))
{
#ifdef POK_NEEDS_MONITOR
   f(&partition_monitor);
#endif /* POK_NEEDS_MONITOR */

#ifdef POK_NEEDS_GDB
   f(&partition_gdb);
#endif /* POK_NEEDS_GDB */


   for(int i = 0; i < pok_partitions_arinc_n; i++)
   {
      f(&pok_partitions_arinc[i].base_part);
   }
}
