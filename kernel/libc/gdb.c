#include <config.h>

#include <libc.h>
#include <bsp.h>
#include <arch.h>
#include <core/partition.h>


static pok_bool_t *partiton_on_pause;


void
gdb()
{
    printf("Welcome to GDB server!\n");
    asm("trap");
    printf("Exit from GDB server!\n");
}


void pok_gdb_thread(void)
{
    printf("pok_gdb_thread\n");     
    pok_arch_preempt_enable(); //Initialize interrupts   
    for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
        partiton_on_pause[i]=TRUE;
    }
    for (;;) {
        if (data_to_read_1() == 1) {
            printf("\n\nIt works!!!!!\n\n");
            /*
             * Set all partition on pause
             */
            pok_arch_preempt_disable();         
            for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
                if (!pok_partitions[i].is_paused){ 
                    partiton_on_pause[i]=FALSE;
                    pok_partitions[i].is_paused=TRUE;
                }
            }
            
            gdb();
            
            for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
                if (!partiton_on_pause[i]){ 
                    pok_partitions[i].is_paused=FALSE;
                }
            }
            pok_arch_preempt_enable();        
        }
    }
    printf("End of gdb func\n");
}

void pok_gdb_thread_init()
{
    
#ifdef POK_NEEDS_GDB
    pok_threads[GDB_THREAD].entry = pok_gdb_thread;
    pok_threads[GDB_THREAD].sp = pok_context_create(GDB_THREAD, 4096, (uintptr_t) pok_gdb_thread);
#endif
}


