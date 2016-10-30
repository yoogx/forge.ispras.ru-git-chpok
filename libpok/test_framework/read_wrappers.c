#include "wrappers.h"
 
 
pok_ret_t blackboard_write_wrapper(void* param, uint8_t pre_created_id)
{
    int ret = 0;
    pok_message_size_t len = 8192;
    
    ret = pok_blackboard_display(pre_created_id, param, len);   
    
    return ret;
}


pok_ret_t blackboard_read_wrapper(uint8_t pre_created_id)
{
    int ret = 0;
    const pok_time_t timeout = 100;
    
    char* message;                  // out 
    pok_message_size_t* out_len;    // out
    
    ret = pok_blackboard_read(pre_created_id, timeout, message, out_len);
    
    return ret;
}
