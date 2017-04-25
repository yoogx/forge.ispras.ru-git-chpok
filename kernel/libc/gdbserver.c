#include <config.h>

#ifdef POK_NEEDS_GDB

#define PC_REGNUM 64
#define SP_REGNUM 1


#define MULTIPROCESS

/*
 * For e500mc: ~/qemu/ppc-softmmu/qemu-system-ppc -serial stdio -serial pty -M ppce500 -cpu e500mc -kernel pok.elf
 *
 * For i386:  qemu-system-i386 -serial stdio -serial pty -kernel pok.elf
 *
 */

#include <core/partition_arinc.h>
#include <core/uaccess.h>

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
#include <core/thread.h>
#include <core/partition.h>

#include <core/time.h>
#include <core/debug.h>
#include <config.h>
#include <cons.h>

#include <gdb.h>

/***********************************************************************
 * 
 * Threading model of the POK.
 * 
 * For each kernel-only partition with (relative) index K there is thread
 * 1.(K+1)
 * 
 * For each partition with access to user space (space_id is not 0)
 * with (relative) index U there is inferior
 * (U+1). For each thread with index T within such partition there is
 * GDB thread (U+1).(T+1)
 * 
 */
struct gdb_thread
{
    int inferior_id; // count from 1
    int thread_id; // count from 1
    // model-specific data
    pok_partition_t* part;
    void* part_private;
};

/* Currently executed thread */
static struct gdb_thread gdb_thread_current;

/* Whether t1 and t2 actually refers to the same thread. */
static inline pok_bool_t gdb_thread_equal(const struct gdb_thread* t1,
    const struct gdb_thread* t2)
{
    pok_bool_t ret = (t1->inferior_id == t2->inferior_id)
        && (t1->thread_id == t2->thread_id);

    return ret;
}

/* Copy src thread into dest. */
static inline void gdb_thread_copy(struct gdb_thread* dest,
    const struct gdb_thread* src)
{
    *dest = *src;
}

/* Variables used in callbacks for 'for_each_partition'.*/

/* Index of the partition. */
int _partition_index;
/* Partition at given index. */
pok_partition_t* _partition_at_index;
/* Index of currently iterated partition. */
int _partition_index_current;
/* Whether we enumerate kernel-only partitions or user-space ones. */
pok_bool_t _partition_is_kernel;

/* 
 * Callback for 'for_each_partition'.
 * 
 * If current partition's index is '_partition_index', set
 * '_partition_found' variable.
 */
static void _get_partition_at_index_cb(pok_partition_t* part)
{
    if(_partition_is_kernel) {
        if(part->space_id != 0) return;
    }
    else {
        if(part->space_id == 0) return;
    }

    if(_partition_index_current == _partition_index)
        _partition_at_index = part;

    _partition_index_current++;
}

/*
 * Return partition at given index.
 *
 * Return NULL if not found.
 */
static pok_partition_t* get_partition_at_index(uint8_t index,
    pok_bool_t is_kernel)
{
    _partition_is_kernel = is_kernel;
    _partition_index = index;
    _partition_index_current = 0;
    _partition_at_index = NULL;

    for_each_partition(_get_partition_at_index_cb);

    return _partition_at_index;
}

/*
 * If 't.inferior_id' and 't.thread_id' are both zero, fill 't' with
 * the first thread.
 *
 * Otherwise fill 't' with next thread.
 *
 * Return 0 on success, non-zero on EOF.
 *
 * Firstly enumerate all user-space threads (inferior_id = 2,3,...),
 * then enumerate kernel space threads (inferior_id = 1).
 * This will overcome gdb(?) problem, when it forget all inferiors
 * except the last listed one, so it cannot find address space for
 * current (GDB, kernel) thread.
 */
