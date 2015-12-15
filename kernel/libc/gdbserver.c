#define PC_REGNUM 64
#define SP_REGNUM 1


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
 *  Because gdb will sometimes write to the stack area to execute function
 *  calls, this program cannot rely on using the supervisor stack so it
 *  uses it's own stack area reserved in the int array remcomStack.
 *
 *************
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
 *
 * All commands and responses are sent with a packet which includes a
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

#include <core/debug.h>
#include <config.h>


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


extern void putDebugChar( char );   /* write a single character      */
extern int getDebugChar();  /* read and return a single char */
extern void exceptionHandler(); /* assign an exception handler   */

void putDebugChar(char c){
    pok_cons_write_1(&c,1);
    pok_cons_write(&c,1);
}

int getDebugChar(){
    int inf = getchar2();
    printf("%c",inf);
    return inf;
}

/************************************************************************/
/* BUFMAX defines the maximum number of characters in inbound/outbound buffers*/
/* at least NUMREGBYTES*2 are needed for register packets */
#define BUFMAX 1000

static char initialized;  /* boolean flag. != 0 means we've been initialized */

int     remote_debug;
/*  debug >  0 prints ill-formed commands in valid packets & checksum errors */

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

#define STACKSIZE 10000
int remcomStack[STACKSIZE/sizeof(int)];
static int* stackPtr = &remcomStack[STACKSIZE/sizeof(int) - 1];


void
_returnFromException ()
{
  ////return_to_prog ();
}

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
    printf("Lets getpacket <---\n");
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
            if (checksum != xmitcsum)
                putDebugChar('-');  /* failed checksum */
            else {
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
    printf("\n");
}
#endif
#ifdef __i386__
unsigned char *
getpacket (void)
{
    printf("Lets getpacket <---\n");
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
    printf("\n");
}
#endif


/* send the packet in buffer.  */

void
putpacket (unsigned char *buffer)
{
    unsigned char checksum;
    int count;
    char ch;
    printf("\nLets putpacket --->\n");
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
        printf("\n");
}

void
debug_error (format, parm)
     char *format;
     char *parm;
{
    if (remote_debug)
        ;
    ////fprintf (stderr, format, parm);
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
            break;            /* debug exception */
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
hexToInt (char **ptr, int *intValue)
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

#define SPRN_PID        0x030   /* Process ID */

#define __stringify_1(x)        #x
#define __stringify(x)          __stringify_1(x)

#define mfspr(rn)       ({unsigned long rval; \
                                asm volatile("mfspr %0," __stringify(rn) \
                                                                    : "=r" (rval)); rval;})
#define mtspr(rn, v)    asm volatile("mtspr " __stringify(rn) ",%0" : \
                                             : "r" ((unsigned long)(v)) \
                                             : "memory")



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

int max_breakpoint = 100;
int b_need_to_delete = -1;
int b_need_to_set = -1;


struct T_breakpoint breakpoints[100];
int Head_of_breakpoints;
int last_breakpoint;
  
pok_partition_id_t give_part_num_of_thread(int thread_num){

    for (int i = 0; i < POK_CONFIG_NB_PARTITIONS; i++){
        if (pok_partitions[i].thread_index_high > thread_num) return i + 1;
    }
    return 0;
}    

