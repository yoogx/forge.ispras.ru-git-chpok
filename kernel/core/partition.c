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
