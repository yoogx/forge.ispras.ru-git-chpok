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

#ifdef POK_NEEDS_GDB

#include <gdb.h>
#include <asp/gdb.h>
/* Fill 'registers' array according to 'ea'. */
void gdb_set_regs(const struct jet_interrupt_context* ea, uint32_t* registers)
{
    registers[EAX] = ea->eax;
    registers[ECX] = ea->ecx;
    registers[EDX] = ea->edx;
    registers[EBX] = ea->ebx;
    registers[ESP] = ea->__esp;
    registers[EBP] = ea->ebp;
    registers[ESI] = ea->esi;
    registers[EDI] = ea->edi;
    registers[pc] = ea->eip;
    registers[PS] = ea->eflags;
    registers[CS] = ea->cs;
    registers[SS] = ea->ss;
    registers[DS] = ea->ds;
    registers[ES] = ea->es;
    registers[FS] = -1;
    registers[GS] = -1;
}

/* Fill 'ea' array according to 'registers'. */
void gdb_get_regs(struct jet_interrupt_context* ea, const uint32_t* registers)
{
    (void) ea;
    (void) registers;
    //TODO
}

static const char hexchars[]="0123456789abcdef";
char trap[2] = "CC";

pok_bool_t ja_gdb_single_step(struct jet_interrupt_context* ea, uint32_t* registers)
{
  /* set the trace bit if we're stepping */
    registers[PS] |= 0x100;
    ea->eflags = registers[PS];   
    return FALSE; 
}

char * ja_gdb_write_regs(char * ptr, const uint32_t* registers)
{
    hex2mem (ptr, (char *) registers, NUMREGBYTES);
    return ptr;
}


char * ja_gdb_read_regs(char * ptr, const uint32_t* registers)
{
    mem2hex ((char *) registers, ptr, NUMREGBYTES);
    return ptr;
}

void ja_gdb_decrease_pc(struct jet_interrupt_context* ea)
{
    ea->eip--;
}


char * ja_gdb_send_first_package(char * ptr, int sigval, const uint32_t* registers)
{
    *ptr++ = 'T';         /* notify gdb with signo, PC, FP and SP */
    *ptr++ = hexchars[sigval >> 4];
    *ptr++ = hexchars[sigval & 0xf];

    *ptr++ = hexchars[ESP]; 
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[ESP], ptr, 4);    /* SP */
    *ptr++ = ';';

    *ptr++ = hexchars[EBP]; 
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[EBP], ptr, 4);    /* FP */
    *ptr++ = ';';

    *ptr++ = hexchars[pc]; 
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[pc], ptr, 4);     /* PC */
    *ptr++ = ';';
    return ptr;
}
void ja_gdb_restore_instr(struct jet_interrupt_context* ea)
{
    /* 
     * Nothing .....
     * We don't need it in x86, because we use special debug register for single step
     */
    (void) ea;
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
