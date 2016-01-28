#include <config.h>

#include <libc.h>
#include <bsp.h>
#include <arch.h>
#include <core/partition.h>


void pok_trap();

static pok_bool_t *partiton_on_pause;


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
    pok_arch_preempt_enable(); //Initialize interrupts   
    for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
        partiton_on_pause[i]=TRUE;
    }
    //~ int j = 0;
    for (;;) {
        //~ j++;
        //~ if (j % 1000 == 0) printf("Waiting...\n");
        if (data_to_read_1() == 1) {
            printf("\n\nIt works!!!!!\n\n");

            pok_arch_preempt_disable();         
            gdb();
            pok_arch_preempt_enable();        
            //~ printf();
            
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


