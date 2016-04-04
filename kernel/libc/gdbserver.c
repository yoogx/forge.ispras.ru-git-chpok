#define PC_REGNUM 64
#define SP_REGNUM 1
 
//~ #define DEBUG_GDB
#define QEMU
#define MULTIPROCESS

/*
 * For e500mc: ~/qemu/ppc-softmmu/qemu-system-ppc -serial stdio -serial pty -M ppce500 -cpu e500mc -kernel pok.elf
 *
 * For i386:  qemu-system-i386 -serial stdio -serial pty -kernel pok.elf
 *
 */



/****************************************************************************

        THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or it's performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for 386 by Jim Kingdon, Cygnus Support.
 *
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps() is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint().  Breakpoint()
 *  simulates a breakpoint by executing a trap #1.
 *
 *  The external function exceptionHandler() is
 *  used to attach a specific handler to a specific 386 vector number.
 *  It should use the same privilege level it runs at.  It should
 *  install it as an interrupt gate so that interrupts are masked
 *  while the handler runs.
 *
 *
 ****************************************************************************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *    .... and others
 *  
 *  All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#include <libc.h>
#include <bsp.h>
#include <core/thread.h>
#include <core/partition.h>

#include <core/time.h>
#include <core/debug.h>
#include <config.h>

 
#ifdef __i386__
#include <arch.h>
#endif

/************************************************************************
 *
 * external low-level support routines
 */

char        *strcpy(char *dest, const char *str)
{
    unsigned int i;
    for (i = 0; str[i];i++)
        dest[i] = str[i];
    dest[i] = '\0';
    return dest;
}

char string[1000];
int st_idx = 0;

extern void putDebugChar( char );   /* write a single character      */
extern int getDebugChar();  /* read and return a single char */
extern void exceptionHandler(); /* assign an exception handler   */

void putDebugChar(char c){
    data_to_read_1();
    pok_cons_write_1(&c,1);
#ifdef DEBUG_GDB
    pok_cons_write(&c,1);
#endif
}

int getDebugChar(){
    data_to_read_1();
    int inf = getchar2();
#ifdef DEBUG_GDB
    string[st_idx++] = inf;
    //~ printf("%c",inf);
#endif
    return inf;
}

/************************************************************************/
/* BUFMAX defines the maximum number of characters in inbound/outbound buffers*/
/* at least NUMREGBYTES*2 are needed for register packets */
#define BUFMAX 1000

static const char hexchars[]="0123456789abcdef";

/* Number of registers.  */
#ifdef __PPC__
#define NUMREGS 38
enum fp_regnames {
f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13,
f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29,
f30, f31    
};
uint32_t fp_registers[32];
#endif
#ifdef __i386__
#define NUMREGS 16
#endif
/* Number of bytes of registers.  */
#define NUMREGBYTES (NUMREGS * 4)

enum regnames {
#ifdef __PPC__
r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13,
r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29,
r30, r31, pc, msr, cr, lr, ctr, xer 
#endif
#ifdef __i386__
EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI,
           PC /* also known as eip */,
           PS /* also known as eflags */,
           CS, SS, DS, ES, FS, GS
#endif
};




/*
 * these should not be static cuz they can be used outside this module
 */
uint32_t registers[NUMREGS];

int
hex (ch)
     char ch;
{
    if ((ch >= 'a') && (ch <= 'f'))
        return (ch - 'a' + 10);
    if ((ch >= '0') && (ch <= '9'))
        return (ch - '0');
    if ((ch >= 'A') && (ch <= 'F'))
        return (ch - 'A' + 10);
    return (-1);
}

static char remcomInBuffer[BUFMAX];
static char remcomOutBuffer[BUFMAX];

#ifdef __PPC__
/* scan for the sequence $<data>#<checksum>     */
static void
getpacket(char *buffer)
{
    data_to_read_1();
#ifdef DEBUG_GDB
    printf("Lets getpacket <---\n");
#endif
    unsigned char checksum;
    unsigned char xmitcsum;
    int i;
    int count;
    unsigned char ch;

    do {
        /* wait around for the start character, ignore all other
         * characters */
        while ((ch = (getDebugChar() & 0x7f)) != '$') ;
        
        checksum = 0;
        xmitcsum = -1;

        count = 0;

        /* now, read until a # or end of buffer is found */
        while (count < BUFMAX) {
            ch = getDebugChar() & 0x7f;
            if (ch == '#')
                break;
            checksum = checksum + ch;
            buffer[count] = ch;
            count = count + 1;
        }

        if (count >= BUFMAX)
            continue;

        buffer[count] = 0;

        if (ch == '#') {
            xmitcsum = hex(getDebugChar() & 0x7f) << 4;
            xmitcsum |= hex(getDebugChar() & 0x7f);
            if (checksum != xmitcsum){
                putDebugChar('-');  /* failed checksum */
            }else {;
                putDebugChar('+'); /* successful transfer */
                /* if a sequence char is present, reply the ID */
                if (buffer[2] == ':') {
                    putDebugChar(buffer[0]);
                    putDebugChar(buffer[1]);
                    /* remove sequence chars from buffer */
                    count = strlen(buffer);
                    for (i=3; i <= count; i++)
                        buffer[i-3] = buffer[i];
                }
            }
        }
    } while (checksum != xmitcsum);
#ifdef DEBUG_GDB
    printf("\n");
#endif
}
#endif
#ifdef __i386__
unsigned char *
getpacket (void)
{
#ifdef DEBUG_GDB
    printf("Lets getpacket <---\n");
#endif
    unsigned char *buffer = (unsigned char *) (&remcomInBuffer[0]);
    unsigned char checksum;
    unsigned char xmitcsum;
    int count;
    char ch;

    while (1)
    {
      /* wait around for the start character, ignore all other characters */
        while ((ch = getDebugChar ()) != '$')
        ;
        retry:
        checksum = 0;
        xmitcsum = -1;
        count = 0;

      /* now, read until a # or end of buffer is found */
        while (count < BUFMAX - 1)
        {
            ch = getDebugChar ();
            if (ch == '$')
                goto retry;
            if (ch == '#')
                break;
            checksum = checksum + ch;
            buffer[count] = ch;
            count = count + 1;
        }
        buffer[count] = 0;

        if (ch == '#')
        {
            ch = getDebugChar ();
            xmitcsum = hex (ch) << 4;
            ch = getDebugChar ();
            xmitcsum += hex (ch);

            if (checksum != xmitcsum)
            {
                putDebugChar ('-');   /* failed checksum */
            }
            else
            {
          
            }
      
            {
                putDebugChar ('+');   /* successful transfer */

                /* if a sequence char is present, reply the sequence ID */
                if (buffer[2] == ':')
                {
                    putDebugChar (buffer[0]);
                    putDebugChar (buffer[1]);

                    return &buffer[3];
                }

            return &buffer[0];
            }
        }
    }
#ifdef DEBUG_GDB
    printf("\n");
#endif
}
#endif


/* send the packet in buffer.  */

void
putpacket (unsigned char *buffer)
{
    data_to_read_1();
    unsigned char checksum;
    int count;
    char ch;
#ifdef DEBUG_GDB
    string[st_idx] = '\0';
    printf("Buffered string:\n  %s\n", string);
    st_idx = 0;
    printf("\nLets putpacket --->\n");
#endif
  /*  $<packet info>#<checksum>. */
    do
    {
        putDebugChar ('$');
        checksum = 0;
        count = 0;
        ch = buffer[count];
        while (ch != '\0')
        {
      
            putDebugChar (ch);
            checksum += ch;
            count += 1;
            ch = buffer[count];
        }

        putDebugChar ('#');
        putDebugChar (hexchars[checksum >> 4]);
        putDebugChar (hexchars[checksum % 16]);
    }
    while (getDebugChar () != '+');
#ifdef DEBUG_GDB
        printf("\n");
#endif
}


/* Address of a routine to RTE to if we get a memory fault.  */
static void (*volatile mem_fault_routine) () = NULL;

/* Indicate to caller of mem2hex or hex2mem that there has been an
   error.  */
static volatile int mem_err = 0;

void
set_mem_err (void)
{
    mem_err = 1;
}

/* These are separate functions so that they are so short and sweet
   that the compiler won't save any registers (if there is a fault
   to mem_fault, they won't get restored, so there better not be any
   saved).  */