int gdb_thread_get_next(struct gdb_thread* t)
{
    t->thread_id++;

    if(t->inferior_id == 0) // Just begins
    {
        t->inferior_id = 1; // Fake, will be 2 after "goto"
        goto next_inferior;
    }


    if(t->inferior_id >= 2) goto user_thread;
    else goto kernel_thread;

user_thread:
    if(t->thread_id > t->part->part_sched_ops->get_number_of_threads(t->part))
        goto next_inferior;

    t->part->part_sched_ops->get_thread_at_index(t->part, 0, &t->part_private);

    return 0;

kernel_thread:
    t->part = get_partition_at_index(t->thread_id - 1, TRUE);
    if(!t->part) return 1; // EOF

    assert(t->part->part_sched_ops->get_number_of_threads(t->part) == 1);
    t->part->part_sched_ops->get_thread_at_index(t->part, 0, &t->part_private);

    return 0;

next_inferior:
    t->inferior_id++;
    t->thread_id = 1;
    t->part = get_partition_at_index(t->inferior_id - 2, FALSE);
    if(!t->part)
    {
        t->inferior_id = 1;
        t->thread_id = 1;
        goto kernel_thread;
    }

    t->part->part_sched_ops->get_thread_at_index(t->part, 0, &t->part_private);

    return 0;
}

/* 
 * Fill given `gdb_thread` structure with thread corresponded to
 * '.inferior_id' and '.thread_id' set for given structure.
 * 
 * Return 0 on success, negative value on fail.
 */
int gdb_thread_find(struct gdb_thread* t)
{
    if(t->inferior_id == 1)
    {
        t->part = get_partition_at_index(t->thread_id - 1, TRUE);
        if(!t->part) return 1;
        t->part->part_sched_ops->get_thread_at_index(t->part, 0, &t->part_private);
    }
    else
    {
        t->part = get_partition_at_index(t->inferior_id - 2, FALSE);
        if(!t->part) return 1;
        if(t->thread_id > t->part->part_sched_ops->get_number_of_threads(t->part)) return 1;

        t->part->part_sched_ops->get_thread_at_index(t->part, t->thread_id - 1, &t->part_private);
    }

    return 0;
}

/* 
 * Callback for determine index of the partition.
 */
void _get_partition_index_cb(pok_partition_t* part)
{
    if(_partition_is_kernel) {
        if(part->space_id != 0) return;
    }
    else {
        if(part->space_id == 0) return;
    }

    if(part == _partition_at_index)
        _partition_index = _partition_index_current;

    _partition_index_current++;
}

int get_partition_index(pok_partition_t* part)
{
    assert(part);

    _partition_is_kernel = (part->space_id == 0);
    _partition_at_index = current_partition;
    _partition_index = -1;
    _partition_index_current = 0;

    for_each_partition(_get_partition_index_cb);

    assert(_partition_index != -1);

    return _partition_index;
}

/* Update current thread. */
void gdb_thread_get_current(void)
{
    struct gdb_thread* t = &gdb_thread_current;
    t->part = current_partition;
    if(t->part->space_id == 0)
    {
        t->inferior_id = 1;
        t->thread_id = get_partition_index(t->part) + 1;
        t->part->part_sched_ops->get_thread_at_index(t->part, 0, &t->part_private);
    }
    else
    {
        t->inferior_id = get_partition_index(t->part) + 2;
        t->thread_id = t->part->part_sched_ops->get_current_thread_index(t->part) + 1;
        t->part->part_sched_ops->get_thread_at_index(t->part, t->thread_id - 1, &t->part_private);
    }
}

/*
 * Return (generic) partition for given thread.
 */
static pok_partition_t* gdb_thread_get_partition(const struct gdb_thread* t)
{
    return t->part;
}

/*
 * Return space identificator for given thread.
 */
static uint8_t gdb_thread_get_space(const struct gdb_thread* t)
{
    return t->part->space_id;
}


/*
 * Return address which can be used by GDB for
 * r/w from/to given memory.
 * 
 * Access is garanteed only if switched to thread's space.
 * 
 * If given area doesn't belong to the given thread or cannot be written, returns NULL.
 * 
 * TODO: Access is garanteed only under effect of jet_uspace_grant_access_debug().
 */
uintptr_t gdb_thread_write_addr(const struct gdb_thread* t, uintptr_t addr,
    size_t size)
{
    return (uintptr_t)jet_addr_to_gdb((void*)addr, size, gdb_thread_get_partition(t));
}

uintptr_t gdb_thread_read_addr(const struct gdb_thread* t, uintptr_t addr,
    size_t size)
{
    return (uintptr_t)jet_addr_to_gdb_ro((const void*)addr, size, gdb_thread_get_partition(t));
}

