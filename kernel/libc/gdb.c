#include <config.h>

#include <libc.h>
#include <bsp_common.h>
#include <arch.h>
#include <core/partition.h>
#include <core/sched.h>


//static pok_bool_t *partiton_on_pause;

void
gdb()
{
    printf("Welcome to GDB server!\n");
    pok_trap();
    printf("Exit from GDB server!\n");
}


void pok_gdb_thread(void)
{
    printf("pok_gdb_thread\n");

    //for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
    //    partiton_on_pause[i]=TRUE;
    //}
    //~ int j = 0;
    for (;;) {
        //~ j++;
        //~ if (j % 1000 == 0) printf("Waiting...\n");
        if (data_to_read_1() == 1) {
            pok_arch_preempt_disable();         
            gdb();
            pok_arch_preempt_enable();        
            //~ printf();
            
        }
    }
    printf("End of gdb func\n");
}

void gdb_process_error(pok_system_state_t partition_state,
        pok_error_id_t error_id,
        uint8_t state_byte_preempt_local,
        void* failed_address)
{
    pok_fatal("Error in monitor");
}


static const struct pok_partition_operations gdb_operations =
{
    .start = pok_gdb_thread,
    .process_partition_error = gdb_process_error
};


void pok_gdb_thread_init()
{
    
#ifdef POK_NEEDS_GDB
    partition_gdb.part_sched_ops = &partition_sched_ops_kernel;
    partition_gdb.part_ops = &gdb_operations;
    pok_dstack_alloc(&partition_gdb.initial_sp, 4096);
#endif
}


