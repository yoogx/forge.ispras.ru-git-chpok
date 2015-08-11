#include <config.h>
#include <core/thread.h>
#include <core/partition.h>

struct pok_space
{
  uintptr_t     phys_base;
  size_t        size;
};

pok_lockobj_t *pok_partitions_lockobjs;
pok_thread_t *pok_threads;
pok_partition_t *pok_partitions;
struct pok_space *spaces;

void pok_config_init() {
    pok_lockobj_t tmp_pok_partitions_lockobjs[POK_CONFIG_NB_LOCKOBJECTS + 1];
    pok_partitions_lockobjs = tmp_pok_partitions_lockobjs;
    
    pok_thread_t tmp_pok_threads[POK_CONFIG_NB_THREADS];
    pok_threads = tmp_pok_threads;
    
    pok_partition_t tmp_pok_partitions[POK_CONFIG_NB_PARTITIONS];
    pok_partitions = tmp_pok_partitions;

    struct pok_space tmp_spaces[POK_CONFIG_NB_PARTITIONS];
    spaces = tmp_spaces;
}