/* Print name of the thread using given callback. */
void gdb_thread_print_name(const struct gdb_thread* t,
    print_cb_t print_cb, void* cb_data)
{
#define WRITE_STR(s) print_cb(s, strlen(s), cb_data)
    if(gdb_thread_equal(&gdb_thread_current, t))
        WRITE_STR("* ");

    size_t name_len = strnlen(t->part->name, MAX_NAME_LENGTH);
    print_cb(t->part->name, name_len, cb_data);

    WRITE_STR(".");

    t->part->part_sched_ops->get_thread_info(t->part, t->thread_id - 1,
        t->part_private, print_cb, cb_data);
#undef WRITE_STR
}

/* 
 * Return registers(arch-specific) for non-current thread.
 */
static struct jet_interrupt_context* gdb_thread_get_regs(const struct gdb_thread* t)
{
    if(t->inferior_id == 1)
    {
        /* Kernel threads always current for partition. */
        return t->part->entry_sp;
    }

    int current_thread_id = t->part->part_sched_ops->get_current_thread_index(t->part) + 1;

    if(current_thread_id == t->thread_id)
    {
        return t->part->entry_sp;
    }
    else
    {
        return t->part->part_sched_ops->get_thread_registers(t->part, t->thread_id - 1,
            t->part_private);
    }
}

#ifdef NUMREGS_FP
/* 
 * Return floating-point registers(arch-specific) for non-current thread.
 */
void gdb_thread_get_fp_regs(const struct gdb_thread* t, uint32_t* fp_registers)
{
    (void) t;
    memset(fp_registers, 0, NUMREGS_FP * sizeof(unsigned long));
}

/* 
 * Set floating-point registers(arch-specific) for non-current thread.
 */
void gdb_thread_set_fp_regs(struct gdb_thread* t, const uint32_t* registers)
{
    (void) t;
    (void) registers;
    //TODO: Currently floating point registers are not stored.
}
#endif

/************************************************************************
 *
 * external low-level support routines
 */

char *gdb_strcpy(char *dest, const char *str)
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

/*
 * these should not be static cuz they can be used outside this module
 */
static uint32_t registers[NUMREGS];

#ifdef NUMREGS_FP
static uint32_t fp_registers[NUMREGS_FP];
#endif

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

/* scan for the sequence $<data>#<checksum>     */
static void
getpacket(char *buffer)
{
    data_to_read_1();
    printf_GDB("Lets getpacket <---\n");
    unsigned char checksum;
    unsigned char xmitcsum;
    int i;
    int count;
    unsigned char ch;

    do {
        /* wait around for the start character, ignore all other
         * characters 
         */
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
    printf_GDB("\n");
}


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
    printf_GDB("Buffered string:\n  %s\n", string);
    st_idx = 0;
    printf_GDB("\nLets putpacket --->\n");
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
        printf_GDB("\n");
}


/* Address of a routine to RTE to if we get a memory fault.  */
static void (*volatile mem_fault_routine) () = NULL;


/*
 * These are separate functions so that they are so short and sweet
 * that the compiler won't save any registers (if there is a fault
 * to mem_fault, they won't get restored, so there better not be any
 * saved).  
 */
int
get_char (const char *addr)
{
    return *addr;
}

void
set_char (char *addr, int val)
{
    *addr = val;
/*
 * Instructions sync
 */
    GDB_INSTR_SYNC(addr);
}
/*
 * Convert the memory pointed to by mem into hex, placing result in buf 
 * return a pointer to the last char put in buf (null) 
 */
char * 
mem2hex (const char *mem, char *buf, int count)
{
    int i;
    unsigned char ch;

    for (i = 0; i < count; i++)
    {
        ch = get_char (mem++);
        *buf++ = hexchars[ch >> 4];
        *buf++ = hexchars[ch % 16];
    }
    *buf = 0;
    return (buf);
}

/*
 * Convert the hex array pointed to by buf into binary to be placed in mem.
 * Return a pointer to the character AFTER the last byte written.
 */