int
get_char (char *addr)
{
    return *addr;
}

void
set_char (char *addr, int val)
{
    *addr = val;
asm volatile("dcbst 0, %0; sync; icbi 0,%0; sync; isync" : : "r" (addr));
}

/* convert the memory pointed to by mem into hex, placing result in buf */
/* return a pointer to the last char put in buf (null) */
/* If MAY_FAULT is non-zero, then we should set mem_err in response to
   a fault; if zero treat a fault like any other fault in the stub.  */
char *
mem2hex (mem, buf, count, may_fault)
     char *mem;
     char *buf;
     int count;
     int may_fault;
{
    int i;
    unsigned char ch;

    if (may_fault)
        mem_fault_routine = set_mem_err;
    for (i = 0; i < count; i++)
    {
        ch = get_char (mem++);
        if (may_fault && mem_err)
            return (buf);
        *buf++ = hexchars[ch >> 4];
        *buf++ = hexchars[ch % 16];
    }
    *buf = 0;
    if (may_fault)
        mem_fault_routine = NULL;
    return (buf);
}

/* convert the hex array pointed to by buf into binary to be placed in mem */
/* return a pointer to the character AFTER the last byte written */
char *
hex2mem (buf, mem, count, may_fault)
     char *buf;
     char *mem;
     int count;
     int may_fault;
{
    int i;
    unsigned char ch;

    if (may_fault)
        mem_fault_routine = set_mem_err;
    for (i = 0; i < count; i++)
    {
        ch = hex (*buf++) << 4;
        ch = ch + hex (*buf++);
        set_char (mem++, ch);
        if (may_fault && mem_err)
            return (mem);
    }
    if (may_fault)
        mem_fault_routine = NULL;
    return (mem);
}

/* this function takes the 386 exception vector and attempts to
   translate this number into a unix compatible signal value */
int
computeSignal (int exceptionVector)
{
    int sigval;
    switch (exceptionVector)
    {
        case 0:
            sigval = 8;
            break;            /* divide by zero */
        case 1:
            sigval = 5;
            break;            /* debug exception (watchpoint)*/
        case 3:
            sigval = 5;
            break;            /* breakpoint */
        case 4:
            sigval = 16;
            break;            /* into instruction (overflow) */
        case 5:
            sigval = 16;
            break;            /* bound instruction */
        case 6:
            sigval = 4;
            break;            /* Invalid opcode */
        case 7:
            sigval = 8;
            break;            /* coprocessor not available */
        case 8:
            sigval = 7;
            break;            /* double fault */
        case 9:
            sigval = 11;
            break;            /* coprocessor segment overrun */
        case 10:
            sigval = 11;
            break;            /* Invalid TSS */
        case 11:
            sigval = 11;
            break;            /* Segment not present */
        case 12:
            sigval = 11;
            break;            /* stack exception */
        case 13:
            sigval = 11;
            break;            /* general protection */
        case 14:
            sigval = 11;
            break;            /* page fault */
        case 16:
            sigval = 7;
            break;            /* coprocessor error */
        case 17:            /* SIGINT  */
            sigval = 2;
            break;
        default:
            sigval = 7;       /* "software generated" */
    }
    return (sigval);
}

/**********************************************/
/* WHILE WE FIND NICE HEX CHARS, BUILD AN INT */
/* RETURN NUMBER OF CHARS PROCESSED           */
/**********************************************/
int
hexToInt (char **ptr, uintptr_t *intValue)
{
    int numChars = 0;
    int hexValue;

    *intValue = 0;

    while (**ptr)
    {
        hexValue = hex (**ptr);
        if (hexValue >= 0)
        {
            *intValue = (*intValue << 4) | hexValue;
            numChars++;
        }
        else
            break;

        (*ptr)++;
    }

    return (numChars);
}

/*
 * Saving instruction for single step
 */
#ifdef __PPC__
char instr[8] = "00000000";
int addr_instr = 0;
char instr2[8] = "00000000";
int addr_instr2 = 0;
char trap[8] = "7fe00008";
#endif
#ifdef __i386__
char instr[2] = "00";
int addr_instr = 0;
char trap[2] = "CC";

#endif



#define MSR_SE_LG   10      /* Single Step */
#define __MASK(X)   (1<<(X))
#define MSR_SE      __MASK(MSR_SE_LG)   /* Single Step */

static inline void set_msr(int msr)
{
    asm volatile("mtmsr %0" : : "r" (msr));
}

#ifdef __PPC__
#define SPRN_PID        0x030   /* Process ID */

#define __stringify_1(x)        #x
#define __stringify(x)          __stringify_1(x)

