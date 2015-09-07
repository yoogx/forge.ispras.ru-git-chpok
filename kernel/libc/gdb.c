#include <config.h>

#include <libc.h>
#include <bsp.h>
#include <arch.h>
#include <core/partition.h>


#define NCOMMANDS 8 //Number of commands, change it if you want to 
                    //add a new command.



/*
 *  Don't use '[' in your command!
 */



/*
 * Convert string in int
 */
 

void
gdb()
{
    char *buf;
    pok_bool_t want_to_exit=FALSE;
    printf("Welcome to the monitor!\n");
    printf("Type 'help' for a list of commands.\n");


    while (!want_to_exit) {
        buf = readline("K> ");
        if (buf != NULL)
            //if (runcmd(buf) < 0)
                break;
            
    }
}


void pok_gdb_thread(void)
{
    printf("pok_gdb_thread\n");     
    pok_arch_preempt_enable(); //Initialize interrupts   
    //for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
    //    partiton_on_pause[i]=TRUE;
    //}
    for (;;) {
        if (data_to_read_1() == 1) {
            /*
             * Set all partition on pause
             */
            //for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
            //    if (!pok_partitions[i].is_paused){ 
                    //partiton_on_pause[i]=FALSE;
                    //pok_partitions[i].is_paused=TRUE;
            //    }
            //}
            
            //pok_arch_preempt_disable();         
            gdb();
            //pok_arch_preempt_enable();        
            
            //for (int i=0; i < POK_CONFIG_NB_PARTITIONS; i++){
            //    if (!partiton_on_pause[i]){ 
             //       pok_partitions[i].is_paused=FALSE;
            //    }
            //}
        }
        #ifdef i386
        asm("hlt");
        #endif
    }
}

void pok_gdb_thread_init()
{
//#ifdef POK_NEEDS_MONITOR
    //pok_threads[MONITOR_THREAD].entry = pok_monitor_thread;
    //pok_threads[MONITOR_THREAD].sp = pok_context_create(MONITOR_THREAD, 4096, (uintptr_t) pok_monitor_thread);
    //pok_bool_t tmp[POK_CONFIG_NB_PARTITIONS];
    //partiton_on_pause = tmp;
//#endif
}