char *
hex2mem (char *buf, char *mem, int count)
{
    int i;
    unsigned char ch;

    for (i = 0; i < count; i++)
    {
        ch = hex (*buf++) << 4;
        ch = ch + hex (*buf++);
        set_char (mem++, ch);
    }
    return (mem);
}

/*
 * This function takes the 386 exception vector and attempts to
 * translate this number into a unix compatible signal value 
 */
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
        case 17:              /* SIGINT  */
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

static char * trap;

#define max_breakpoints  20
int max_breakpoint = max_breakpoints;
int b_need_to_delete = -1;
int Connect_to_new_inferior = -1;


struct T_breakpoint breakpoints[max_breakpoints];
int Head_of_breakpoints;
int last_breakpoint;



/*
 * Helper: writes identificator of the thread into output packet.
 */

static char* write_thread_id(char* ptr, const struct gdb_thread* t)
{
#ifdef MULTIPROCESS
    *ptr++ = 'p';
    ptr = mem2hex( (char *)(&t->inferior_id), ptr, GDB_INSTR_SIZE);
    *ptr++ = '.';
#endif
    ptr = mem2hex( (char *)(&t->thread_id), ptr, GDB_INSTR_SIZE);
    return ptr;
}


/*
 * Helper: reads identificator of the thread from input packet.
 */
static char* read_thread_id(char* ptr, struct gdb_thread* t)
{
#ifdef MULTIPROCESS
    assert(*ptr == 'p');
    ptr++;
    hexToInt(&ptr, (uintptr_t *)(&t->inferior_id));
    assert(*ptr == '.');
    ptr++;
#endif
    hexToInt(&ptr, (uintptr_t *)(&t->thread_id));

    return ptr;
}

struct packet_write_cb_data
{
    char* ptr;
    // TODO: Maximum length should be there
};

/*
 * Callback of type print_cb_t for write into gdb packet.
 */
void packet_write_cb(const void* data, size_t len, void* cb_data)
{
    struct packet_write_cb_data* cb_data_real = cb_data;

    cb_data_real->ptr = mem2hex(data, cb_data_real->ptr, len);
}


/* State of gdb server while stopped. */
struct gdb_state
{
    struct gdb_thread t; // processed thread

    struct jet_interrupt_context* current_ea; // These regs should be used when processed thread is actually the current one.
};

/*
 * Initialize cache of the thread.
 *
 * Initially processed thread is current one.
 */
static void gdb_state_init(struct gdb_state* tc,
    struct jet_interrupt_context* current_ea)
{
    gdb_thread_copy(&tc->t, &gdb_thread_current);

    tc->current_ea = current_ea;
}

/* Make processed thread state uptodate. */
static void gdb_state_store_registers(struct gdb_state* tc)
{
    struct jet_interrupt_context* ea;

    if(!gdb_thread_equal(&tc->t, &gdb_thread_current))
    {
        ea = gdb_thread_get_regs(&tc->t);

#ifdef NUMREGS_FP
        gdb_thread_set_fp_regs(&tc->t, fp_registers);
#endif
    }
    else
    {
        ea = tc->current_ea;
    }
    //~ printf("Flush registers. pc = 0x%lx\n", registers[pc]);

    gdb_get_regs(ea, registers);
}

/* Make exposed registers for processed state up-to-date. */
static void gdb_state_fill_registers(struct gdb_state* tc)
{
    struct jet_interrupt_context* ea;

    if(!gdb_thread_equal(&tc->t, &gdb_thread_current))
    {
        ea = gdb_thread_get_regs(&tc->t);

#ifdef NUMREGS_FP
        gdb_thread_get_fp_regs(&tc->t, fp_registers);
#endif
    }
    else
    {
        ea = tc->current_ea;
    }

    if(ea)
        gdb_set_regs(ea, registers);
}

/*
 * Set given thread as processed.
 *
 * Previously processed thread is flushed if needed.
 */
static void gdb_sate_set_thread(struct gdb_state* tc,
    struct gdb_thread* t)
{
    if(gdb_thread_equal(&tc->t, t)) return;

    gdb_thread_copy(&tc->t, t);
}



int new_start;

void clear_breakpoints(){
    for (int i = 0; i < max_breakpoint; i++)
        breakpoints[i].addr = 0;
}



