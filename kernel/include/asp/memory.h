#ifndef __POK_MEM_H__
#define __POK_MEM_H__

/* TODO: Whole header should be removed when functionality becomes not needed. */

/* 
 * Return physical address for given user space address.
 * 
 * Return 0 if memory cannot be accessed by the user.
 */
uintptr_t pok_virt_to_phys(uintptr_t virt);
/* 
 * Return user space address for given physical address.
 * 
 * Return 0 if memory cannot be accessed by the user.
 */
uintptr_t pok_phys_to_virt(uintptr_t phys);

#endif