#define mfspr(rn)       ({unsigned long rval; \
                                asm volatile("mfspr %0," __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
#define mtspr(rn, v)    asm volatile("mtspr " __stringify(rn) ",%0" : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")
#endif


#ifdef __PPC__
#define MSR_EE      1<<(15)              /* External Interrupt Enable */
#endif

int current_part_id(){
#ifdef __PPC__
    return  mfspr(SPRN_PID);
#endif
#ifdef __i386__
    return current_segment();
#endif
}

void switch_part_id(int old_pid, int new_pid){
#ifdef __PPC__
    (void) old_pid;
    mtspr(SPRN_PID, new_pid);
#endif

#ifdef __i386__
    pok_space_switch(old_pid, new_pid);
#endif
}

static char info_thread[59] = "StoppedRunnableWaitingLockWait next activationDelayed start";


static int pok_thread_info(pok_state_t State, int * info_offset){
  switch (State){
        case POK_STATE_STOPPED: // DORMANT (must be started first)
        {
            *info_offset = 0;
            return  7;
        }
        case POK_STATE_RUNNABLE: // READY
        {
            *info_offset = 7;
            return 8;
        }
        case POK_STATE_WAITING: // WAITING (sleeping for specified time OR waiting for a lock with timeout)
        {
            *info_offset = 15;
            return 7;
        }
        case POK_STATE_LOCK:  // WAITING (waiting for a lock without timeout)
        {
            *info_offset = 22;
            return 4;
        }
        case POK_STATE_WAIT_NEXT_ACTIVATION:  // WAITING (for next activation aka "release point")
        {
            *info_offset = 26;
            return 20;
        }
        case POK_STATE_DELAYED_START:  // WAITING (waitng for partition mode NORMAL)
        {
            *info_offset = 46;
            return 13;
        }
    }
    return 0;
}


struct regs  null_ea = {
#ifdef __PPC__
    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0
#endif
#ifdef __i386__
    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0
#endif     
};


int   POK_CHECK_ADDR_IN_PARTITION(int pid,uintptr_t address){
    if (pid > 0)
        return ((POK_CHECK_VPTR_IN_PARTITION(pid - 1,address)) || (address >= 0x0 && address <  pok_partitions[0].base_addr));
    else
        return address >= 0x0 && address <  pok_partitions[0].base_addr;
}



#define max_breakpoints  20
int max_breakpoint = max_breakpoints;
int b_need_to_delete = -1;
int Connect_to_new_inferior = -1;


struct T_breakpoint breakpoints[max_breakpoints];
int Head_of_breakpoints;
int last_breakpoint;
  
pok_partition_id_t give_part_num_of_thread(int thread_num){

    for (int i = 0; i < POK_CONFIG_NB_PARTITIONS; i++){
        if (pok_partitions[i].thread_index_high + 1 > thread_num) return i + 1;
    }
    return 0;
}    

pok_bool_t watchpoint_is_set = FALSE;
/*
 *  Added watchpoint.
 *  Types:   2 -  Write watchpoint
 *           3 -   Read watchpoint
 *           4 - Access watchpoint
 */
 
 uintptr_t pok_virt_to_phys(uintptr_t virt, int pid)
{
    if (pid > 0)
        return virt - pok_partitions[pid - 1].base_vaddr + pok_partitions[pid - 1].base_addr;
    return virt;
}
uintptr_t pok_phys_to_virt(uintptr_t phys, int pid)
{
    if (pid > 0)
        return phys - pok_partitions[pid - 1].base_addr + pok_partitions[pid - 1].base_vaddr;
    return phys;
}
 

#define SPRN_DBCR0      0x134   /* Debug Control Register 0 */
#define SPRN_DBCR2       0x136   /* Debug Control Register 2 */
#define SPRN_DAC1       0x13C   /* Data Address Compare 1 */
#define SPRN_DAC2       0x13D   /* Data Address Compare 2 */
#define SPRN_DBSR       0x130   /* Debug Status Register */
#define SPRN_DBSRWR       0x132   /* Debug Status Register Write Register*/

void add_watchpoint(uintptr_t addr, int length, int *using_thread, int type){
#ifdef QEMU
    /*do nothing*/
    strcpy (remcomOutBuffer, "E22");
    return;
#else
    /*If 1 watchpoint was already set*/
    if ((watchpoint_is_set) || (type > 4) || (type < 2)){
        strcpy (remcomOutBuffer, "E22");
        return;
    }
    if (!POK_CHECK_ADDR_IN_PARTITION(give_part_num_of_thread(*using_thread + 1), addr)){
        strcpy (remcomOutBuffer, "E03");
        return;
    }
    //~ addr = pok_virt_to_phys(addr, give_part_num_of_thread(*using_thread + 1));
    
    mtspr(SPRN_DAC1, addr);
    mtspr(SPRN_DAC2, addr + length);
    uint32_t DBCR2 = mfspr(SPRN_DBCR2);
    DBCR2 |= 0x800000UL;
    mtspr(SPRN_DBCR2, DBCR2);
    int DAC;
    if (type == 2) DAC = 0x40050000UL;
    if (type == 3) DAC = 0x400A0000UL;
    if (type == 4) DAC = 0x400F0000UL;
    uint32_t DBCR0 = mfspr(SPRN_DBCR0);

#ifdef DEBUG_GDB
    printf("\nBefore DBCR0 = 0x%lx\n", DBCR0);
#endif
    DBCR0 |= DAC;
#ifdef DEBUG_GDB
    printf("After DBCR0 = 0x%lx\n", DBCR0);
#endif
    mtspr(SPRN_DBCR0, DBCR0);
    //~ ea->srr1 |= 0x400000;
    ((struct regs *)pok_threads[*using_thread].entry_sp)->srr1 |= 0x200;
    watchpoint_is_set = TRUE;
    strcpy(remcomOutBuffer, "OK");
#endif
}

void remove_watchpoint(uintptr_t addr, int length, int *using_thread, int type){
#ifdef QEMU
    /*do nothing*/
    strcpy (remcomOutBuffer, "E22");
    return;
#else
    /*do nothing*/
    if ((!watchpoint_is_set) || (type > 4) || (type < 2)){
        
        strcpy (remcomOutBuffer, "E22");
        return;
    }
    strcpy(remcomOutBuffer, "OK");
    watchpoint_is_set = FALSE;
#ifdef DEBUG_GDB
    printf("\nWatchpoint_is_set = %d \n",watchpoint_is_set);
#endif
    uint32_t DBCR0 = mfspr(SPRN_DBCR0);
#ifdef DEBUG_GDB
    printf("Before set MSR DBCR0 = 0x%lx\n", DBCR0);
#endif
    struct regs * MSR = (struct regs *)pok_threads[*using_thread].entry_sp;
    MSR->srr1 &= (~0x200);
    DBCR0 = mfspr(SPRN_DBCR0);    
#ifdef DEBUG_GDB
    printf("Before DBCR0 = 0x%lx\n", DBCR0);
#endif
    DBCR0 &= (~0x400F0000UL);
#ifdef DEBUG_GDB
    printf("After DBCR0 = 0x%lx\n", DBCR0);
#endif
    mtspr(SPRN_DBCR0, DBCR0);
    uint32_t DBSR = mfspr(SPRN_DBSR);
#ifdef DEBUG_GDB
    printf("Before DBSR = 0x%lx\n", DBSR);
#endif
    DBSR &= (~0xF0000);
    mtspr(SPRN_DBSRWR, DBSR);
    DBSR = mfspr(SPRN_DBSR);
#ifdef DEBUG_GDB
    printf("After DBSR = 0x%lx\n", DBSR);
#endif
    uint32_t DBCR2 = mfspr(SPRN_DBCR2);
    DBCR2 &= (~0x800000);
    mtspr(SPRN_DBCR2, DBCR2);
    
    //~ ea->srr1 &= (~0x400000);
    
#endif    
    
}



void add_0_breakpoint(uintptr_t addr, int length, int *using_thread){
    int old_pid = current_part_id();
    int new_pid = give_part_num_of_thread(*using_thread + 1);
#ifdef DEBUG_GDB
    printf("New_pid = %d\n",new_pid);
    printf("Old_pid = %d\n",old_pid);    
#endif
    if (POK_CHECK_ADDR_IN_PARTITION(new_pid, addr))
    { 
#ifdef DEBUG_GDB
        printf("Load new_pid\n");
#endif
        switch_part_id(old_pid, new_pid);
    }
    if (!mem2hex((char *)addr, &(breakpoints[Head_of_breakpoints].Instr), length)){
        strcpy (remcomOutBuffer, "E22");
        switch_part_id(new_pid, old_pid);    
        return;
    }

    last_breakpoint++;

    breakpoints[Head_of_breakpoints].T_num = *using_thread + 1;
    breakpoints[Head_of_breakpoints].P_num = new_pid;
    breakpoints[Head_of_breakpoints].B_num = last_breakpoint;
    breakpoints[Head_of_breakpoints].Reason = 2;
    breakpoints[Head_of_breakpoints].addr = addr;
    Head_of_breakpoints++;
    if (Head_of_breakpoints == max_breakpoint){
        strcpy(remcomOutBuffer, "E22");
        switch_part_id(new_pid, old_pid);    
        return;
    }

    if (hex2mem(trap, (char *)addr, length)) {
        strcpy(remcomOutBuffer, "OK");
#ifdef DEBUG_GDB
        printf("hex2mem: addr = 0x%x; instr = 0x%lx", addr, *(uint32_t *)addr);
#endif
    } else {
        strcpy(remcomOutBuffer, "E22");
        switch_part_id(new_pid, old_pid);    
        return;
    }
    switch_part_id(new_pid, old_pid);
}

void remove_0_breakpoint(uintptr_t addr, int length, int *using_thread){
#ifdef DEBUG_GDB
    printf("            Z0, breakpoint[0].addr = 0x%x\n",breakpoints[0].addr);
#endif
    int i = 0;
    int old_pid = current_part_id();
    int new_pid = give_part_num_of_thread(*using_thread + 1);
#ifdef DEBUG_GDB
    printf("New_pid = %d\n",new_pid);
    printf("Old_pid = %d\n",old_pid);    
#endif
    if (POK_CHECK_ADDR_IN_PARTITION(new_pid, addr))
    { 
#ifdef DEBUG_GDB
        printf("Load new_pid\n");
#endif
        switch_part_id(old_pid, new_pid);
    }
    for (i = 0; i < max_breakpoint; i++){
        if ((breakpoints[i].addr == addr) && (breakpoints[i].P_num == new_pid))
            break;
    }
    if (i == max_breakpoint){
        strcpy (remcomOutBuffer, "E22");
        switch_part_id(new_pid, old_pid);    
        return;        
    }
    b_need_to_delete = -1;
    breakpoints[i].T_num = 0;
    breakpoints[i].P_num = 0;
    breakpoints[i].B_num = 0;
    breakpoints[i].Reason = 0;
    breakpoints[i].addr = 0;

    if (hex2mem(breakpoints[i].Instr, (char *)addr, length)) {
        strcpy(remcomOutBuffer, "OK");
#ifdef DEBUG_GDB
        printf("hex2mem: addr = 0x%x; instr = 0x%lx", addr, *(uint32_t *)addr);
#endif
    } else {
        strcpy (remcomOutBuffer, "E22");
        switch_part_id(new_pid, old_pid);    
        return;
    }
    switch_part_id(new_pid, old_pid);
}
    

void set_regs(struct regs *ea){
    if ((uint32_t) ea == 0) {
        ea =  &null_ea;
    }
#ifdef __PPC__
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
    fp_registers[f0] = 0;
    fp_registers[f1] = 0;
    fp_registers[f2] = 0;
    fp_registers[f3] = 0;
    fp_registers[f4] = 0;
    fp_registers[f5] = 0;
    fp_registers[f6] = 0;
    fp_registers[f7] = 0;
    fp_registers[f8] = 0;
    fp_registers[f9] = 0;
    fp_registers[f10] = 0;
    fp_registers[f11] = 0;
    fp_registers[f12] = 0;
    fp_registers[f13] = 0;
    fp_registers[f14] = 0;
    fp_registers[f15] = 0;
    fp_registers[f16] = 0;
    fp_registers[f17] = 0;
    fp_registers[f18] = 0;
    fp_registers[f19] = 0;
    fp_registers[f20] = 0;
    fp_registers[f21] = 0;
    fp_registers[f22] = 0;
    fp_registers[f23] = 0;
    fp_registers[f24] = 0;
    fp_registers[f25] = 0;
    fp_registers[f26] = 0;
    fp_registers[f27] = 0;
    fp_registers[f28] = 0;
    fp_registers[f29] = 0;
    fp_registers[f30] = 0;
    fp_registers[f31] = 0;
    
#endif        
#ifdef __i386__
    registers[EAX] = ea->eax;
    registers[ECX] = ea->ecx;
    registers[EDX] = ea->edx;
    registers[EBX] = ea->ebx;
    registers[ESP] = ea->__esp;
    registers[EBP] = ea->ebp;
    registers[ESI] = ea->esi;
    registers[EDI] = ea->edi;
    registers[PC] = ea->eip;
    registers[PS] = ea->eflags;
    registers[CS] = ea->cs;
    registers[SS] = ea->ss;
    registers[DS] = ea->ds;
    registers[ES] = ea->es;
    registers[FS] = -1;
    registers[GS] = -1;
#endif
}


int new_start;

void clear_breakpoints(){
    for (int i = 0; i < max_breakpoint; i++)
        breakpoints[i].addr = 0;
}

/*
 * This function does all command procesing for interfacing to gdb.
 */
void
handle_exception (int exceptionVector, struct regs * ea)
{
    Connect_to_new_inferior = -1;
    /*Add regs*/
    uint32_t old_entryS = pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp;
    pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = (uint32_t) ea;
    
    set_regs(ea);
    int using_thread = POK_SCHED_CURRENT_THREAD;
    int number_of_thread = 0;
  
    memset(remcomOutBuffer, 0, BUFMAX);
    memset(remcomInBuffer, 0, BUFMAX);
    int sigval;
#ifdef __i386__
    pok_bool_t stepping = FALSE;
#endif
    uintptr_t addr;
    int length;
    char *ptr;


    if (addr_instr != 0){
#ifdef __PPC__
        ea->srr1 |= MSR_EE;
        hex2mem(instr, (char *) (addr_instr), 4);
        addr_instr = 0;
        if (addr_instr2 != 0){
            hex2mem(instr2, (char *) (addr_instr2), 4);
            addr_instr2 = 0;
        }
#endif
//~ #ifdef __i386__
    //~ if (addr_instr2 != 0){
        //~ hex2mem(instr, (char *) (addr_instr), 1);
        //~ addr_instr = 0;
    //~ }
//~ #endif
    }
  
  /* reply to host that an exception has occurred */
    sigval = computeSignal (exceptionVector);

    ptr = remcomOutBuffer;
#ifdef __PPC__
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
    *ptr++ = 't';
    *ptr++ = 'h';
    *ptr++ = 'r';
    *ptr++ = 'e';
    *ptr++ = 'a';
    *ptr++ = 'd';
    *ptr++ = ':';

    int thread_num = POK_SCHED_CURRENT_THREAD + 1;
#ifdef MULTIPROCESS
    int part_of_this_thread = give_part_num_of_thread(thread_num) + 1;////give_part_num_of_thread(thread_num);
    *ptr++ = 'p';
    ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
    *ptr++ = '.';
#endif
    ptr = mem2hex( (char *)(&(thread_num)), ptr, 4); 
    *ptr++ = ';';


#ifdef QEMU

#else
    if (exceptionVector == 1){
        *ptr++ = 'w';
        *ptr++ = 'a';
        *ptr++ = 't';
        *ptr++ = 'c';
        *ptr++ = 'h';
        *ptr++ = ':';
        addr =  mfspr(SPRN_DAC1);
        ptr = mem2hex( (char *)&addr, ptr, 4, 0); 
        *ptr++ = ';';
    }        
#endif

    ptr = 0;
#endif
#ifdef __i386__

    *ptr++ = 'T';         /* notify gdb with signo, PC, FP and SP */
    *ptr++ = hexchars[sigval >> 4];
    *ptr++ = hexchars[sigval & 0xf];

    *ptr++ = hexchars[ESP]; 
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[ESP], ptr, 4, 0);    /* SP */
    *ptr++ = ';';

    *ptr++ = hexchars[EBP]; 
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[EBP], ptr, 4, 0);    /* FP */
    *ptr++ = ';';

    *ptr++ = hexchars[PC]; 
    *ptr++ = ':';
    ptr = mem2hex((char *)&registers[PC], ptr, 4, 0);     /* PC */
    *ptr++ = ';';
    *ptr++ = 't';
    *ptr++ = 'h';
    *ptr++ = 'r';
    *ptr++ = 'e';
    *ptr++ = 'a';
    *ptr++ = 'd';
    *ptr++ = ':';
    int thread_num = POK_SCHED_CURRENT_THREAD + 1;
    ptr = mem2hex( (char *)&thread_num, ptr, 1); 
    *ptr++ = ';';
  *ptr = 0;
