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
    registers[cp0_status] = ea->STATUS;
    registers[cp0_lo] = ea->lo;
    registers[cp0_hi] = ea->hi;
    registers[cp0_badvaddr] = ea->BadVAddr;
    registers[cp0_cause] = ea->CAUSE;
    registers[pc] = ea->EPC;
    registers[cp1_fcsr] = ea->FCSR;
    registers[cp1_fir] = ea->FIR;
}

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct jet_interrupt_context* ea, const uint32_t* registers)
{
    //TODO
    (void) ea;
    (void) registers;
}


static const char hexchars[]="0123456789abcdef";

char instr[8] = "00000000";
int addr_instr = 0;
char instr2[8] = "00000000";
int addr_instr2 = 0;
char trap[8] = "0000000d";
#define REG_EPC                 37
#define REG_FP                  72
#define REG_SP                  29


pok_bool_t ja_gdb_single_step(struct jet_interrupt_context* ea, uint32_t* registers)
{
    (void) ea;
    (void) registers;
    //~ uint32_t inst = *((uint32_t *)registers[pc]);
    //~ uint32_t c_inst = registers[pc];
/*
 *  If it's a 'b' instruction
 */
/*
 *  If it's a 'bal' instruction
 */
/*
 *  If it's a 'beq' instruction
 */
/*
 *  If it's a 'beql' instruction
 */
/*
 *  If it's a 'bgez' instruction
 */
/*
 *  If it's a 'BGEZAL' instruction
 */
/*
 *  If it's a 'BGEZALL' instruction
 */
/*
 *  If it's a 'BGEZL' instruction
 */
/*
 *  If it's a 'BGTZ' instruction
 */
/*
 *  If it's a 'BGTZL' instruction
 */
/*
 *  If it's a 'BLEZ' instruction
 */

/*
 *  If it's a 'J' instruction
 */

/*
 *  If it's a 'JAL' instruction
 */
/*
 *  If it's a 'JR' instruction
 */

  
    return TRUE; 
}

char * ja_gdb_write_regs(char * ptr, const uint32_t* registers, const uint32_t* fp_registers)
{
    hex2mem(ptr, (char *)registers, 32*4);
    hex2mem(ptr, (char *)&registers[cp0_status], 6*4);/* cp0: status, lo, hi, badvaddr, cause, epc */
    hex2mem(ptr, (char *)fp_registers, 32*8);
    hex2mem(ptr, (char *)&registers[cp1_fcsr], 2*4); /* cp1: fcsr, fir*/
    //~ hex2mem(ptr, (char *)&registers[r30], 2*4);         /* framepointer and dummy (unused) */
    //~ hex2mem(ptr, (char *)&registers[cp0_index], 16*4); /* cp0: index, random, entrylo0, entrylo1, context, pagemask, wired, reg7,
                                                                      //~ reg8, reg9, entryhi, reg11, reg12, reg13, reg14, prid*/
    return ptr;
}


char * ja_gdb_read_regs(char * ptr, const uint32_t* registers, const uint32_t* fp_registers)
{
    ptr = mem2hex((char *)registers, ptr, 32*4); /* r0...r31 */
    ptr = mem2hex((char *)&registers[cp0_status], ptr, 6*4); /* cp0: status, lo, hi, badvaddr, cause, epc */
    ptr = mem2hex((char *)fp_registers, ptr, 32*4); /* f0...31 */
    ptr = mem2hex((char *)&registers[cp1_fcsr], ptr, 2*4); /* cp1: fcsr, fir*/
    //~ ptr = mem2hex((char *)&registers[r30], ptr, 2*4); /* framepointer and dummy (unused) */
    //~ ptr = mem2hex((char *)&registers[cp0_index], ptr, 16*4); /* cp0: index, random, entrylo0, entrylo1, context, pagemask, wired, reg7,
    return ptr;
}

void ja_gdb_decrease_pc(struct jet_interrupt_context* ea)
{
    ea->EPC = ea->EPC - 4;
}


char * ja_gdb_send_first_package(char * ptr, int sigval, const uint32_t* registers)
{
	/*
	 * Send trap type (converted to signal)
	 */
	*ptr++ = 'T';
	*ptr++ = hexchars[sigval >> 4];
	*ptr++ = hexchars[sigval & 0xf];

	/*
	 * Send Error PC
	 */
	*ptr++ = hexchars[REG_EPC >> 4];
	*ptr++ = hexchars[REG_EPC & 0xf];
	*ptr++ = ':';
	ptr = mem2hex((char *)&registers[pc], ptr, 4);
	*ptr++ = ';';

	/*
	 * Send frame pointer
	 */
	*ptr++ = hexchars[REG_FP >> 4];
	*ptr++ = hexchars[REG_FP & 0xf];
	*ptr++ = ':';
	ptr = mem2hex((char *)&registers[r30], ptr, 4);
	*ptr++ = ';';

	/*
	 * Send stack pointer
	 */
	*ptr++ = hexchars[REG_SP >> 4];
	*ptr++ = hexchars[REG_SP & 0xf];
	*ptr++ = ':';
	ptr = mem2hex((char *)&registers[r29], ptr, 4);
    *ptr++ = ';';
    return ptr;
}
void ja_gdb_restore_instr(struct jet_interrupt_context* ea)
{
    (void) ea;
    if (addr_instr != 0){
        hex2mem(instr, (char *) (addr_instr), 4);
        addr_instr = 0;
        if (addr_instr2 != 0){
            hex2mem(instr2, (char *) (addr_instr2), 4);
            addr_instr2 = 0;
        }
    }
    
}

void ja_gdb_add_watchpoint(uintptr_t addr, int length, int type, struct jet_interrupt_context* ea, char * remcomOutBuffer)
{
    /* 
     * Didn't realized in x86
     */
    (void) addr;
    (void) length;
    (void) type;
    (void) ea;
    (void) remcomOutBuffer;
    gdb_strcpy (remcomOutBuffer, "E22");
}


void ja_gdb_remove_watchpoint(uintptr_t addr, int length, int type, struct jet_interrupt_context* ea, char * remcomOutBuffer)
{
    /* 
     * Didn't realized in x86
     */
    (void) addr;
    (void) length;
    (void) type;
    (void) ea;
    (void) remcomOutBuffer;
    gdb_strcpy (remcomOutBuffer, "E22");
}

char * ja_gdb_alloc_trap(){
    return trap;
}

void ja_gdb_instr_sync(char* addr)
{
    /* 
     * Didn't realized in x86
     */
    (void) addr;
}



#endif /* POK_NEEDS_GDB */