void add_0_breakpoint(int * addr,int * length,int * using_thread){
    if (b_need_to_set == -1){
        strcpy (remcomOutBuffer, "OK");
        return;
    }
    b_need_to_set = -1;
    int old_pid = mfspr(SPRN_PID);
    int new_pid = give_part_num_of_thread(*using_thread + 1);
    printf("New_pid = %d\n",new_pid);
    printf("Old_pid = %d\n",old_pid);    
    if (*addr >= 0x80000000)
    { 
        printf("Load new_pid\n");
        mtspr(SPRN_PID, new_pid);
    }
    if (!mem2hex((char *)(*addr),&(breakpoints[Head_of_breakpoints].Instr),*length)){
        strcpy (remcomOutBuffer, "E25");
        mtspr(SPRN_PID, old_pid);    
        return;
    }
    last_breakpoint++;
    breakpoints[Head_of_breakpoints].T_num = *using_thread + 1;
    breakpoints[Head_of_breakpoints].P_num = new_pid;
    breakpoints[Head_of_breakpoints].B_num = last_breakpoint;
    breakpoints[Head_of_breakpoints].Reason = 2;
    Head_of_breakpoints++;
    if (Head_of_breakpoints == max_breakpoint){
        strcpy(remcomOutBuffer, "E25");
        mtspr(SPRN_PID, old_pid);    
        return;
    }
    if (hex2mem(trap, (char *)(*addr), *length)) {
        strcpy(remcomOutBuffer, "OK");
    } else {
        strcpy(remcomOutBuffer, "E25");
        mtspr(SPRN_PID, old_pid);    
        return;
    }
    if (*addr >= 0x80000000)
        mtspr(SPRN_PID, old_pid);    
}

