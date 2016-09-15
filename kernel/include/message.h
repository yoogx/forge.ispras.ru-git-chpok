#ifndef __POK_MESSAGE_H__
#define __POK_MESSAGE_H__

#include <types.h>
#include <common.h>

typedef struct {
    /*
     * Size of the message.
     * 
     * Because of `unsigned long` type, futher `content` field will be
     * suitable aligned for perform fast memcpy().
     */
    unsigned long size;
    /* Content of the message. Open-bounds array.*/
    char content[1];
} pok_message_t;

/* 
 * Size of message structure, which can hold messages up to given size.
 * 
 * This respects sizeof() convention, so can be used for indexing array of messages.
 * 
 */
#define POK_MESSAGE_STRUCT_SIZE(max_size) \
    ALIGN_VAL(offsetof(pok_message_t, content) + max_size, sizeof(unsigned long))

/* 
 * Pair of [ptr,size] for store message into user space.
 * 
 * Normally this structure is allocated on the stack and
 * pointer to it is stored in thread->wait_private field.
 */
typedef struct
{
    const void* __kuser data;
    pok_message_size_t size;
} pok_message_send_t;


#endif /* __POK_MESSAGE_H__ */