#endif      
    data_to_read_1();
    //~ int flag = 0;
    putpacket ( (unsigned char *) remcomOutBuffer);

    
    while (1) {
#ifdef __PPC__
    data_to_read_1();

    remcomOutBuffer[0] = 0;
    
    getpacket(remcomInBuffer);
    //~ if (flag == 0){
        //~ while (data_to_read_1()){
            //~ getDebugChar();
        //~ }
        //~ flag = 1;
    //~ }
    switch (remcomInBuffer[0]) {

        case 'T':               /*Find out if the thread thread-id is alive*/
            ptr = &remcomInBuffer[1];
            int thread_num = -1;
#ifdef MULTIPROCESS
            /*FIX IT*/
            while (* ptr != '.')
                ptr++;
            ptr++;
#endif
            hexToInt(&ptr, (uintptr_t *)(&thread_num));
            if ( thread_num > 0 && thread_num < POK_CONFIG_NB_THREADS + 1){
                remcomOutBuffer[0] = 'O';
                remcomOutBuffer[1] = 'K';
                remcomOutBuffer[2] = 0;
            }
            break;
            /*
             * Insert (‘Z’) or remove (‘z’) a type breakpoint or watchpoint starting at address address of kind kind.
             */
        case 'Z':
        case 'z':
            {
                if (new_start == 1) {
#ifdef DEBUG_GDB
                    printf("New_start = %d",new_start);
#endif
                    new_start = 0;
#ifdef DEBUG_GDB
                    printf("New_start = %d",new_start);
#endif
                    clear_breakpoints();
                }         
                ptr = &remcomInBuffer[1];
                int type = -1;
                hexToInt(&ptr, (uintptr_t *)(&type));
                if (type == -1) break;
                if (*ptr != ',') break;
                ptr++;
                hexToInt(&ptr, &addr);
                if (*ptr != ',') break;
                ptr++;
                int kind = -1;
                hexToInt(&ptr, (uintptr_t *)(&kind));
                if (kind == -1) break;
                if (type == 0){
                    if (remcomInBuffer[0] == 'Z') 
                            add_0_breakpoint(addr, kind, &using_thread);
                        else
                            remove_0_breakpoint(addr, kind, &using_thread);
                }
                if (type == 2){
                    if (remcomInBuffer[0] == 'Z') 
                            add_watchpoint(addr, kind, &using_thread, 2);
                        else
                            remove_watchpoint(addr, kind, &using_thread, 2);
                }
                if (type == 3){
                    if (remcomInBuffer[0] == 'Z') 
                            add_watchpoint(addr, kind, &using_thread, 3);
                        else
                            remove_watchpoint(addr, kind, &using_thread, 3);
                }
                if (type == 4){
                    if (remcomInBuffer[0] == 'Z') 
                            add_watchpoint(addr, kind, &using_thread, 4);
                        else
                            remove_watchpoint(addr, kind, &using_thread, 4);
                }
                break;
            }
        case '?':               /* report most recent signal */
            remcomOutBuffer[0] = 'S';
            remcomOutBuffer[1] = hexchars[sigval >> 4];
            remcomOutBuffer[2] = hexchars[sigval & 0xf];
            remcomOutBuffer[3] = 0;
            break;
        case 'q': /* this screws up gdb for some reason...*/
        {
            //~ extern long _start, sdata, __bss_start;
            ptr = &remcomInBuffer[1];
            if (strncmp(ptr, "C", 1) == 0){
                ptr = remcomOutBuffer;
                *ptr++ = 'Q';
                *ptr++ = 'C';
                uint32_t p;
                if (Connect_to_new_inferior == 1){
                    p = POK_SCHED_CURRENT_THREAD + 1;
#ifdef MULTIPROCESS
                ////TODO: Change number of process
                    int part_of_this_thread = give_part_num_of_thread(p) + 1;
                    *ptr++ = 'p';
                    ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                    *ptr++ = '.';
#endif 

                    ptr = mem2hex( (char *)(&p), ptr, 4); 
                    *ptr++ = 0;
                    Connect_to_new_inferior = -1;
                    putpacket ( (unsigned char *) remcomOutBuffer);
                    pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = old_entryS;
               
                    set_regs(ea);
                    ptr = &remcomInBuffer[1];
                    if (hexToInt(&ptr, &addr)) {
                        registers[pc]/*nip*/ = addr;
                    }
                    ea->srr0 = ea->srr0 - 4;
                    return;                    
                    
                }else{
                    p = POK_SCHED_CURRENT_THREAD + 1;
                }
#ifdef MULTIPROCESS

                ////TODO: Change number of process
                int part_of_this_thread = give_part_num_of_thread(p) + 1;
                *ptr++ = 'p';
                ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                *ptr++ = '.';
#endif 

                ptr = mem2hex( (char *)(&p), ptr, 4); 
                *ptr++ = 0;
                break;
            }
            if (strncmp(ptr, "Offsets", 7) == 0){
                //~ strcpy(remcomOutBuffer,"OK");
                //~ break;
                ////FIX IT
                //~ if (SS == 0){
                    //~ SS ++;
                    //~ ptr = remcomOutBuffer;
                    //~ break;
                //~ }
                ptr = remcomOutBuffer;
                //~ *ptr++ = 'T';
                //~ *ptr++ = 'e';
                //~ *ptr++ = 'x';
                //~ *ptr++ = 't';
                //~ *ptr++ = 'S';
//~ 
                //~ *ptr++ = 'e';
                //~ *ptr++ = 'g';
                //~ *ptr++ = '=';
                    //~ uint32_t offset = 0x10000;
                //~ ptr = mem2hex((char *) offset, ptr, 4);
                *ptr++ = 0;
            //~ sprintf(ptr, "Text=%8.8x;Data=%8.8x;Bss=%8.8x",
                //~ &_start, &sdata, &__bss_start);
                break;
            }
            if (strncmp(ptr, "Attached:", 9) == 0){
                ptr += 9;
                int part_id;
                hexToInt(&ptr, (uintptr_t *)(&part_id));
                ptr = remcomOutBuffer;
//~ #ifdef MULTIPROCESS
                //~ if (part_id <= POK_CONFIG_NB_PARTITIONS + 2 && part_id >= 1)
//~ #endif
                    *ptr++ = '1';
                *ptr = 0;
                break;
            }
            if (strncmp(ptr, "Symbol::", 8) == 0){

                ptr = remcomOutBuffer;
                remcomOutBuffer[0] = 'O';
                remcomOutBuffer[1] = 'K';
                remcomOutBuffer[2] = 0;
                break;
            }
            if (strncmp(ptr, "Supported", 9) == 0){
#ifdef MULTIPROCESS
                ptr+= (9 + 1); //qSupported:
                while  (strncmp(ptr, "multiprocess", 9) != 0){
                    ptr++;
                    if (*ptr == '+' && *(ptr+1) != ';'){
                        break;
                    }
                }
                if (strncmp(ptr, "multiprocess", 9) == 0){
                    char * answer = "multiprocess+";
                    ptr = remcomOutBuffer;
                    for (int i = 0; i < 13; i++)
                    *ptr++ = answer[i];
                }
                
                *ptr++ = 0;
#endif
                break;
                
            }
            if (strncmp(ptr, "fThreadInfo", 11) == 0)   {
                number_of_thread = 1;
                ptr = remcomOutBuffer;  
                *ptr++ = 'm';
                int previous_thread = 1;
#ifdef MULTIPROCESS
                ////TODO: Change number of process
                int part_of_this_thread = give_part_num_of_thread(previous_thread) + 1;
                *ptr++ = 'p';
                ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                *ptr++ = '.';
#endif
                ptr = mem2hex( (char *)(&previous_thread), ptr, 4); 
                *ptr++ = 0;
                number_of_thread++;
                break;
            }
            if (strncmp(ptr, "sThreadInfo", 11) == 0){
                if (number_of_thread == POK_CONFIG_NB_THREADS + 1){
                    ptr = remcomOutBuffer;
                    *ptr++ = 'l';
                    *ptr++ = 0;
                    break;
                }
                ptr = remcomOutBuffer;
                *ptr++ = 'm';
                int previous_thread = number_of_thread;
#ifdef MULTIPROCESS
                int part_of_this_thread = give_part_num_of_thread(previous_thread) + 1;
                *ptr++ = 'p';
                ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                *ptr++ = '.';
#endif
                ptr = mem2hex( (char *)(&previous_thread), ptr, 4); 
                number_of_thread++;
                *ptr++ = 0;
                break;
             }
             if (strncmp(ptr, "ThreadExtraInfo", 15) == 0){
                ptr += 16;
                int thread_num;
#ifdef MULTIPROCESS
                /*FIX IT*/
                while (* ptr != '.')
                    ptr++;
                ptr++;
#endif
                hexToInt(&ptr, (uintptr_t *)(&thread_num));
                thread_num --;


                ptr = remcomOutBuffer;
                int info_offset = 0;
                int lengh = pok_thread_info(pok_threads[thread_num].state, &info_offset);
                if (thread_num == POK_SCHED_CURRENT_THREAD){
                    ptr = mem2hex( (char *) &("* "), ptr, 2);
                }
                
                //FIXME
                ptr = mem2hex( (char *) &("P"), ptr, 1);
                int pid = give_part_num_of_thread(thread_num + 1);
                if (pid == 0) ptr = mem2hex( (char *) &("0 "), ptr, 2);
                if (pid == 1) ptr = mem2hex( (char *) &("1 "), ptr, 2);
                if (pid == 2) ptr = mem2hex( (char *) &("2 "), ptr, 2);
                if (pid == 3) ptr = mem2hex( (char *) &("3 "), ptr, 2);
                if (pid == 4) ptr = mem2hex( (char *) &("4 "), ptr, 2);
                if (pid == 5) ptr = mem2hex( (char *) &("5 "), ptr, 2);
                

                if (thread_num == MONITOR_THREAD){
                    ptr = mem2hex( (char *) &("MONITOR "), ptr, 8);
                }
                if (thread_num == GDB_THREAD){
                    ptr = mem2hex( (char *) &("GDB "), ptr, 4);
                }
                if (thread_num == IDLE_THREAD){
                    ptr = mem2hex( (char *) &("IDLE "), ptr, 5);
                }
                ptr = mem2hex( (char *) (&info_thread[info_offset]), ptr, lengh);
                *ptr++ = 0;
                break;
            }

            break;
        }

        case 'g':   /* return the value of the CPU registers.
                 * some of them are non-PowerPC names :(
                 * they are stored in gdb like:
                 * struct {
                 *     u32 gpr[32];
                 *     f64 fpr[32];
                 *     u32 pc, ps, cnd, lr; (ps=msr)
                 *     u32 cnt, xer, mq;
                 * }
                 */
        {
            ptr = remcomOutBuffer;
            uint32_t old_pc = registers[pc];
            /* General Purpose Regs */
            ptr = mem2hex((char *)registers, ptr, 32 * 4);
            /* Floating Point registers - FIXME */
            ptr = mem2hex((char *)fp_registers, ptr, 32 * 8);
            //~ for(i=0; i<(32*8*2); i++) { /* 2chars/byte */
                //~ ptr[i] = '0';
            //~ }
            //~ ptr += 32*8*2;
            /* pc, msr, cr, lr, ctr, xer, (mq is unused) */
            ptr = mem2hex((char *)&registers[pc], ptr, 4);
            ptr = mem2hex((char *)&registers[msr], ptr, 4);
            ptr = mem2hex((char *)&registers[cr]/*[ccr]*/, ptr, 4);
            ptr = mem2hex((char *)&registers[lr]/*[link]*/, ptr, 4);
            ptr = mem2hex((char *)&registers[ctr], ptr, 4);
            ptr = mem2hex((char *)&registers[xer], ptr, 4);
            registers[pc] = old_pc;
        }
            break;

        case 'G':   /* set the value of the CPU registers */
        {
            ptr = &remcomInBuffer[1];

            /*
             * If the stack pointer has moved, you should pray.
             * (cause only god can help you).
             */

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

            strcpy(remcomOutBuffer,"OK");
            }
            break;
        case 'H':                   /*Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
            if (number_of_thread == 1){
                //TODO: FIX IT
                strcpy(remcomOutBuffer,"OK");
                //~ printf("\nH break\n");
                break;
            }    
            ptr = &remcomInBuffer[1];
            if ( *ptr == 'c'){
                using_thread = POK_SCHED_CURRENT_THREAD;
                //~ printf("pok_threads[%d].sp=0x%lx\n",using_thread,pok_threads[using_thread].sp);
                //~ printf("pok_threads[%d].entry_sp=0x%lx\n",using_thread,pok_threads[using_thread].entry_sp);
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
                
            }else if (*ptr++ == 'g'){
#ifdef MULTIPROCESS
                /*FIX IT*/
                /*Read 'p'*/
                ptr++;
                uintptr_t pid = -1;
                hexToInt(&ptr, &pid);
                ptr++;
#ifdef DEBUG_GDB
                printf("PID = %d\n", pid);
#endif
#endif
                hexToInt(&ptr, &addr);
                if (addr != -1 && addr != 0) 
                {
                    using_thread = addr;
                    using_thread --;
                    //~ strcpy(remcomOutBuffer,"OK");
                    //~ printf("\nH-1 break\n");
                    //~ break;
                
                }else{
#ifdef DEBUG_GDB
                    printf("ADDR = 0\n");
#endif
                    if (pid == 1) using_thread = IDLE_THREAD;
                    else using_thread = pok_partitions[pid - 2].thread_index_low + 1;
                }
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
            }
            //~ printf("\nH\n");
            
            strcpy(remcomOutBuffer,"OK");
            break;

        case 'm':   /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
                /* Try to read %x,%x.  */
            {
                ptr = &remcomInBuffer[1];
                int old_pid = current_part_id();
                int new_pid = give_part_num_of_thread(using_thread + 1);
#ifdef DEBUG_GDB
                printf("New_pid = %d\n",new_pid);
                printf("Old_pid = %d\n",old_pid);
#endif
                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, (uintptr_t *)(&length))) {
                    if (!POK_CHECK_ADDR_IN_PARTITION(new_pid,addr)){
                        strcpy (remcomOutBuffer, "E03");
                        break;
                    }else{ 
#ifdef DEBUG_GDB
                        printf("Load new_pid\n");
#endif
                        switch_part_id(old_pid, new_pid);
                    }
                    if (mem2hex((char *)addr, remcomOutBuffer,length)){
                        switch_part_id(new_pid, old_pid);
                        break;
                    }
                    strcpy (remcomOutBuffer, "E03");
                    if (POK_CHECK_ADDR_IN_PARTITION(new_pid,addr))
                        switch_part_id(new_pid, old_pid);
                } else {
                    strcpy(remcomOutBuffer,"E01");
                }
                break;
            }
        case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
            /* Try to read '%x,%x:'.  */
            {

                ptr = &remcomInBuffer[1];
                int old_pid = current_part_id();
                int new_pid = give_part_num_of_thread(using_thread + 1);
                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, (uintptr_t *)(&length))
                    && *ptr++ == ':') {
                    if (POK_CHECK_ADDR_IN_PARTITION(new_pid,addr))
                        switch_part_id(old_pid, new_pid);
                    else{
                        strcpy (remcomOutBuffer, "E03");
                        break;
                    }
                    if (strncmp(ptr, "7d821008", 8) == 0)
                        ptr = trap;
                    if (hex2mem(ptr, (char *)addr, length)) {
                        strcpy(remcomOutBuffer, "OK");
                    } else {
                        strcpy(remcomOutBuffer, "E03");
                    }
                    if (POK_CHECK_ADDR_IN_PARTITION(new_pid,addr))
                        switch_part_id(new_pid, old_pid);
                } else {
                    strcpy(remcomOutBuffer, "E02");
                }
                break;
            }