/*
 *  Added watchpoint.
 *  Types:   2 -  Write watchpoint
 *           3 -   Read watchpoint
 *           4 - Access watchpoint
 */

//Work only on bared metal p3041

void add_watchpoint(uintptr_t addr, int length, const struct gdb_thread* t, int type)
{
    GDB_ADD_WATCHPOINT(addr, length, type, gdb_thread_get_regs(t), remcomOutBuffer);
}

//Work only on bared metal p3041

void remove_watchpoint(uintptr_t addr, int length, const struct gdb_thread* t, int type){
    GDB_REMOVE_WATCHPOINT(addr, length, type, gdb_thread_get_regs(t), remcomOutBuffer);   
}

void add_0_breakpoint(uintptr_t addr, int length, const struct gdb_thread* t){
    uint8_t old_space_id = pok_space_get_current();

    uintptr_t gdb_addr = gdb_thread_write_addr(t, addr, length);

    if(!gdb_addr) goto err;

    uint8_t space_id = gdb_thread_get_space(t);
    pok_space_switch(space_id);

    int i;
    for (i = 0; i < max_breakpoint; i++){
        if ((breakpoints[i].P_num == 0) && (breakpoints[i].B_num == 0) && (breakpoints[i].T_num == 0))
            break;
    }
    if (i == max_breakpoint) goto err;

    Head_of_breakpoints = i;
    last_breakpoint ++;
    breakpoints[Head_of_breakpoints].T_num = t->thread_id;
    breakpoints[Head_of_breakpoints].P_num = t->inferior_id;
    breakpoints[Head_of_breakpoints].B_num = last_breakpoint;
    breakpoints[Head_of_breakpoints].Reason = 2;
    breakpoints[Head_of_breakpoints].addr = gdb_addr;
    if (!mem2hex((char *)gdb_addr, (char *)&(breakpoints[Head_of_breakpoints].Instr), length)) goto err;

    if (!hex2mem(trap, (char *)gdb_addr, length)) goto err;

    gdb_strcpy(remcomOutBuffer, "OK");

    pok_space_switch(old_space_id);
    return;

err:
    pok_space_switch(old_space_id);
    gdb_strcpy(remcomOutBuffer, "E22");
    return;
}

void remove_0_breakpoint(uintptr_t addr, int length, const struct gdb_thread* t){
    int old_pid = pok_space_get_current();
    uintptr_t gdb_addr = gdb_thread_write_addr(t, addr, length);
    if(!gdb_addr) goto err;

    int i = 0;
    int new_pid = gdb_thread_get_space(t);

    printf_GDB("New_pid = %d\n",new_pid);
    printf_GDB("Old_pid = %d\n",old_pid);    

    pok_space_switch(new_pid);


    for (i = 0; i < max_breakpoint; i++){
        if ((breakpoints[i].addr == addr) && (breakpoints[i].P_num == t->inferior_id))
            break;
    }
    if (i == max_breakpoint) goto err;

    b_need_to_delete = -1;
    breakpoints[i].T_num = 0;
    breakpoints[i].P_num = 0;
    breakpoints[i].B_num = 0;
    breakpoints[i].Reason = 0;
    breakpoints[i].addr = 0;

    if (!hex2mem(breakpoints[i].Instr, (char *)gdb_addr, length)) goto err;

    gdb_strcpy(remcomOutBuffer, "OK");

    pok_space_switch(old_pid);
    return;
err:
    gdb_strcpy (remcomOutBuffer, "E22");
    pok_space_switch(old_pid);
}


/*
 * This function does all command procesing for interfacing to gdb.
 */
