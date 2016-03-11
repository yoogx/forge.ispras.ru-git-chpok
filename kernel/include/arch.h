/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */

/**
 * \file    kernel/include/arch.h
 * \author  Julian Pidancet
 * \author  Julien Delange
 * \date    2008-2009
 * \brief   Generic interface to handle architectures
 */

#ifndef __POK_ARCH_H__
#define __POK_ARCH_H__

#include <types.h>
#include <errno.h>
#include <bsp.h>

// TODO: Where should be that definition?
#define KERNEL_STACK_SIZE_DEFAULT 8192

struct pok_space
{
    uintptr_t     phys_base;
    size_t        size;
};

extern struct pok_space spaces[];

/**
 * Function that initializes architecture concerns.
 */
pok_ret_t   pok_arch_init ();

/**
 * Disable interruptions
 */
pok_ret_t   pok_arch_preempt_disable ();

/**
 * Enable interruptions
 */
pok_ret_t   pok_arch_preempt_enable ();

/**
 * Returns true if interrupts are enabled
 */
pok_bool_t pok_arch_preempt_enabled(void);

/**
 * Function that do nothing. Useful for the idle task for example.
 */
pok_ret_t   pok_arch_idle ();

/**
 * Register an event (for example, an interruption)
 */
pok_ret_t   pok_arch_event_register (uint8_t vector, void (*handler)(void));

/**
 * Initialize `context` on the given stack.
 * 
 * Return stack pointer, which can be used by pok_context_switch() to
 * jump into given entry with given stack.
 */
uint32_t pok_context_init(uint32_t sp, void (*entry)(void));


/**
 * pok_stack_alloc + pok_context_init.
 */
/*uint32_t pok_context_create (uint32_t thread_id,
                                uint32_t stack_size,
                                void (*entry)(void))
{
    uint32_t sp = pok_stack_alloc(stack_size);
    (void)thread_id;
    
    return pok_context_init(sp, entry);
}*/

/**
 * Switch to context, stored in @new_sp.
 * 
 * Pointer to the current context will stored in @old_sp.
 */
void pok_context_switch (uint32_t* old_sp, uint32_t new_sp);


/**
 * DEV: It should be simple uint32_t eventually.
 * 
 * Some arch-dependent jump-like operations are implemented using
 * context switch, which requires 2 stacks. So, we prepare additional,
 * private stack for these operations.
 */
struct dStack
{
    uint32_t stacks[2];
    int index;
};

// => pok_stack_alloc
static inline void pok_dstack_alloc(struct dStack* d, uint32_t stack_size)
{
    d->stacks[0] = pok_stack_alloc(stack_size);
    d->stacks[1] = pok_stack_alloc(stack_size);
    d->index = 0;
}

/* Extract "normal" stack. */
static inline uint32_t pok_dstack_get_stack(struct dStack* d)
{
    return d->stacks[d->index];
}

/**
 * Jump to context, stored in @new_sp.
 * 
 * Current context will be lost.
 */
static inline void pok_context_jump(uint32_t new_sp)
{
    uint32_t fake_sp;
    pok_context_switch(&fake_sp, new_sp);
}

/**
 * Jump to given entry with given stack.
 * 
 * Mainly used for restart current context.
 */
static inline void pok_context_restart(struct dStack* d,
        void (*entry)(void))
{
    int index = d->index;
    int index_other = index ^ 1;
    d->index = index_other;
    pok_context_init(d->stacks[index_other], entry);
    pok_context_jump(d->stacks[index_other]);
}

// Unused
void			pok_context_reset(uint32_t stack_size,
					  uint32_t stack_addr);

/**
 * Create TLB descriptor, which maps physical addresses in range
 * 
 * [addr;addr+size)
 * 
 * into user space.
 * 
 * Descriptor will be accessible via its identificator (@space_id).
 */
pok_ret_t   pok_create_space (uint8_t space_id, uintptr_t addr, size_t size);

/**
 * Return base virtual address for space mapping.
 * 
 * @addr is currently unused.
 * 
 */
uintptr_t	   pok_space_base_vaddr (uintptr_t addr);

/**
 * Initialize context which can be used with pok_context_switch()
 * 
 * for jump into user space.
 */
uint32_t pok_space_context_init(
        uint32_t sp,
        uint8_t space_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2);

/*
 * Basically the same as above(pok_space_context_create), but don't allocate new stack,
 * and reuse existing one.
 *
 * sp should be the value returned by 
 * pok_space_context_create earlier.
 * 
 * TODO: pok_space_context_init() should be used instead.
 */
void pok_space_context_restart(
        uint32_t sp,
        uint8_t space_id,
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2);

/**
 * Switch to given (user) space.
 */
pok_ret_t   pok_space_switch (uint8_t new_space_id);

/**
 * Jump to the user space.
 * 
 * Current kernel stack will be used in interrupts/syscalls.
 */
static inline void pok_context_user_jump (
        struct dStack* d,
        uint8_t space_id, /* Actually, unused (should already be set with pok_space_switch). */
        uint32_t entry_rel,
        uint32_t stack_rel,
        uint32_t arg1,
        uint32_t arg2)
{
        int index_other = d->index ^ 1;
        d->index = index_other;
        
        uint32_t sp = pok_space_context_init(d->stacks[index_other],
                space_id,
                entry_rel,
                stack_rel,
                arg1,
                arg2);
                
        pok_context_jump(sp);
}

/**
 * Returns the stack address for the thread in a partition.
 *
 * @arg space_id indicates space for the partition that contains
 * the thread.
 * 
 * @arg stack_size indicates size of requested stack.
 *
 * @arg state should either
 * 
 *    - points to 0, which denotes first allocation request
 *                (all previous allocations are invalidated)
 *    - be value, passed to previous call to the function.
 *
 * On success function returns head to the stack and update value
 * pointed by @state.
 * 
 * On fail (e.g., insufficient space for requested stack) function
 * returns 0 and leave value pointed by @state unchanged.
 */
uint32_t    pok_thread_stack_addr   (uint8_t    space_id,
                                     uint32_t stack_size,
                                     uint32_t* state);

/*
 * Load given elf into given user space to given ARINC partition.
 * 
 * After the call @entry will be filled with address of start function.
 */
struct _pok_patition_arinc;
void pok_arch_load_partition(struct _pok_patition_arinc* part,
        uint8_t elf_id,
        uint8_t space_id,
        uintptr_t *entry);

//#ifdef POK_ARCH_PPC
#ifdef __PPC__
#include <arch/ppc/spinlock.h>
#endif

//#ifdef POK_ARCH_X86
#ifdef __i386__
#include <arch/x86/spinlock.h>
#endif

//#ifdef POK_ARCH_SPARC
#ifdef __sparc__
#include <arch/sparc/spinlock.h>
#endif

#endif /* !__POK_ARCH_H__ */