#ifdef MULTIPROCESS
        case 'v':               /*
                                 * Packets starting with ‘v’ are identified by a multi-letter name, up to the first ‘;’ or ‘?’ (or the end of the packet). 
                                 */
            {
#ifdef DEBUG_GDB
            printf("IN V\n");
            printf("String = %s, %d\n", remcomOutBuffer, remcomOutBuffer[0]);
#endif
            ptr = &remcomInBuffer[1];
            if (strncmp(ptr, "Attach;", 7) == 0)   {
#ifdef DEBUG_GDB
                printf("Added reply\n");
#endif
                strcpy(remcomOutBuffer, "Any stop packet");
            }
            if (strncmp(ptr, "Cont;c", 6) == 0) {
                pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = old_entryS;
           
                set_regs(ea);
                ptr = &remcomInBuffer[1];
                if (hexToInt(&ptr, &addr)) {
                    registers[pc]/*nip*/ = addr;
                }
                return;
            }         
            Connect_to_new_inferior = 1;
            break;
            //~ putpacket((unsigned char *)remcomOutBuffer);            
            //~ return;
        }
        
        case 'D':               /*
                                 * The first form of the packet is used to detach gdb from the remote system
                                 * It is sent to the remote target before gdb disconnects via the detach command.
                                 */
            {
#ifdef DEBUG_GDB
            printf("HERE\n");
#endif
            ptr = &remcomInBuffer[2];
            int part_id;
            hexToInt(&ptr, (uintptr_t *)(&part_id));
            remcomOutBuffer[0] = 'O';
            remcomOutBuffer[1] = 'K';
            remcomOutBuffer[2] = 0;
            
            /*If it is last detaching process - continue*/
            if (part_id != 1) break;
            putpacket((unsigned char *)remcomOutBuffer);            
        }