void
handle_exception (int exceptionVector, struct jet_interrupt_context* ea)
{
    Connect_to_new_inferior = -1;
    trap = GDB_ALLOC_TRAP();
    gdb_thread_get_current();
    // Cached thread for current operation.
    struct gdb_state tc;
    gdb_state_init(&tc, ea);
    // Temporary thread
    struct gdb_thread t;

    memset(remcomOutBuffer, 0, BUFMAX);
    memset(remcomInBuffer, 0, BUFMAX);
    int sigval;
    uintptr_t addr;
    int length;
    char *ptr;

    /*
     * Resore instructions if single step was used
     */
    GDB_RESTORE_INSTR(ea);

  /* reply to host that an exception has occurred */
    sigval = computeSignal (exceptionVector);

    ptr = remcomOutBuffer;

    gdb_state_fill_registers(&tc);
    ptr = GDB_SEND_FIRST_PACKAGE(ptr, sigval, registers);
    *ptr++ = 't';
    *ptr++ = 'h';
    *ptr++ = 'r';
    *ptr++ = 'e';
    *ptr++ = 'a';
    *ptr++ = 'd';
    *ptr++ = ':';

    ptr = write_thread_id(ptr, &gdb_thread_current);
    *ptr++ = ';';
#ifdef __PPC__

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
        ptr = mem2hex( (char *)&addr, ptr, 4); 
        *ptr++ = ';';
    }        
#endif

    ptr = 0;
