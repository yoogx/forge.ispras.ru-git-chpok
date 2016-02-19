#ifndef __POK_COMMON_H__
#define __POK_COMMON_H__

/**
 * Casts pointer to member of structure to pointer to structure itself.
 * 
 * @ptr: the pointer to the member
 * @type: the type of the container struct
 * @member: name of the member within the container.
 * 
 * Implementation is taken from Linux kernel.
 */
#define container_of(ptr, type, member) ({                   \
    const typeof( ((type *)0)->member)* __mptr = (ptr);      \
    (type *)( (char*) __mptr - offsetof(type, member) ); })


#endif /* !__POK_COMMON_H__ */
