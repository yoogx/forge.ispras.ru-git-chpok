/*
 * Institute for System Programming of the Russian Academy of Sciences
 * Copyright (C) 2016 ISPRAS
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, Version 3.
 *
 * This program is distributed in the hope # that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License version 3 for more details.
 */

#include <config.h>

#if POK_NEEDS_GDB

#include <gdb.h>


/* Fill 'registers' array according to 'ea'. */
void gdb_set_regs(const struct jet_interrupt_context* ea, uint32_t* registers)
{
    if(ea == NULL)
    {
        memset(registers, 0, NUMREGS * sizeof(uint32_t));
        return;
    }

    registers[r0] = ea->r0;
    registers[r1] = ea->r1;
    registers[r2] = ea->r2;
    registers[r3] = ea->r3;
    registers[r4] = ea->r4;
    registers[r5] = ea->r5;
    registers[r6] = ea->r6;
    registers[r7] = ea->r7;
    registers[r8] = ea->r8;
    registers[r9] = ea->r9;
    registers[r10] = ea->r10;
    registers[r11] = ea->r11;
    registers[r12] = ea->r12;
    registers[r13] = ea->r13;
    registers[r14] = ea->r14;
    registers[r15] = ea->r15;
    registers[r16] = ea->r16;
    registers[r17] = ea->r17;
    registers[r18] = ea->r18;
    registers[r19] = ea->r19;
    registers[r20] = ea->r20;
    registers[r21] = ea->r21;
    registers[r22] = ea->r22;
    registers[r23] = ea->r23;
    registers[r24] = ea->r24;
    registers[r25] = ea->r25;
    registers[r26] = ea->r26;
    registers[r27] = ea->r27;
    registers[r28] = ea->r28;
    registers[r29] = ea->r29;
    registers[r30] = ea->r30;
    registers[r31] = ea->r31;
    registers[ctr] = ea->ctr;
    registers[xer] = ea->xer;
    registers[pc] = ea->srr0;
    registers[msr] = ea->srr1;
    registers[lr] = ea->lr;
    registers[cr] = ea->cr;
}

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct jet_interrupt_context* ea, const uint32_t* registers)
{
    (void) ea;
    (void) registers;
    //TODO
}

static const char hexchars[]="0123456789abcdef";

/*
 * Saving instruction for single step
 */
char instr[8] = "00000000";
int addr_instr = 0;
char instr2[8] = "00000000";
int addr_instr2 = 0;
char trap[8] = "7fe00008";

#define SPRN_DBCR0      0x134   /* Debug Control Register 0 */
#define SPRN_DBCR2       0x136   /* Debug Control Register 2 */
#define SPRN_DAC1       0x13C   /* Data Address Compare 1 */
#define SPRN_DAC2       0x13D   /* Data Address Compare 2 */
#define SPRN_DBSR       0x130   /* Debug Status Register */
#define SPRN_DBSRWR       0x132   /* Debug Status Register Write Register*/

#define SPRN_PID        0x030   /* Process ID */

#define __stringify_1(x)        #x
#define __stringify(x)          __stringify_1(x)