#endif
    data_to_read_1();
    //~ int flag = 0;
    putpacket ( (unsigned char *) remcomOutBuffer);

    
    while (1) {
    data_to_read_1();

    remcomOutBuffer[0] = 0;

    getpacket(remcomInBuffer);

    switch (remcomInBuffer[0]) {
        case 'T':               /*Find out if the thread thread-id is alive*/
            ptr = &remcomInBuffer[1];
            ptr = read_thread_id(ptr, &t);

            if(!gdb_thread_find(&t)) {
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

                    printf_GDB("New_start = %d",new_start);

                    new_start = 0;

                    printf_GDB("New_start = %d",new_start);

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
                            add_0_breakpoint(addr, kind, &tc.t);
                        else
                            remove_0_breakpoint(addr, kind, &tc.t);
                }
                if (type == 2){
                    if (remcomInBuffer[0] == 'Z') 
                            add_watchpoint(addr, kind, &tc.t, 2);
                        else
                            remove_watchpoint(addr, kind, &tc.t, 2);
                }
                if (type == 3){
                    if (remcomInBuffer[0] == 'Z') 
                            add_watchpoint(addr, kind, &tc.t, 3);
                        else
                            remove_watchpoint(addr, kind, &tc.t, 3);
                }
                if (type == 4){
                    if (remcomInBuffer[0] == 'Z') 
                            add_watchpoint(addr, kind, &tc.t, 4);
                        else
                            remove_watchpoint(addr, kind, &tc.t, 4);
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

                ptr = write_thread_id(ptr, &gdb_thread_current);
                *ptr++ = 0;

                if (Connect_to_new_inferior == 1){
                    Connect_to_new_inferior = -1;
                    putpacket ( (unsigned char *) remcomOutBuffer);
               
                    ptr = &remcomInBuffer[1];
                    if (hexToInt(&ptr, &addr)) {
                        //ea->srr0 = addr;
                    }
 
                    GDB_DECREASE_PC(ea);
                    return;
                }

                break;
            }
            if (strncmp(ptr, "Offsets", 7) == 0){
                ptr = remcomOutBuffer;
                *ptr++ = 0;
                break;
            }
            if (strncmp(ptr, "Attached:", 9) == 0){
                ptr += 9;
                int part_id;
                hexToInt(&ptr, (uintptr_t *)(&part_id));
                ptr = remcomOutBuffer;
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
                t.inferior_id = 0;
                t.thread_id = 0;

                ptr = remcomOutBuffer;
                *ptr++ = 'm';

                gdb_thread_get_next(&t);

                ptr = write_thread_id(ptr, &t);
                *ptr++ = 0;

                break;
            }
            if (strncmp(ptr, "sThreadInfo", 11) == 0){
                ptr = remcomOutBuffer;

                if(gdb_thread_get_next(&t))
                {
                    *ptr++ = 'l';
                    *ptr++ = 0;
                    break;
                }

                *ptr++ = 'm';
                ptr = write_thread_id(ptr, &t);
                *ptr++ = 0;
                break;
             }
             if (strncmp(ptr, "ThreadExtraInfo", 15) == 0){
                ptr += 16;

                ptr = read_thread_id(ptr, &t);
                gdb_thread_find(&t);

                ptr = remcomOutBuffer;

                struct packet_write_cb_data cb_data = {.ptr = ptr};

                gdb_thread_print_name(&t, packet_write_cb, &cb_data);

                ptr = cb_data.ptr;

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
            gdb_state_fill_registers(&tc);
            /*
             * If the stack pointer has moved, you should pray.
             * (cause only god can help you).
             */
#ifdef NUMREGS_FP
            ptr = GDB_READ_REGS(ptr, registers, fp_registers);
#else
            ptr = GDB_READ_REGS(ptr, registers);
#endif
            break;
        }

        case 'G':   /* set the value of the CPU registers */
        {
            ptr = &remcomInBuffer[1];
#ifdef NUMREGS_FP
            ptr = GDB_WRITE_REGS(ptr, registers, fp_registers);
#else
            ptr = GDB_WRITE_REGS(ptr, registers);
#endif
            gdb_state_store_registers(&tc);


            gdb_strcpy(remcomOutBuffer,"OK");
            }
            break;
        case 'H':                   /*Set thread for subsequent operations (‘m’, ‘M’, ‘g’, ‘G’, et.al.). */
            ptr = &remcomInBuffer[1];
            if ( *ptr == 'c'){
                // Ignore thread id and assume it to be current one.
                gdb_strcpy(remcomOutBuffer, "OK");
            }else if (*ptr == 'g'){
                ptr++;
                ptr = read_thread_id(ptr, &t);
                
                // "Any" choice assume the current.
                // TODO: What means "all" choice in that case?
                if(t.inferior_id == -1 || t.inferior_id == 0) {
                    gdb_sate_set_thread(&tc, &gdb_thread_current);
                    gdb_strcpy(remcomOutBuffer,"OK");
                }

                else if(gdb_thread_find(&t)) {
                    gdb_strcpy(remcomOutBuffer,"E22");
                }else {
                    gdb_sate_set_thread(&tc, &t);
                    gdb_strcpy(remcomOutBuffer,"OK");
                }
            }
            break;

        case 'm':   /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
                /* Try to read %x,%x.  */
            {
                ptr = &remcomInBuffer[1];
                int old_pid = pok_space_get_current();
                int new_pid = gdb_thread_get_space(&tc.t);

                printf_GDB("New_pid = %d\n",new_pid);
                printf_GDB("Old_pid = %d\n",old_pid);

                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, (uintptr_t *)(&length))) {
                    uintptr_t gdb_addr = gdb_thread_read_addr(&tc.t, addr, length);
                    if (!gdb_addr){
                        gdb_strcpy (remcomOutBuffer, "E03");
                        break;
                    }else{ 

                        printf_GDB("Load new_pid\n");

                        pok_space_switch(new_pid);
                    }
                    if (!mem2hex((char *)gdb_addr, remcomOutBuffer,length)){
                        gdb_strcpy (remcomOutBuffer, "E03");
                    }
                    pok_space_switch(old_pid);
                } else {
                    gdb_strcpy(remcomOutBuffer,"E01");
                }
                break;
            }
        case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
            /* Try to read '%x,%x:'.  */
            {
                ptr = &remcomInBuffer[1];
                int old_pid = pok_space_get_current();
                int new_pid = gdb_thread_get_space(&tc.t);

                if (hexToInt(&ptr, &addr)
                    && *ptr++ == ','
                    && hexToInt(&ptr, (uintptr_t *)(&length))
                    && *ptr++ == ':') {

                    uintptr_t gdb_addr = gdb_thread_write_addr(&tc.t, addr, length);
                    if (!gdb_addr) {
                        gdb_strcpy (remcomOutBuffer, "E03");
                        break;
                    }

                    pok_space_switch(new_pid);
                    if (strncmp(ptr, "7d821008", 8) == 0) // TODO: Magic constant
                        ptr = trap;
                    if (hex2mem(ptr, (char *)gdb_addr, length)) {
                        gdb_strcpy(remcomOutBuffer, "OK");
                    } else {
                        gdb_strcpy(remcomOutBuffer, "E03");
                    }
                    pok_space_switch(old_pid);
                } else {
                    gdb_strcpy(remcomOutBuffer, "E02");
                }
                break;
            }