void remove_0_breakpoint(int * addr,int * length,int * using_thread){
    if (b_need_to_delete == -1){
        strcpy (remcomOutBuffer, "OK");
        return;
    }
    int old_pid = mfspr(SPRN_PID);
    int new_pid = give_part_num_of_thread(*using_thread + 1);
    printf("New_pid = %d\n",new_pid);
    printf("Old_pid = %d\n",old_pid);    
    if (*addr >= 0x80000000)
    { 
        printf("Load new_pid\n");
        mtspr(SPRN_PID, new_pid);
    }
    int i = 0;
    for (i = 0; i < max_breakpoint; i++){
        if (breakpoints[i].B_num == b_need_to_delete)
            break;
    }
    b_need_to_delete = -1;
    if (i == (Head_of_breakpoints - 1)) 
        Head_of_breakpoints--;
    breakpoints[i].T_num = 0;
    breakpoints[i].P_num = 0;
    breakpoints[i].B_num = 0;
    breakpoints[i].Reason = 0;
    if (hex2mem(breakpoints[i].Instr, (char *)(*addr), *length)) {
        strcpy(remcomOutBuffer, "OK");
    } else {
        strcpy (remcomOutBuffer, "E25");
        mtspr(SPRN_PID, old_pid);    
        return;
    }
    if (*addr >= 0x80000000)
        mtspr(SPRN_PID, old_pid);    
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





/*
 * This function does all command procesing for interfacing to gdb.
 */
void
handle_exception (int exceptionVector, struct regs * ea)
{
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
    int addr, length;
    char *ptr;

    if (1 == 1)////remote_debug)
    {
      ////printf ("vector=%d, sr=0x%x, pc=0x%x\n",
      ////    exceptionVector, registers[pc], registers[pc]);
    }

    if (addr_instr != 0){
#ifdef __PPC__    
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
    int part_of_this_thread = 1;////give_part_num_of_thread(thread_num);
    *ptr++ = 'p';
    ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
    *ptr++ = '.';
    ptr = mem2hex( (char *)(&(thread_num)), ptr, 4); 
    *ptr++ = ';';


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
    thread_num = POK_SCHED_CURRENT_THREAD + 1;
    ptr = mem2hex( (char *)&thread_num, ptr, 1); 
    *ptr++ = ';';
  *ptr = '0';
#endif      
    putpacket ( (unsigned char *) remcomOutBuffer);
    
    while (1) {
#ifdef __PPC__
        remcomOutBuffer[0] = 0;

        getpacket(remcomInBuffer);
        switch (remcomInBuffer[0]) {

        case 'T':               /*Find out if the thread thread-id is alive*/
            ptr = &remcomInBuffer[1];
            int thread_num = -1;
            /*FIX IT*/
            while (* ptr != '.')
                ptr++;
            ptr++;
            hexToInt(&ptr, &thread_num);
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
                uint32_t p = POK_SCHED_CURRENT_THREAD + 1;
                ////TODO: Change number of process
                int part_of_this_thread = 1;////give_part_num_of_thread(p);
                *ptr++ = 'p';
                ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                *ptr++ = '.';
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
                hexToInt(&ptr, &part_id);
                ptr = remcomOutBuffer;
                //~ if (part_id <= POK_CONFIG_NB_PARTITIONS + 2 && part_id >= 1){
                    *ptr++ = '1';
                //~ }
                *ptr = 0;
                break;
            }
            if (strncmp(ptr, "Symbol::", 8) == 0){

                ptr = remcomOutBuffer;
                remcomOutBuffer[0] = 'O';
                remcomOutBuffer[1] = 'K';
                remcomOutBuffer[2] = 0;
                break;
            }            if (strncmp(ptr, "Supported", 9) == 0){
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
                break;
                
            }
            if (strncmp(ptr, "fThreadInfo", 11) == 0)   {
                number_of_thread = 1;
                ptr = remcomOutBuffer;  
                *ptr++ = 'm';
                int previous_thread = 1;
                ////TODO: Change number of process
                int part_of_this_thread = 1;////give_part_num_of_thread(previous_thread);
                *ptr++ = 'p';
                ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                *ptr++ = '.';
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
                ////TODO: Change number of process
                int part_of_this_thread = 1;////give_part_num_of_thread(previous_thread);
                *ptr++ = 'p';
                ptr = mem2hex( (char *)(&part_of_this_thread), ptr, 4); 
                *ptr++ = '.';
                ptr = mem2hex( (char *)(&previous_thread), ptr, 4); 
                number_of_thread++;
                *ptr++ = 0;
                break;
             }
             if (strncmp(ptr, "ThreadExtraInfo", 15) == 0){
                ptr += 16;
                int thread_num;
                /*FIX IT*/
                while (* ptr != '.')
                    ptr++;
                ptr++;
                hexToInt(&ptr, &thread_num);
                thread_num --;


                ptr = remcomOutBuffer;
                int info_offset = 0;
                int lengh = pok_thread_info(pok_threads[thread_num].state, &info_offset);
                if (thread_num == POK_SCHED_CURRENT_THREAD){
                    ptr = mem2hex( (char *) &("* "), ptr, 2);
                }
                
                //FIXME
                ptr = mem2hex( (char *) &("P"), ptr, 1);
                int pid = give_part_num_of_thread(thread_num);
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
        case 'd':
            /* toggle debug flag */
            ////kdebug ^= 1;
            break;

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
                /*FIX IT*/
                while (*ptr != '.')
                    ptr++;
                ptr++;
                hexToInt(&ptr, &addr);
                if (addr != -1 && addr != 0) 
                {
                    using_thread = addr;
                    using_thread --;
                    //~ strcpy(remcomOutBuffer,"OK");
                    //~ printf("\nH-1 break\n");
                    //~ break;
                
                }else using_thread = POK_SCHED_CURRENT_THREAD;
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
            }
            //~ printf("\nH\n");
            
            strcpy(remcomOutBuffer,"OK");
            break;

        case 'm':   /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
                /* Try to read %x,%x.  */
            {
                ptr = &remcomInBuffer[1];
                int old_pid = mfspr(SPRN_PID);
                int new_pid = give_part_num_of_thread(using_thread + 1);
                printf("New_pid = %d\n",new_pid);
                printf("Old_pid = %d\n",old_pid);
                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, &length)) {
                    if (addr >= 0x80000000 && new_pid == 0){
                        
                        strcpy (remcomOutBuffer, "E03");
                        break;
                    }
                    if (addr >= 0x80000000)
                    { 
                        printf("Load new_pid\n");
                        mtspr(SPRN_PID, new_pid);
                    }
                    if (mem2hex((char *)addr, remcomOutBuffer,length)){
                        mtspr(SPRN_PID, old_pid);
                        break;
                    }
                    strcpy (remcomOutBuffer, "E03");
                    if (addr >= 0x80000000)
                        mtspr(SPRN_PID, old_pid);
                } else {
                    strcpy(remcomOutBuffer,"E01");
                }
                break;
            }
        case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
            /* Try to read '%x,%x:'.  */
            {

                ptr = &remcomInBuffer[1];
                int old_pid = mfspr(SPRN_PID);
                int new_pid = give_part_num_of_thread(using_thread + 1);
                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, &length)
                    && *ptr++ == ':') {
                    if (addr >= 0x80000000)
                        mtspr(SPRN_PID, new_pid);
                    if (strncmp(ptr, "7d821008", 8) == 0)
                        ptr = trap;
                    if (hex2mem(ptr, (char *)addr, length)) {
                        strcpy(remcomOutBuffer, "OK");
                    } else {
                        strcpy(remcomOutBuffer, "E03");
                    }
                    if (addr >= 0x80000000)
                        mtspr(SPRN_PID, old_pid);
                } else {
                    strcpy(remcomOutBuffer, "E02");
                }
                break;
            }

        case 'D':               /*
                                 * The first form of the packet is used to detach gdb from the remote system
                                 * It is sent to the remote target before gdb disconnects via the detach command.
                                 */
            ptr = &remcomInBuffer[2];
            int part_id;
            hexToInt(&ptr, &part_id);
            remcomOutBuffer[0] = 'O';
            remcomOutBuffer[1] = 'K';
            remcomOutBuffer[2] = 0;
            
            if (part_id != 1) break;
            putpacket((unsigned char *)remcomOutBuffer);            
        case 'k':    /* kill the program, actually just continue */
        case 'c':    /* cAA..AA  Continue; address AA..AA optional */
            /* try to read optional parameter, pc unchanged if no parm */

            pok_threads[POK_SCHED_CURRENT_THREAD].entry_sp = old_entryS;
       
            set_regs(ea);
            ptr = &remcomInBuffer[1];
            if (hexToInt(&ptr, &addr)) {
                registers[pc]/*nip*/ = addr;
            }
        
