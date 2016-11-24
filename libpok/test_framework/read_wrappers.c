#include "read_wrappers.h"
 

///////////////////////////// BUFFER //////////////////////////// 
 
pok_ret_t buffer_write_wrapper(void* param, uint8_t pre_created_id)
{
    int ret = 0;
    
    // check allowed values at kernel/include/uapi/types.h
    // TODO: check whether it matches the actual size of sent message (does it actually matter?)
    pok_message_size_t length = 8192;
    
    const pok_time_t timeout  = 100; // TODO: try another values
    
    ret = pok_buffer_send(pre_created_id, param, length, &timeout);  
    
    return ret;
}


pok_ret_t buffer_read_wrapper(uint8_t pre_created_id)
{
    int ret = 0;
    
    const pok_time_t timeout = 100; // TODO: try another values
    
    
    const void* message;       // out, address of received message (we don't know the exact type in advance)
    pok_message_size_t length; // out
    
    ret = pok_buffer_receive(pre_created_id, &timeout, message, &length); 
    
    return ret;
}

///////////////////////////// BLACKBOARD ////////////////////////
 
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
    
    char* message;                 // out 
    pok_message_size_t* length;    // out
    
    ret = pok_blackboard_read(pre_created_id, timeout, message, length);
    
    return ret;
}