#define mfspr(rn)       ({unsigned long rval; \
                                asm volatile("mfspr %0," __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
#define mtspr(rn, v)    asm volatile("mtspr " __stringify(rn) ",%0" : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")
#define MSR_EE      1<<(15)              /* External Interrupt Enable */
#define MSR_SE_LG   10      /* Single Step */
#define __MASK(X)   (1<<(X))
#define MSR_SE      __MASK(MSR_SE_LG)   /* Single Step */
pok_bool_t watchpoint_is_set = FALSE;





pok_bool_t ja_gdb_single_step(struct jet_interrupt_context* ea, uint32_t* registers)
{
    uint32_t inst = *((uint32_t *)registers[pc]);
    uint32_t c_inst = registers[pc];
    ea->srr1 &= (~((uint32_t)MSR_EE));

/*  
* if it's  unconditional branching, e.g. 'b' or 'bl'
*/
    if ((inst >> (6*4+2)) == 0x12){
        if (!(inst & 0x2)){
            inst = ((inst << 6) >> 8) << 2;
            if (inst >> 25){
                inst = inst | 0xFE000000;
            }
            inst = inst + c_inst;
            mem2hex((char *)(registers[pc]+4), instr,4);            
            hex2mem(trap, (char *)(registers[pc]+4), 4);
            addr_instr = registers[pc] + 4;
            mem2hex((char *)(inst), instr2,4);            
            hex2mem(trap, (char *)(inst), 4);
            addr_instr2 = inst;
            return TRUE;
        }else{
            inst = ((inst << 6) >> 8) << 2;
            mem2hex((char *)(registers[pc]+4), instr,4);            
            hex2mem(trap, (char *)(registers[pc]+4), 4);
            addr_instr = registers[pc] + 4;
            mem2hex((char *)(inst), instr2,4);            
            hex2mem(trap, (char *)(inst), 4);
            addr_instr2 = inst;
            return TRUE;
        }
        if (inst & 0x1){
        }
    }
/*
* if it's conditional branching, e.g. 'bne' or 'beq'
*/    
    if ((inst >> (6*4+2)) == 0x10){
        if (!(inst & 0x2)){
            inst = ((inst << 16) >> 18) << 2;
            if (inst >> 15){
                inst=inst | 0xFFFF8000;
            }
            inst = inst + c_inst;
            mem2hex((char *)(registers[pc]+4), instr,4);            
            hex2mem(trap, (char *)(registers[pc]+4), 4);
            addr_instr = registers[pc] + 4;
            mem2hex((char *)(inst), instr2,4);            
            hex2mem(trap, (char *)(inst), 4);
            addr_instr2 = inst;
            return TRUE;
    
        }else{
            inst = ((inst << 16) >> 18) << 2;
            mem2hex((char *)(registers[pc]+4), instr,4);            
            hex2mem(trap, (char *)(registers[pc]+4), 4);
            addr_instr = registers[pc] + 4;
            mem2hex((char *)(inst), instr2,4);            
            hex2mem(trap, (char *)(inst), 4);
            addr_instr2 = inst;
            return TRUE;
        }
        if (inst & 0x1){
        }
    }
/*
*  If it's a 'brl' instruction
*/            
    if ((inst >> (6*4+2)) == 0x13){
        mem2hex((char *)(registers[lr]), instr,4);            
        hex2mem(trap, (char *)(registers[lr]), 4);
        addr_instr = registers[lr];                
        mem2hex((char *)(registers[pc]+4), instr2,4);            
        hex2mem(trap, (char *)(registers[pc]+4), 4);
        addr_instr2 = registers[pc] + 4;
        return TRUE;
    }
    mem2hex((char *)(registers[pc]+4), instr,4);            
    hex2mem(trap, (char *)(registers[pc]+4), 4);
    addr_instr = registers[pc] + 4;
    return TRUE;    

}

char * ja_gdb_write_regs(char * ptr, const uint32_t* registers, const uint32_t* fp_registers)
{
  

    /* General Purpose registers */
    hex2mem(ptr, (char *)registers, 32 * 4);

    /* Floating Point registers - FIXME?? */
    ptr = hex2mem(ptr, (char *)fp_registers, 32 * 8);
    ////ptr += 32*8*2;
    /* pc, msr, cr, lr, ctr, xer, (mq is unused) */
    ptr = hex2mem(ptr, (char *)&registers[pc]/*nip*/, 4);
    ptr = hex2mem(ptr, (char *)&registers[msr], 4);
    ptr = hex2mem(ptr, (char *)&registers[cr]/*[ccr]*/, 4);
    ptr = hex2mem(ptr, (char *)&registers[lr]/*[link]*/, 4);
    ptr = hex2mem(ptr, (char *)&registers[ctr], 4);
    ptr = hex2mem(ptr, (char *)&registers[xer], 4);    
    
    return ptr;
}


char * ja_gdb_read_regs(char * ptr, const uint32_t* registers, const uint32_t* fp_registers)
{
    /* General Purpose Regs */
    ptr = mem2hex((char *)registers, ptr, 32 * 4);
    /* Floating Point registers - FIXME */
    ptr = mem2hex((char *)fp_registers, ptr, 32 * 8);
    //~ ptr += 32*8*2;
    /* pc, msr, cr, lr, ctr, xer, (mq is unused) */
    ptr = mem2hex((char *)&registers[pc], ptr, 4);
    ptr = mem2hex((char *)&registers[msr], ptr, 4);
    ptr = mem2hex((char *)&registers[cr]/*[ccr]*/, ptr, 4);
    ptr = mem2hex((char *)&registers[lr]/*[link]*/, ptr, 4);
    ptr = mem2hex((char *)&registers[ctr], ptr, 4);
    ptr = mem2hex((char *)&registers[xer], ptr, 4);    
    
    return ptr;
}

void ja_gdb_decrease_pc(struct jet_interrupt_context* ea)
{
    ea->srr0 = ea->srr0 - 4;
    
}


char * ja_gdb_send_first_package(char * ptr, int sigval, const uint32_t* registers)
{
    *ptr++ = 'T';
    *ptr++ = hexchars[sigval >> 4];
    *ptr++ = hexchars[sigval & 0xf];
    *ptr++ = hexchars[64 >> 4];
    *ptr++ = hexchars[64 & 0xf];
    *ptr++ = ':';
    ptr = mem2hex((char *)(&registers[pc]), ptr, 4);
    *ptr++ = ';';
    *ptr++ = hexchars[1 >> 4];
    *ptr++ = hexchars[1 & 0xf];
    *ptr++ = ':';
    ptr = mem2hex((char *)(&registers) + 1*4, ptr, 4);
    *ptr++ = ';';    
    return ptr;
}

void ja_gdb_restore_instr(struct jet_interrupt_context* ea)
{
    if (addr_instr != 0){
        ea->srr1 |= MSR_EE;
        hex2mem(instr, (char *) (addr_instr), 4);
        addr_instr = 0;
        if (addr_instr2 != 0){
            hex2mem(instr2, (char *) (addr_instr2), 4);
            addr_instr2 = 0;
        }
    }
}

void ja_gdb_add_watchpoint(uintptr_t addr, int length, int type,struct jet_interrupt_context* ea, char * remcomOutBuffer)
{
#ifdef QEMU
    (void) addr;
    (void) length;
    (void) type;
    (void) ea;
    /*do nothing*/
    gdb_strcpy (remcomOutBuffer, "E22");
    return;
#else
    /*If 1 watchpoint was already set*/
    if ((watchpoint_is_set) || (type > 4) || (type < 2)){
        gdb_strcpy (remcomOutBuffer, "E22");
        return;
    }

    uintptr_t gdb_addr = gdb_thread_write_addr(t, addr, 4);

    if (!gdb_addr){
        gdb_strcpy (remcomOutBuffer, "E03");
        return;
    }

    int old_pid = pok_space_get_current();
    int new_pid = gdb_thread_get_space(t);
    pok_space_switch(new_pid);

    mtspr(SPRN_DAC1, addr); // Use original address, as it is not accessed immediately.
    mtspr(SPRN_DAC2, addr + length);
    uint32_t DBCR2 = mfspr(SPRN_DBCR2);
    DBCR2 |= 0x800000UL;
    mtspr(SPRN_DBCR2, DBCR2);
    int DAC;
    if (type == 2) DAC = 0x40050000UL;
    if (type == 3) DAC = 0x400A0000UL;
    if (type == 4) DAC = 0x400F0000UL;
    uint32_t DBCR0 = mfspr(SPRN_DBCR0);

    printf_GDB("\nBefore DBCR0 = 0x%lx\n", DBCR0);
    DBCR0 |= DAC;
    printf_GDB("After DBCR0 = 0x%lx\n", DBCR0);
    mtspr(SPRN_DBCR0, DBCR0);
    ea->srr1 |= 0x200;
    watchpoint_is_set = TRUE;
    gdb_strcpy(remcomOutBuffer, "OK");   
#endif 

}


void ja_gdb_remove_watchpoint(uintptr_t addr, int length, int type, struct jet_interrupt_context* ea, char * remcomOutBuffer)
{
#ifdef QEMU
    (void) addr;
    (void) length;
    (void) type;
    (void) ea;
    /*do nothing*/
    gdb_strcpy (remcomOutBuffer, "E22");
    return;
#else
    (void) addr;
    (void) length;
    if ((!watchpoint_is_set) || (type > 4) || (type < 2)){
        
        gdb_strcpy (remcomOutBuffer, "E22");
        return;
    }

    gdb_strcpy(remcomOutBuffer, "OK");
    watchpoint_is_set = FALSE;
    printf_GDB("\nWatchpoint_is_set = %d \n",watchpoint_is_set);
    uint32_t DBCR0 = mfspr(SPRN_DBCR0);
    printf_GDB("Before set MSR DBCR0 = 0x%lx\n", DBCR0);
    ea->srr1 &= (~0x200);
    DBCR0 = mfspr(SPRN_DBCR0);    
    printf_GDB("Before DBCR0 = 0x%lx\n", DBCR0);
    DBCR0 &= (~0x400F0000UL);
    printf_GDB("After DBCR0 = 0x%lx\n", DBCR0);
    mtspr(SPRN_DBCR0, DBCR0);
    uint32_t DBSR = mfspr(SPRN_DBSR);
    printf_GDB("Before DBSR = 0x%lx\n", DBSR);
    DBSR &= (~0xF0000);
    mtspr(SPRN_DBSRWR, DBSR);
    DBSR = mfspr(SPRN_DBSR);
    printf_GDB("After DBSR = 0x%lx\n", DBSR);
    uint32_t DBCR2 = mfspr(SPRN_DBCR2);
    DBCR2 &= (~0x800000);
    mtspr(SPRN_DBCR2, DBCR2);    

#endif
}


char * ja_gdb_alloc_trap(){
    return trap;
}

void ja_gdb_instr_sync(char* addr)
{
    asm volatile("dcbst 0, %0; sync; icbi 0,%0; sync; isync" : : "r" (addr));
}

#endif /* POK_NEEDS_GDB */
