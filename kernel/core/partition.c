#include <config.h>
#include <core/partition_arinc.h>

#ifdef POK_NEEDS_MONITOR
extern pok_partition_t partition_monitor;
#endif /* POK_NEEDS_MONITOR */


void for_each_partition(void (*f)(pok_partition_t* part))
{
#ifdef POK_NEEDS_MONITOR
   f(&partition_monitor);
#endif /* POK_NEEDS_MONITOR */

   for(int i = 0; i < pok_partitions_arinc_n; i++)
   {
      f(&pok_partitions_arinc[i].base_part);
   }
}
