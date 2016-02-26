/*
 * Access to user space from kernel space.
 */

#ifndef __POK_UACCESS_H__
#define __POK_UACCESS_H__

#include <types.h>

// TODO: Single <asm/uaccess.h>
#ifdef __PPC__
#include <arch/ppc/uaccess.h>
#endif

//#ifdef POK_ARCH_X86
#ifdef __i386__
#include <arch/x86/uaccess.h>
#endif

//#ifdef POK_ARCH_SPARC
#ifdef __sparc__
#include <arch/sparc/uaccess.h>
#endif

/* Check that user space can read area specified. */
static inline pok_bool_t check_access_read(const void* __user addr, size_t size)
{
    arch_check_access_read(addr, size);
}

/* Check that user space can write area specified. */
static inline pok_bool_t check_access_write(void* __user addr, size_t size)
{
    arch_check_access_write(addr, size);
}

/* 
 * Check that user space can read and write area specified.
 * 
 * TODO: Is this symlink needed?
 */
static inline pok_bool_t check_access_rw(void* __user addr, size_t size)
{
    return check_access_write(addr, size);
}


/* 
 * Copy from user memory to kernel one.
 * 
 * NOTE: Access check should be performed before.
 */
static inline void __copy_from_user(const void* __user from, void* to, size_t n)
{
    assert(check_access_read(from, n));

    memcpy(to, from, n);
}

/* 
 * Copy from kernel memory to user one.
 * 
 * NOTE: Access check should be performed before.
 */
static inline void __copy_to_user(const void* from, void* __user to, size_t n)
{
    assert(check_access_write(to, n));
    
    memcpy(to, from, n);
}

/* 
 * Copy from user memory to kernel one.
 * 
 * Return TRUE on success, FALSE if user memory is invalid.
 */
static inline pok_bool_t copy_from_user(const void* __user from, void* to, size_t n)
{
    if(!check_access_read(from, n)) return FALSE;
    
    memcpy(to, from, n);
    
    return TRUE;
}

/* 
 * Copy from kernel memory to user one.
 * 
 * Return TRUE on success, FALSE if user memory is invalid.
 */
static inline pok_bool_t copy_to_user(const void* from, void* __user to, size_t n)
{
    if(!check_access_write(to, n)) return FALSE;
    
    memcpy(to, from, n);
    
    return TRUE;
}

/* Check that given *typed* user area is readable. */
#define check_user_read(ptr) check_access_read(ptr, sizeof(*ptr))
/* Check that given *typed* user area is writable. */
#define check_user_write(ptr) check_access_write(ptr, sizeof(*ptr))

/* 
 * Return value from *typed* user memory.
 * 
 * NOTE: Access check should be performed before.
 */
#define __get_user(ptr) ({typeof(*(ptr)) __val = *(ptr); val; })


/* 
 * Return value of the field in user-space structure.
 * 
 * NOTE: Access check should be performed before.
 */
#define __get_user_f(ptr, field) ({typeof((ptr)->field) __val = (ptr)->field; val; })


/* 
 * Put value to *typed* user memory.
 * 
 * NOTE: Access check should be performed before.
 */
#define __put_user(ptr, val) do {*ptr = (typeof(*ptr))val; } while(0)

/* 
 * Set field of user-space structure.
 * 
 * NOTE: Access check should be performed before.
 */
#define __put_user_f(ptr, field, value) do {(ptr)->field = (typeof((ptr)->field))val; } while(0)


/* 
 * Copy name to the user space.
 * 
 * Destination buffer should be checked before.
 */
static inline void pok_copy_name_to_user(const char* name, void* __user to)
{
    // How many bytes to copy.
    size_t n = strnlen(name, MAX_NAME_LENGTH);
    if(n != MAX_NAME_LENGTH) n++; // null-byte should be copied too.
    
    __copy_to_user(name, to, n);
}


#endif __POK_UACCESS_H__
