#ifndef __POK_ARCH_UACCESS_H__
#define __POK_ARCH_UACCESS_H__

// TODO: Revisit
static inline pok_bool_t arch_check_access(const void* __user addr, size_t size)
{
    unsigned long p = (unsigned long)addr;
    
    return (p >= 0x80000000) && ((p + size) < (0x80000000 + 0x1000000ULL))
}

static inline pok_bool_t arch_check_access_read(const void* __user addr, size_t size)
{
     return arch_check_access(addr, size);
}


static inline pok_bool_t arch_check_access_write(void* __user addr, size_t size)
{
     return arch_check_access(addr, size);
}

#endif /* __POK_ARCH_UACCESS_H__ */
