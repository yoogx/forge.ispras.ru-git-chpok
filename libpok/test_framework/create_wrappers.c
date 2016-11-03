#include "create_wrappers.h"

uint8_t  create_entity (int entity_type, char* pre_created_name)
{
    RETURN_CODE_TYPE ReturnCode;
    long id;
    
    switch (entity_type)
    {
        case STARTED_PROCESS:
            ;
            
		    PROCESS_ATTRIBUTE_TYPE process_attrs = {
		        .PERIOD = INFINITE_TIME_VALUE,
		        .TIME_CAPACITY = INFINITE_TIME_VALUE,
		        .STACK_SIZE = 8096, // the only accepted stack size!
		        .BASE_PRIORITY = MIN_PRIORITY_VALUE,
		        .DEADLINE = SOFT,
		    };
		
		    // create process
		    process_attrs.ENTRY_POINT = entry_point_method;
		    strncpy(process_attrs.NAME, "pre_created process", sizeof(PROCESS_NAME_TYPE));
		
		    CREATE_PROCESS(&process_attrs, &id, &ReturnCode);
		    if (ReturnCode != NO_ERROR) {
		        printf("couldn't create pre_created process: %d\n", (int) ReturnCode);
		        return 1;
		    } else {
		        printf("pre_created process created\n");
		    }
		    
		    // start process
		    START(ReturnCode, &ReturnCode);
		    if (ReturnCode != NO_ERROR) {
		        printf("couldn't start pre_created process: %d\n", (int) ReturnCode);
		        return 1;
		    } else {
		        printf("pre_created process \"started\" (it won't actually run until operating mode becomes NORMAL)\n");
		    }
            
            
            break;
            
        case BUFFER:
            ;
            
            CREATE_BUFFER(pre_created_name, sizeof(int), 10, FIFO, &id, &ReturnCode);
            
            if (ReturnCode != NO_ERROR) {
                printf("error creating a buffer: %d\n", (int) ReturnCode);
                return 1;
            } else {
                printf("buffer successfully created\n");
            }
            return id;
        
            break;
        
        case BLACKBOARD:
            ;
            
            CREATE_BLACKBOARD(pre_created_name, sizeof(int), &id, &ReturnCode);
            
            if (ReturnCode != NO_ERROR) {
                printf("error creating a blackboard: %d\n", (int) ReturnCode);
                return 1;
            } else {
                printf("buffer successfully created\n");
            }
            return id;
        
            ;
            break;
        
        case SEMAPHORE:
            ;
            
            CREATE_SEMAPHORE(pre_created_name, 0, 5, FIFO, &id, &ReturnCode);
            
            if (ReturnCode != NO_ERROR) {
                printf("error creating a semaphore: %d\n", (int) ReturnCode);
                return 1;
            } else {
                printf("buffer successfully created\n");
            }
            return id;
            
            break;
            
        case EVENT:
            ;
            
            CREATE_EVENT(pre_created_name, &id, &ReturnCode);
            
            if (ReturnCode != NO_ERROR) {
                printf("error creating an event: %d\n", (int) ReturnCode);
                return 1;
            } else {
                printf("buffer successfully created\n");
            }
            return id;
            break;
    }
}