/* Need to flush the instruction cache here, as we may have deposited a
 * breakpoint, and the icache probably has no way of knowing that a data ref to
 * some location may have changed something that is in the instruction cache.
 */
////            kgdb_flush_cache_all();
////            set_msr(msr);
////            kgdb_interruptible(1);
////            unlock_kernel();
////            kgdb_active = 0;
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
           
                ////flush_icache_range(addr, addr+length);

                

////            kgdb_flush_cache_all();
////            registers[msr] |= MSR_SE;
            
////#if 0
////            set_msr(registers[msr]);
////#endif
////            unlock_kernel();
////            kgdb_active = 0;
            return;
        }
        case 'r':       /* Reset (if user process..exit ???)*/
////            panic("kgdb reset.");
            break;
        }           /* switch */
        if (remcomOutBuffer[0] && 0 /*kdebug*/) {
            ////printf("remcomInBuffer: %s\n", remcomInBuffer);
            ////printf("remcomOutBuffer: %s\n", remcomOutBuffer);
        }
        /* reply to the request */
        putpacket((unsigned char *)remcomOutBuffer);
#endif
#ifdef __i386__
    remcomOutBuffer[0] = 0;
    ptr = (char *)getpacket ();

    switch (*ptr++)
    {
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
                printf("in first if\n");
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
                printf("thread_num=%d\n",thread_num);
                printf("pok_threads[%d].state=%d\n",thread_num,pok_threads[thread_num].state);
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
                printf("lengh = %d\n",lengh);
                printf("info_offset = %d\n",info_offset);
                printf("%c%c%c\n",info_thread[info_offset],info_thread[info_offset+1],info_thread[info_offset+2]);
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
        case 'd':
            remote_debug = !(remote_debug); /* toggle debug flag */
            break;
        case 'g':       /* return the value of the CPU registers */
            mem2hex ((char *) registers, remcomOutBuffer, NUMREGBYTES, 0);
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

      /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
        case 'm':
      /* TRY TO READ %x,%x.  IF SUCCEED, SET PTR = 0 */
            if (hexToInt (&ptr, &addr))
                if (*(ptr++) == ',')
                    if (hexToInt (&ptr, &length))
                    {
                        ptr = 0;
                        mem_err = 0;
                        mem2hex ((char *) addr, remcomOutBuffer, length, 1);
                        if (mem_err)
                        {
                            strcpy (remcomOutBuffer, "E03");
                            debug_error ("memory fault");
                        }
                    }

            if (ptr)
            {
                strcpy (remcomOutBuffer, "E01");
            }
            break;

      /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
        case 'M':
      /* TRY TO READ '%x,%x:'.  IF SUCCEED, SET PTR = 0 */
            if (hexToInt (&ptr, &addr))
                if (*(ptr++) == ',')
                    if (hexToInt (&ptr, &length))
                        if (*(ptr++) == ':')
                        {
                            mem_err = 0;
                            hex2mem (ptr, (char *) addr, length, 1);
                            if (mem_err)
                            {
                                strcpy (remcomOutBuffer, "E03");
                                debug_error ("memory fault");
                            }else{
                                strcpy (remcomOutBuffer, "OK");
                            }

                            ptr = 0;
                        }
            if (ptr)
            {
                strcpy (remcomOutBuffer, "E02");
            }
            break;

        case 'H':                   /*Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
        {
            if (number_of_thread == 1){
            //TODO: FIX IT
                strcpy(remcomOutBuffer,"OK");
                printf("\nH break\n");
                break;
            }    
            ptr = &remcomInBuffer[1];
            if ( *ptr == 'c'){
                using_thread = POK_SCHED_CURRENT_THREAD;
                printf("pok_threads[%d].sp=0x%lx\n",using_thread,pok_threads[using_thread].sp);
                printf("pok_threads[%d].entry_sp=0x%lx\n",using_thread,pok_threads[using_thread].entry_sp);
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
                printf("registers [eip] = 0x%lx\n",registers[PC]);
            
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
                printf("pok_threads[%d].sp=0x%lx\n",using_thread,pok_threads[using_thread].sp);
                printf("pok_threads[%d].entry_sp=0x%lx\n",using_thread,pok_threads[using_thread].entry_sp);
                printf("POK_CONFIG_NB_THREADS = %d\n\n",POK_CONFIG_NB_THREADS);
                printf("MONITOR_THREAD = %d\n\n",MONITOR_THREAD);
                printf("POK_SCHED_CURRENT_THREAD = %d\n\n",POK_SCHED_CURRENT_THREAD);
                set_regs((struct regs *)pok_threads[using_thread].entry_sp);
                printf("\nentry= 0x%lx\n",(uint32_t) pok_threads[using_thread].entry);
            }
            printf("\nH\n");
        
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
                printf("\n\n\nStepping\n\n\n");
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

/* this function is used to set up exception handlers for tracing and
   breakpoints */
void
set_debug_traps (void)
{
  stackPtr = &remcomStack[STACKSIZE / sizeof (int) - 1];

 //// exceptionHandler (0, _catchException0);
 //// exceptionHandler (1, _catchException1);
 //// exceptionHandler (3, _catchException3);
 //// exceptionHandler (4, _catchException4);
 //// exceptionHandler (5, _catchException5);
 //// exceptionHandler (6, _catchException6);
 //// exceptionHandler (7, _catchException7);
 //// exceptionHandler (8, _catchException8);
 //// exceptionHandler (9, _catchException9);
 //// exceptionHandler (10, _catchException10);
 //// exceptionHandler (11, _catchException11);
 //// exceptionHandler (12, _catchException12);
 //// exceptionHandler (13, _catchException13);
 //// exceptionHandler (14, _catchException14);
 //// exceptionHandler (16, _catchException16);

  initialized = 1;
}

/* This function will generate a breakpoint exception.  It is used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

void
breakpoint (void)
{
  ////if (initialized)
    ////BREAKPOINT ();
}