#ifdef MULTIPROCESS
        case 'v':               /*
                                 * Packets starting with ‘v’ are identified by a multi-letter name, up to the first ‘;’ or ‘?’ (or the end of the packet). 
                                 */
            {

            printf_GDB("IN V\n");
            printf_GDB("String = %s, %d\n", remcomOutBuffer, remcomOutBuffer[0]);

            ptr = &remcomInBuffer[1];
            if (strncmp(ptr, "Attach;", 7) == 0)   {

                printf_GDB("Added reply\n");

                gdb_strcpy(remcomOutBuffer, "Any stop packet");
            }
            if (strncmp(ptr, "Cont;c", 6) == 0) {
                ptr = &remcomInBuffer[1];
                if (hexToInt(&ptr, &addr)) {
                    registers[pc] = addr;

                }
                return;
            }         
            Connect_to_new_inferior = 1;

            break;
        }
        
        case 'D':               /*
                                 * The first form of the packet is used to detach gdb from the remote system
                                 * It is sent to the remote target before gdb disconnects via the detach command.
                                 */
            {
            ptr = &remcomInBuffer[2];
            int part_id;
            hexToInt(&ptr, (uintptr_t *)(&part_id));
            remcomOutBuffer[0] = 'O';
            remcomOutBuffer[1] = 'K';
            remcomOutBuffer[2] = 0;
            
            /*If it is last detaching process - continue*/
            if (part_id != 1) break;
            putpacket((unsigned char *)remcomOutBuffer);            
            
            goto CONTINUE;
        }
#endif
        case 's':
        {
            gdb_sate_set_thread(&tc, &gdb_thread_current);
            gdb_state_fill_registers(&tc);
            /*Do we need return from handler now or after 'c'*/
            if (GDB_SINGLE_STEP(ea, registers)){
                return;
            }
        }

        case 'k':    /* kill the program, actually just continue */
        case 'c':    /* cAA..AA  Continue; address AA..AA optional */
            /* try to read optional parameter, pc unchanged if no parm */
CONTINUE:
            ptr = &remcomInBuffer[1];
            if (hexToInt(&ptr, &addr)) {
                gdb_sate_set_thread(&tc, &gdb_thread_current);
                gdb_state_fill_registers(&tc);
                registers[pc] = addr;
                gdb_state_store_registers(&tc);
            }
            return;

        case 'r':       /* Reset (if user process..exit ???)*/
////            panic("kgdb reset.");
            break;

/*
 * 'P' and 'p' commands don't work yet.
 */
        case 'P':       /* set the value of a single CPU register - return OK */
        {
            int regno_offset;
            ptr = &remcomInBuffer[1];
            if (hexToInt (&ptr, (uintptr_t *)&regno_offset) && *ptr++ == '=')
                if (regno_offset >= 0 && regno_offset < NUMREGS * 8) /* 8 - max length of each register*/
                {
                    hex2mem (ptr, (char *) &registers[regno_offset >> 3], GDB_INSTR_SIZE);
                    gdb_strcpy (remcomOutBuffer, "OK");
                    break;
                }

            gdb_strcpy (remcomOutBuffer, "E01");
            break;
        }
        case 'p':       /* Read the value of register; Register is in hex. */
        {
            int regno_offset;
            ptr = &remcomInBuffer[1];
            if (hexToInt (&ptr, (uintptr_t *)&regno_offset))
            {
                if (regno_offset >= 0 && regno_offset < NUMREGS * 8) /* 8 - max length of each register*/
                {
                    mem2hex ((char *) &registers[regno_offset >> 3], remcomOutBuffer, GDB_INSTR_SIZE);
                    break;
                }
            }
            gdb_strcpy (remcomOutBuffer, "E01");
            break;
        }



        }/* switch */
    /* reply to the request */
    putpacket((unsigned char *)remcomOutBuffer);
    } /* while(1) */
    ////printf("\n\n\n          End of handle_exeption\n\n");
}

#else /* POK_NEEDS_GDB */
#include <core/debug.h>
void
handle_exception (int exceptionVector, struct jet_interrupt_context* ea)
{
    (void) exceptionVector;
    (void) ea;
    pok_fatal("Exception without GDB enabled");
}
#endif /* POK_NEEDS_GDB */