#endif
        case 'k':    /* kill the program, actually just continue */
        case 'c':    /* cAA..AA  Continue; address AA..AA optional */
            /* try to read optional parameter, pc unchanged if no parm */

            pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = old_entryS;
       
            set_regs(ea);
            ptr = &remcomInBuffer[1];
            if (hexToInt(&ptr, &addr)) {
                registers[pc]/*nip*/ = addr;
            }
            return;

        case 's':
        {
            ////TODO:Use special registers for single step instruction
            pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = old_entryS;
            set_regs(ea);
            uint32_t inst = *((uint32_t *)registers[pc]);
            //~ printf("inst=0x%lx;\n",inst);
            uint32_t c_inst = registers[pc];
            //~ printf("c_inst=0x%lx;\n",c_inst);
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
                    //~ printf("%lx\n",(inst + c_inst));
                    inst = inst + c_inst;
                    mem2hex((char *)(registers[pc]+4), instr,4);            
                    hex2mem(trap, (char *)(registers[pc]+4), 4);
                    addr_instr = registers[pc] + 4;
                    mem2hex((char *)(inst), instr2,4);            
                    hex2mem(trap, (char *)(inst), 4);
                    addr_instr2 = inst;
                    return;
                }else{
                    //~ printf("%lx\n",((inst << 6) >> 8) << 2);
                    inst = ((inst << 6) >> 8) << 2;
                    mem2hex((char *)(registers[pc]+4), instr,4);            
                    hex2mem(trap, (char *)(registers[pc]+4), 4);
                    addr_instr = registers[pc] + 4;
                    mem2hex((char *)(inst), instr2,4);            
                    hex2mem(trap, (char *)(inst), 4);
                    addr_instr2 = inst;
                    return;
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
                    //~ printf("%lx\n",(inst + c_inst));
                    inst = inst + c_inst;
                    mem2hex((char *)(registers[pc]+4), instr,4);            
                    hex2mem(trap, (char *)(registers[pc]+4), 4);
                    addr_instr = registers[pc] + 4;
                    //~ printf("%lx\n",(inst + c_inst));
                    mem2hex((char *)(inst), instr2,4);            
                    hex2mem(trap, (char *)(inst), 4);
                    addr_instr2 = inst;
                    return;
            
                }else{
                    //~ printf("%lx\n",((inst << 16) >> 18) << 2);
                    inst = ((inst << 16) >> 18) << 2;
                    mem2hex((char *)(registers[pc]+4), instr,4);            
                    hex2mem(trap, (char *)(registers[pc]+4), 4);
                    addr_instr = registers[pc] + 4;
                    mem2hex((char *)(inst), instr2,4);            
                    hex2mem(trap, (char *)(inst), 4);
                    addr_instr2 = inst;
                    return;
                }
                if (inst & 0x1){
                }
            }
