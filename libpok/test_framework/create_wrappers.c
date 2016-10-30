#include "wrappers.h"


uint8_t  create_entity (int entity_type, char* pre_created_name)
{
    RETURN_CODE_TYPE ReturnCode;
    long id;
    
    switch (entity_type)
    {
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
