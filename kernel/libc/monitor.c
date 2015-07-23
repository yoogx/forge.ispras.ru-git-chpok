

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

int atoi(char * s)
{
    int i, n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; ++i)
        n = 10 * n + (s[i] - '0');
    return n;
}



int NOT_EXIT=1; // 0 if we want to exit from console

int mon_help(int argc, char **argv);

int help_about(int argc,char **argv);

int print_partition(int argc, char **argv); // List of partition

int pause_N(int argc, char **argv);// pause partition N. 

int resume_N(int argc, char **argv); // continue partition N

int restart_N(int argc, char **argv); // restart pertition N

int exit_from_monitor(int argc, char **argv); //exit from monitor

int info_partition(int argc,char ** argv);

struct Command {
	const char *name;
	const char *desc;
	int (*func)(int argc, char** argv);
};

static struct Command commands[] = {
	{ "help", "Display all list of commands", mon_help },
    { "help_about" , "Display descriptions of this commands", help_about},
    {"ps","Display list of partition",print_partition},
    {"info_partition","Display information about this partition",info_partition},
    {"pause","Pause this partition",pause_N},
    {"resume","Continue this partition",resume_N},
    {"restart","Restart this partition",restart_N},
    {"exit","Exit from console",exit_from_monitor},
};

/* 
 * Implementations of monitor commands
 */

int
mon_help(int argc, char **argv)
{
	int i;
	(void) argc;
	(void) argv;
	for (i = 0; i < NCOMMANDS; i++)
		printf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int 
help_about(int argc,char **argv)
{
    if (argc > 2){
        printf("Too many arguments for help_about!\n");
        return 0;
    }
    if (argc == 1){
        printf("Missing parameter - Name of command!\n");
        return 0;
    }    
    for (int i = 0; i < NCOMMANDS; i++) {
        if (strcmp(argv[1], commands[i].name) == 0){
            printf("%s\n",commands[i].desc);
            return 0;            
        }    
    }
    printf("There is no such command!\n");
    return 0;
}

int 
print_partition(int argc, char **argv){

   pok_partition_id_t number_of_current_partition;
   pok_current_partition_get_id(&number_of_current_partition);
   if (POK_CONFIG_NB_PARTITIONS > 1) 
            printf("There are %d partitions:\n",POK_CONFIG_NB_PARTITIONS);
        else 
            printf("There is %d partition:\n",POK_CONFIG_NB_PARTITIONS);
   for (int i = 0 ; i < POK_CONFIG_NB_PARTITIONS ; i++){
        printf("Partition %d: %s",i,pok_partitions[i].name);
        if (i == number_of_current_partition) printf(" - current partition");
        printf("\n");    
   }
   (void) argc;
   (void) argv;
   return 0;
}

int info_partition(int argc,char **argv){
    if (argc > 2){
        printf("Too many arguments for info_partition!\n");
        return 0;
    }
    if (argc == 1){
        printf("Missing parameter - Number of partition!\n");
        return 0;
    }    
    int number=0;
    number=atoi(argv[1]);
    if (number >= POK_CONFIG_NB_PARTITIONS) {
        printf("There is no such partition!\n");
        return 0;        
    }
    
    printf("\n\n");
    printf("Info about partition #%d\n",number);
    printf("is_paused = %d\n",pok_partitions[number].is_paused);
    printf("base_addr = 0x%lx\n",pok_partitions[number].base_addr);     
    printf("base_vaddr = 0x%lx\n",pok_partitions[number].base_vaddr);   	
    printf("size = 0x%lx\n",pok_partitions[number].size);  	
    printf("name = %s\n",pok_partitions[number].name);    	
    printf("nthreads = %lu\n",pok_partitions[number].nthreads);    	
    printf("priority = 0x%x\n",pok_partitions[number].priority);  	
    printf("period = %lu\n",pok_partitions[number].period);   	
    printf("activation = %llu\n",pok_partitions[number].activation);     	
    printf("prev_thread = %lu\n",pok_partitions[number].prev_thread); 	
    printf("current_thread = %lu\n",pok_partitions[number].current_thread);   	
    printf("thread_index_low = %lu\n",pok_partitions[number].thread_index_low);   	
    printf("thread_index_high = %lu\n",pok_partitions[number].thread_index_high);    	
    printf("thread_index = %lu\n",pok_partitions[number].thread_index);  	
#if defined(POK_NEEDS_LOCKOBJECTS) || defined(POK_NEEDS_ERROR_HANDLING)
    printf("lockobj_index_low = 0x%x\n",pok_partitions[number].lockobj_index_low);   	
    printf("lockobj_index_high = 0x%x\n",pok_partitions[number].lockobj_index_high);    	
    printf("nlockobjs = 0x%x\n",pok_partitions[number].nlockobjs);    	
#endif
    printf("thread_main = %d\n",pok_partitions[number].thread_main);	
#ifdef POK_NEEDS_IO
    //printf("io_min = %d\n",pok_partitions[number].io_min);    	
    //printf("io_max = %d\n",pok_partitions[number].io_max);    	
#endif
    //printf("lock_level = %d\n",pok_partitions[number].lock_level);   uint32 	
    printf("\n\n");

    
    return 0;
}

int 
pause_N(int argc, char **argv){
    if (argc > 2){
        printf("Too many arguments for pause!\n");
        return 0;
    }
    if (argc == 1){
        printf("Missing parameter - Number of partition!\n");
        return 0;
    }    
    int number=0;
    number=atoi(argv[1]);
    //Change mode of this partition to paused
    pok_partitions[number].is_paused=TRUE;
    printf("Partition %d paused\n",number);
    return 0;

}

int 
resume_N(int argc, char **argv){

    if (argc > 2){
        printf("Too many arguments for resume!\n");
        return 0;
    }
    if (argc == 1){
        printf("Missing parameter - Number of partition!\n");
        return 0;
    }    
    int number=0;
    number=atoi(argv[1]);
    //Change mode of this partition to not paused
    pok_partitions[number].is_paused=FALSE;
    printf("Partition %d resumed\n",number);
    return 0;

}

int 
restart_N(int argc, char **argv){

    if (argc > 2){
        printf("Too many arguments for restart!\n");
        return 0;
    }
    if (argc == 1){
        printf("Missing parameter - Number of partition!\n");
        return 0;
    }    
    int number=0;
    number=atoi(argv[1]);
    //Change mode of this partition and call reinit to restart it
    pok_partition_set_mode(number,POK_PARTITION_MODE_INIT_COLD);
    pok_partition_reinit(number);
//    pok_partitions[number].is_paused=FALSE;
    printf("Partition %d restarted\n",number);
    return 0;

}

int 
exit_from_monitor(int argc, char **argv){

	(void) argc;
	(void) argv;
    NOT_EXIT=0;
    return 0;

}




/*
 * Monitor command interpreter
 */

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf) 
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			printf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv);
	}
	printf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor()
{
	char *buf;
    NOT_EXIT=1;
	printf("Welcome to the monitor!\n");
	printf("Type 'help' for a list of commands.\n");


	while (NOT_EXIT == 1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf) < 0)
				break;
        	
    }
}


void pok_monitor_thread(void)
{
	printf("pok_monitor_thread\n");		
       pok_arch_preempt_enable(); //Initialize interrupts   
    for (;;) {
        if (data_to_read() == 1) {
            pok_arch_preempt_disable();			
            monitor();
            pok_arch_preempt_enable();        
        }
        #ifdef i386
        asm("hlt");
        #endif
    }
}