/*
 *  If it's a 'brl' instruction
 */            
            if ((inst >> (6*4+2)) == 0x13){
                //~ printf("lr=%lx\n",registers[lr]);
                mem2hex((char *)(registers[lr]), instr,4);            
                hex2mem(trap, (char *)(registers[lr]), 4);
                addr_instr = registers[lr];                
                mem2hex((char *)(registers[pc]+4), instr2,4);            
                hex2mem(trap, (char *)(registers[pc]+4), 4);
                addr_instr2 = registers[pc] + 4;
                return;
            }
            mem2hex((char *)(registers[pc]+4), instr,4);            
            hex2mem(trap, (char *)(registers[pc]+4), 4);
            addr_instr = registers[pc] + 4;
            return;
        }
        case 'r':       /* Reset (if user process..exit ???)*/
////            panic("kgdb reset.");
            break;
        }           /* switch */
        /* reply to the request */
        putpacket((unsigned char *)remcomOutBuffer);
#endif
#ifdef __i386__
    remcomOutBuffer[0] = 0;
    ptr = (char *)getpacket ();

    switch (*ptr++)
    {
            /*
             * Insert (‘Z’) or remove (‘z’) a type breakpoint or watchpoint starting at address address of kind kind.
             */
        case 'Z':
        case 'z':
            {
                if (new_start == 1) {
#ifdef DEBUG_GDB
                    printf("New_start = %d",new_start);
#endif
                    new_start = 0;
#ifdef DEBUG_GDB
                    printf("New_start = %d",new_start);
#endif
                    clear_breakpoints();
                }         
                ptr = &remcomInBuffer[1];
                int type = -1;
                hexToInt(&ptr, &type);
                if (type == -1) break;
                if (*ptr != ',') break;
                ptr++;
                hexToInt(&ptr, &addr);
                if (*ptr != ',') break;
                ptr++;
                int kind = -1;
                hexToInt(&ptr, &kind);
                if (kind == -1) break;
                if (type == 0){
                    if (remcomInBuffer[0] == 'Z') 
                            add_0_breakpoint(&addr,&kind,&using_thread);
                        else
                            remove_0_breakpoint(&addr,&kind,&using_thread);
                }
                break;
            }
        case 'T':               /*Find out if the thread thread-id is alive*/
            ptr = &remcomInBuffer[1];
            int thread_num = -1;
            hexToInt(&ptr, &thread_num);
            if ( thread_num > 0 && thread_num < POK_CONFIG_NB_THREADS + 1){
                remcomOutBuffer[0] = 'O';
                remcomOutBuffer[1] = 'K';
                remcomOutBuffer[2] = 0;
            }
            break;
        case 'q': /* this screws up gdb for some reason...*/
        {
            //~ extern long _start, sdata, __bss_start;
            ptr = &remcomInBuffer[1];
            if (strncmp(ptr, "C", 1) == 0){
                ptr = remcomOutBuffer;
                *ptr++ = 'Q';
                *ptr++ = 'C';
                uint32_t p = POK_SCHED_CURRENT_THREAD + 1;
                ptr = mem2hex( (char *)(&p),ptr,1); 
                *ptr++ = 0;
                break;
            }
            if (strncmp(ptr, "Offsets", 7) == 0){
                //~ strcpy(remcomOutBuffer,"OK");
                //~ break;
                ptr = remcomOutBuffer;
            //~ sprintf(ptr, "Text=%8.8x;Data=%8.8x;Bss=%8.8x",
                //~ &_start, &sdata, &__bss_start);
                break;
            }
            if (strncmp(ptr, "Supported", 9) == 0){
                /*FIX IT*/
                //~ char * answer = "multiprocess+";
                //~ ptr = remcomOutBuffer;
                //~ for (int i=0; i < 13; i++)
                    //~ *ptr++ = answer[i];
                //~ *ptr++ = 0;
                
                
                break;
                
            }
            if (strncmp(ptr, "fThreadInfo", 11) == 0)   {
                number_of_thread = 1;
#ifdef DEBUG_GDB
                printf("in first if\n");
#endif
                ptr = remcomOutBuffer;  
                *ptr++ = 'm';
                int previous_thread = 1;
                ptr = mem2hex( (char *)(&previous_thread), ptr, 1); 
                *ptr++ = 0;
                number_of_thread++;
                break;
            }
            if (strncmp(ptr, "sThreadInfo", 11) == 0){
                if (number_of_thread == POK_CONFIG_NB_THREADS +1){
                    ptr = remcomOutBuffer;
                    *ptr++ = 'l';
                    *ptr++ = 0;
                    break;
                }
                ptr = remcomOutBuffer;
                *ptr++ = 'm';
                int previous_thread = number_of_thread;
                ptr = mem2hex( (char *)(&previous_thread), ptr, 1); 
                number_of_thread++;
                *ptr++ = 0;
                break;
             }
             if (strncmp(ptr, "ThreadExtraInfo", 15) == 0){
                ptr += 16;
                int thread_num;
                /*FIX IT*/
                hexToInt(&ptr, &thread_num);
                thread_num --;
 #ifdef DEBUG_GDB
                printf("thread_num=%d\n",thread_num);
                printf("pok_threads[%d].state=%d\n",thread_num,pok_threads[thread_num].state);
#endif
                //~ struct thread_stack * id = (struct thread_stack *) pok_threads[thread_num].sp;


                ptr = remcomOutBuffer;
                int info_offset = 0;
                int lengh = pok_thread_info(pok_threads[thread_num].state, &info_offset);
                if (thread_num == POK_SCHED_CURRENT_THREAD){
                    ptr = mem2hex( (char *) &("* "), ptr, 2);
                }
                if (thread_num == MONITOR_THREAD){
                    ptr = mem2hex( (char *) &("MONITOR "), ptr, 8);
                }
                if (thread_num == GDB_THREAD){
                    ptr = mem2hex( (char *) &("GDB "), ptr, 4);
                }
                if (thread_num == IDLE_THREAD){
                    ptr = mem2hex( (char *) &("IDLE "), ptr, 5);
                }
#ifdef DEBUG_GDB
                printf("lengh = %d\n",lengh);
                printf("info_offset = %d\n",info_offset);
                printf("%c%c%c\n",info_thread[info_offset],info_thread[info_offset+1],info_thread[info_offset+2]);
#endif
                //~ strcpy(ptr,info);
                ptr = mem2hex( (char *) (&info_thread[info_offset]), ptr, lengh);
                *ptr++ = 0;
                break;
            }

        break;
        }
        case '?':
            remcomOutBuffer[0] = 'S';
            remcomOutBuffer[1] = hexchars[sigval >> 4];
            remcomOutBuffer[2] = hexchars[sigval % 16];
            remcomOutBuffer[3] = 0;
            break;
        case 'g':       /* return the value of the CPU registers */
            mem2hex ((char *) registers, remcomOutBuffer, NUMREGBYTES);
            break;
        case 'G':       /* set the value of the CPU registers - return OK */
            hex2mem (ptr, (char *) registers, NUMREGBYTES, 0);
            strcpy (remcomOutBuffer, "OK");
            break;
        case 'P':       /* set the value of a single CPU register - return OK */
        {
            int regno;

            if (hexToInt (&ptr, &regno) && *ptr++ == '=')
                if (regno >= 0 && regno < NUMREGS)
                {
                    hex2mem (ptr, (char *) &registers[regno], 4, 0);
                    strcpy (remcomOutBuffer, "OK");
                    break;
                }

            strcpy (remcomOutBuffer, "E01");
            break;
        }
        case 'p':       /* Read the value of register; Register is in hex. */
        {
            int regno;

            if (hexToInt (&ptr, &regno))
            {
                //~ printf("Regno = %d\n",regno);
                ///FIXME
                //~ if (regno == 64){
                    //~ mem2hex ((char *) &registers[PC], remcomOutBuffer, 4, 0);
                    //~ break;
                //~ }
                //~ if (regno == 67){
                    //~ mem2hex ((char *) &registers[Д], remcomOutBuffer, 4, 0);
                    //~ break;
                //~ }                
                if (regno >= 0 && regno < NUMREGS)
                {
                    
                    mem2hex ((char *) &registers[regno], remcomOutBuffer, 4, 0);
                    strcpy (remcomOutBuffer, "OK");
                    break;
                }
            }
            strcpy (remcomOutBuffer, "E01");
            break;
        }

        //~ case 'm':
      //~ /* TRY TO READ %x,%x.  IF SUCCEED, SET PTR = 0 */
            //~ if (hexToInt (&ptr, &addr))
                //~ if (*(ptr++) == ',')
                    //~ if (hexToInt (&ptr, &length))
                    //~ {
                        //~ ptr = 0;
                        //~ mem_err = 0;
                        //~ mem2hex ((char *) addr, remcomOutBuffer, length, 1);
                        //~ if (mem_err)
                        //~ {
                            //~ strcpy (remcomOutBuffer, "E03");
                        //~ }
                    //~ }
//~ 
            //~ if (ptr)
            //~ {
                //~ strcpy (remcomOutBuffer, "E01");
            //~ }
            //~ break;
      /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
        case 'm':   /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
                /* Try to read %x,%x.  */
            {
                ptr = &remcomInBuffer[1];
                int old_pid = current_part_id();
                int new_pid = give_part_num_of_thread(using_thread + 1);
#ifdef DEBUG_GDB
                printf("New_pid = %d\n",new_pid);
                printf("Old_pid = %d\n",old_pid);
#endif
                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, &length)) {
                    if (!POK_CHECK_ADDR_IN_PARTITION(new_pid,addr)){
                        
                        strcpy (remcomOutBuffer, "E03");
                        break;
                    }else{ 
#ifdef DEBUG_GDB
                        printf("Load new_pid\n");
#endif
                        switch_part_id(old_pid, new_pid);
                    }
                    if (mem2hex((char *)addr, remcomOutBuffer,length)){
                        switch_part_id(new_pid, old_pid);
                        break;
                    }
                    strcpy (remcomOutBuffer, "E03");
                    if (POK_CHECK_ADDR_IN_PARTITION(new_pid,addr))
                        switch_part_id(new_pid, old_pid);
                } else {
                    strcpy(remcomOutBuffer,"E01");
                }
                break;
            }

      //~ /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
        //~ case 'M':
      //~ /* TRY TO READ '%x,%x:'.  IF SUCCEED, SET PTR = 0 */
            //~ if (hexToInt (&ptr, &addr))
                //~ if (*(ptr++) == ',')
                    //~ if (hexToInt (&ptr, &length))
                        //~ if (*(ptr++) == ':')
                        //~ {
                            //~ mem_err = 0;
                            //~ hex2mem (ptr, (char *) addr, length, 1);
                            //~ if (mem_err)
                            //~ {
                                //~ strcpy (remcomOutBuffer, "E03");
                            //~ }else{
                                //~ strcpy (remcomOutBuffer, "OK");
                            //~ }
