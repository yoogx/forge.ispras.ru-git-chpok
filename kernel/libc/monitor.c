

#include <libc.h>
#include <bsp.h>
#include <arch.h>
#include <core/partition.h>


#define NCOMMANDS 8 //Number of commands, change it if you want to 
		            //add a new command.


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



int NOT_EXIT=1;

int mon_help(int argc, char **argv);

int help_about(int argc,char **argv);

int print_partition(int argc, char **argv); // List of partition

int pause_N(int argc, char **argv);// pause partition N. Когда выпадает её время в
                                    // расписании,                         
                                    //то процессор ставится на паузу до следующего прерывания

int resume_N(int argc, char **argv); // continue partition N

int restart_N(int argc, char **argv); // restart pertition N

int exit_from_monitor(int argc, char **argv); //exit from monitor

int info_partition(int argc,char ** argv);

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
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

/***** Implementations of basic kernel monitor commands *****/

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


   if (POK_CONFIG_NB_PARTITIONS > 1) 
                printf("There are %d partitions:\n",POK_CONFIG_NB_PARTITIONS);
        else 
                printf("There is %d partition:\n",POK_CONFIG_NB_PARTITIONS);
    for (int i = 0 ; i < POK_CONFIG_NB_PARTITIONS ; i++){
        printf("%d) %s\n",i+1,pok_partitions[i].name);    
    }
    //TODO your code here
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
    

    //TODO read int from string argv[1]        
    //printf("Info about partition №%d\n",argv[1]);
    //printf("base_addr = %d\n",pok_partitions[argv[1]].base_addr); //TODO    uint32     
    //printf("base_vaddr = %d\n",pok_partitions[argv[1]].base_vaddr);   uint32 	
    //printf("size = %d\n",pok_partitions[argv[1]].size);  uint32  	
    //printf("name = %s\n",pok_partitions[argv[1]].name);    	
    //printf("nthreads = %d\n",pok_partitions[argv[1]].nthreads); uint32    	
    //printf("priority = %d\n",pok_partitions[argv[1]].priority); uint8  	
    //printf("period = %d\n",pok_partitions[argv[1]].period); uint32   	
    //printf("activation = %d\n",pok_partitions[argv[1]].activation); uint64    	
    //printf("prev_thread = %d\n",pok_partitions[argv[1]].prev_thread); uint32  	
    //printf("current_thread = %d\n",pok_partitions[argv[1]].current_thread); uint32   	
    //printf("thread_index_low = %d\n",pok_partitions[argv[1]].thread_index_low); uint32   	
    //printf("thread_index_high = %d\n",pok_partitions[argv[1]].thread_index_high); uint32    	
    //printf("thread_index = %d\n",pok_partitions[argv[1]].thread_index); uint32   	
#if defined(POK_NEEDS_LOCKOBJECTS) || defined(POK_NEEDS_ERROR_HANDLING)
    //printf("lockobj_index_low = %d\n",pok_partitions[argv[1]].lockobj_index_low);  uint8  	
    //printf("lockobj_index_high = %d\n",pok_partitions[argv[1]].lockobj_index_high);    	uint8
    //printf("nlockobjs = %d\n",pok_partitions[argv[1]].nlockobjs);    	uint8
#endif
    //printf("thread_main = %d\n",pok_partitions[argv[1]].thread_main);    pok_thread_id_t	
#ifdef POK_NEEDS_IO
    //printf("io_min = %d\n",pok_partitions[argv[1]].io_min);    	
    //printf("io_max = %d\n",pok_partitions[argv[1]].io_max);    	
#endif
    //printf("lock_level = %d\n",pok_partitions[argv[1]].lock_level);   uint32 	
    //printf(" = %d\n",pok_partitions[argv[1]].);    	
    //printf(" = %d\n",pok_partitions[argv[1]].);    	

    
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
    //Change mode of this partition to stopped.
    pok_partition_set_mode(number,POK_PARTITION_MODE_IDLE);

 
    //TODO your code here
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
 
    //Change mode of this partition to normal.
    pok_partition_set_mode(number,POK_PARTITION_MODE_NORMAL);    


    //TODO your code here
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
 
    //TODO your code here
    return 0;

}

int 
exit_from_monitor(int argc, char **argv){

	(void) argc;
	(void) argv;
    NOT_EXIT=0;
    return 0;

}




/***** Kernel monitor command interpreter *****/

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

