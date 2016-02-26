#ifndef __POK_COMMON_H__
#define __POK_COMMON_H__

/*
 * Mark pointer as pointer to user space.
 * 
 * Pointers of such types should be dereferenced only with special function
 * (see uaccess.h).
 * 
 * Some checkers can set this mark to something, which *actually*
 * prohibits incorrect usage of marked pointers.
 * 
 * Usage example:
 * 
 *    void a(char* __user buf);
 */
#define __user

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

/*
 * Return minimal value, which is greater-or-equal than size
 * and has corresponded alignment.
 * 
 * @align should be constant and be power of 2.
 */
static inline unsigned long ALIGN_SIZE(unsigned long size, unsigned long align)
{
    return (size + align - 1) & (~(align - 1));
}

#endif /* !__POK_COMMON_H__ */