//~ 
                            //~ ptr = 0;
                        //~ }
            //~ if (ptr)
            //~ {
                //~ strcpy (remcomOutBuffer, "E02");
            //~ }
            //~ break;
        case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
            /* Try to read '%x,%x:'.  */
            {

                ptr = &remcomInBuffer[1];
                int old_pid = current_part_id();
                int new_pid = give_part_num_of_thread(using_thread + 1);
                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, &length)
                    && *ptr++ == ':') {
                    if (POK_CHECK_ADDR_IN_PARTITION(new_pid,addr))
                        switch_part_id(old_pid, new_pid);
                    else{
                        strcpy (remcomOutBuffer, "E03");
                        break;
                    }
                    if (strncmp(ptr, "7d821008", 8) == 0)
                        ptr = trap;
                    if (hex2mem(ptr, (char *)addr, length)) {
                        strcpy(remcomOutBuffer, "OK");
                    } else {
                        strcpy(remcomOutBuffer, "E03");
                    }
                    if (POK_CHECK_ADDR_IN_PARTITION(new_pid,addr))
                        switch_part_id(new_pid, old_pid);
                } else {
                    strcpy(remcomOutBuffer, "E02");
                }
                break;
            }


        case 'H':                   /*Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
        {
            if (number_of_thread == 1){
            //TODO: FIX IT
                strcpy(remcomOutBuffer,"OK");
#ifdef DEBUG_GDB
                printf("\nH break\n");
#endif
                break;
            }    
            ptr = &remcomInBuffer[1];
            if ( *ptr == 'c'){
                using_thread = POK_SCHED_CURRENT_THREAD;
#ifdef DEBUG_GDB
                printf("pok_threads[%d].sp=0x%lx\n",using_thread,pok_threads[using_thread].sp);
                printf("pok_threads[%d].entry_sp=0x%lx\n",using_thread,pok_threads[using_thread].entry_sp);
#endif
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
#ifdef DEBUG_GDB
                printf("registers [eip] = 0x%lx\n",registers[PC]);
#endif            
            }else if (*ptr++ == 'g'){
            /*FIX IT*/
                //~ while (*ptr != '.')
                    //~ ptr++;
                //~ ptr++;
                hexToInt(&ptr, &addr);
                if (addr != -1 && addr != 0) 
                {
                    using_thread = addr;
                    using_thread --;
                    //~ strcpy(remcomOutBuffer,"OK");
                    //~ printf("\nH-1 break\n");
                    //~ break;
            
                }else using_thread = POK_SCHED_CURRENT_THREAD;
#ifdef DEBUG_GDB
                printf("pok_threads[%d].sp=0x%lx\n",using_thread,pok_threads[using_thread].sp);
                printf("pok_threads[%d].entry_sp=0x%lx\n",using_thread,pok_threads[using_thread].entry_sp);
                printf("POK_CONFIG_NB_THREADS = %d\n\n",POK_CONFIG_NB_THREADS);
                printf("MONITOR_THREAD = %d\n\n",MONITOR_THREAD);
                printf("POK_SCHED_CURRENT_THREAD = %d\n\n",POK_SCHED_CURRENT_THREAD);
#endif
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
#ifdef DEBUG_GDB
                printf("\nentry= 0x%lx\n",(uint32_t) pok_threads[using_thread].entry);
#endif
            }
#ifdef DEBUG_GDB
            printf("\nH\n");
#endif        
            strcpy(remcomOutBuffer,"OK");
            break;
        }
      /* cAA..AA    Continue at address AA..AA(optional) */
      /* sAA..AA   Step one instruction from AA..AA(optional) */
        case 's':
            stepping = TRUE;
        case 'k':       /* do nothing */
        case 'c':
            pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = old_entryS;
       
            set_regs(ea);

      /* try to read optional parameter, pc unchanged if no parm */
            if (hexToInt (&ptr, &addr))
                registers[PC] = addr;


      /* clear the trace bit */
            registers[PS] &= 0xfffffeff;
            ea->eflags = registers[PS];
      /* set the trace bit if we're stepping */
            if (stepping){
#ifdef DEBUG_GDB
                printf("\n\n\nStepping\n\n\n");
#endif
                stepping=FALSE;
                registers[PS] |= 0x100;
                ea->eflags = registers[PS];      
            }
            return;

    }           /* switch */

      /* reply to the request */
      putpacket ((unsigned char *)remcomOutBuffer);
#endif
    } /* while(1) */
    ////printf("\n\n\n          End of handle_exeption\n\n");
}