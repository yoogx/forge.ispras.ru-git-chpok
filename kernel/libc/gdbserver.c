#define PC_REGNUM 64
#define SP_REGNUM 1




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



/************************************************************************
 *
 * external low-level support routines
 */

char		*strcpy(char *dest, const char *str)
{
  unsigned int i;
  for (i = 0; str[i];i++)
    dest[i] = str[i];
  dest[i] = '\0';
  return dest;
}


extern void putDebugChar( char );	/* write a single character      */
extern int getDebugChar();	/* read and return a single char */
extern void exceptionHandler();	/* assign an exception handler   */

void putDebugChar(char c){
    pok_cons_write_1(&c,1);
    pok_cons_write(&c,1);
}

int getDebugChar(){
    int inf=getchar2();
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
#define NUMREGS	38

/* Number of bytes of registers.  */
#define NUMREGBYTES (NUMREGS * 4)

enum regnames { r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13,
r14, r15, r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29,
r30, r31, pc, msr, cr, lr, ctr, xer 
};




/*
 * these should not be static cuz they can be used outside this module
 */
int registers[NUMREGS];

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
				putDebugChar('-');	/* failed checksum */
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



/* send the packet in buffer.  */

void
putpacket (unsigned char *buffer)
{
  unsigned char checksum;
  int count;
  char ch;
  printf("Lets putpacket --->\n");
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
      break;			/* divide by zero */
    case 1:
      sigval = 5;
      break;			/* debug exception */
    case 3:
      sigval = 5;
      break;			/* breakpoint */
    case 4:
      sigval = 16;
      break;			/* into instruction (overflow) */
    case 5:
      sigval = 16;
      break;			/* bound instruction */
    case 6:
      sigval = 4;
      break;			/* Invalid opcode */
    case 7:
      sigval = 8;
      break;			/* coprocessor not available */
    case 8:
      sigval = 7;
      break;			/* double fault */
    case 9:
      sigval = 11;
      break;			/* coprocessor segment overrun */
    case 10:
      sigval = 11;
      break;			/* Invalid TSS */
    case 11:
      sigval = 11;
      break;			/* Segment not present */
    case 12:
      sigval = 11;
      break;			/* stack exception */
    case 13:
      sigval = 11;
      break;			/* general protection */
    case 14:
      sigval = 11;
      break;			/* page fault */
    case 16:
      sigval = 7;
      break;			/* coprocessor error */
    case 17:            /* SIGINT  */
      sigval = 2;
      break;
    default:
      sigval = 7;		/* "software generated" */
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

char instr[8]="00000000";
int addr_instr=0;
char trap[8]="7fe00008";



#define MSR_SE_LG	10		/* Single Step */
#define __MASK(X)	(1<<(X))
#define MSR_SE		__MASK(MSR_SE_LG)	/* Single Step */

static inline void set_msr(int msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}




/*
 * This function does all command procesing for interfacing to gdb.
 */
void
handle_exception (int exceptionVector, struct regs * ea)
{
  /*Add regs*/
  registers[r0]=ea->r0;
  registers[r1]=ea->r1;
  registers[r2]=ea->r2;
  registers[r3]=ea->r3;
  registers[r4]=ea->r4;
  registers[r5]=ea->r5;
  registers[r6]=ea->r6;
  registers[r7]=ea->r7;
  registers[r8]=ea->r8;
  registers[r9]=ea->r9;
  registers[r10]=ea->r10;
  registers[r11]=ea->r11;
  registers[r12]=ea->r12;
  registers[r13]=ea->r13;
  registers[r14]=ea->r14;
  registers[r15]=ea->r15;
  registers[r16]=ea->r16;
  registers[r17]=ea->r17;
  registers[r18]=ea->r18;
  registers[r19]=ea->r19;
  registers[r20]=ea->r20;
  registers[r21]=ea->r21;
  registers[r22]=ea->r22;
  registers[r23]=ea->r23;
  registers[r24]=ea->r24;
  registers[r25]=ea->r25;
  registers[r26]=ea->r26;
  registers[r27]=ea->r27;
  registers[r28]=ea->r28;
  registers[r29]=ea->r29;
  registers[r30]=ea->r30;
  registers[r31]=ea->r31;
  registers[ctr]=ea->ctr;
  registers[xer]=ea->xer;
  registers[pc]=ea->srr0;
  registers[msr]=ea->srr1;
  registers[lr]=ea->lr;
    //~ printf("\n\n            In gdbserver:\n");
    //~ printf("cr = 0x%x\n",registers[cr]);
    //~ printf("r0 = 0x%x\n",registers[r0]);
    //~ printf("r2 = 0x%x\n",registers[r2]);
    //~ printf("r3 = 0x%x\n",registers[r3]);
    //~ printf("r4 = 0x%x\n",registers[r4]);
    //~ printf("r5 = 0x%x\n",registers[r5]);
    //~ printf("r6 = 0x%x\n",registers[r6]);
    //~ printf("r7 = 0x%x\n",registers[r7]);
    //~ printf("r8 = 0x%x\n",registers[r8]);
    //~ printf("r9 = 0x%x\n",registers[r9]);
    //~ printf("r10 = 0x%x\n",registers[r10]);
    //~ printf("r11 = 0x%x\n",registers[r11]);
    //~ printf("r12 = 0x%x\n",registers[r12]);
    //~ printf("r13 = 0x%x\n",registers[r13]);
    //~ printf("r14 = 0x%x\n",registers[r14]);
    //~ printf("r15 = 0x%x\n",registers[r15]);
    //~ printf("r16 = 0x%x\n",registers[r16]);
    //~ printf("r17 = 0x%x\n",registers[r17]);
    //~ printf("r18 = 0x%x\n",registers[r18]);
    //~ printf("r19 = 0x%x\n",registers[r19]);
    //~ printf("r20 = 0x%x\n",registers[r20]);
    //~ printf("r21 = 0x%x\n",registers[r21]);
    //~ printf("r22 = 0x%x\n",registers[r22]);
    //~ printf("r23 = 0x%x\n",registers[r23]);
    //~ printf("r24 = 0x%x\n",registers[r24]);
    //~ printf("r25 = 0x%x\n",registers[r25]);
    //~ printf("r26 = 0x%x\n",registers[r26]);
    //~ printf("r27 = 0x%x\n",registers[r27]);
    //~ printf("r28 = 0x%x\n",registers[r28]);
    //~ printf("r29 = 0x%x\n",registers[r29]);
    //~ printf("r30 = 0x%x\n",registers[r30]);
    //~ printf("r31 = 0x%x\n",registers[r31]);
    //~ printf("ctr = 0x%x\n",registers[ctr]);
    //~ printf("xer = 0x%x\n",registers[xer]);
    //~ printf("srr0 or pc = 0x%x\n",registers[pc]); 
    //~ printf("srr1 or mrc = 0x%x\n",registers[msr]); 
    //~ printf("lr = 0x%x\n",registers[lr]);  
  
  memset(remcomOutBuffer, 0, BUFMAX);
  memset(remcomInBuffer, 0, BUFMAX);
  int sigval;////, stepping;
  int addr, length;
  char *ptr;
////  int newPC;

  ////gdb_i386vector = exceptionVector;

  if (1 == 1)////remote_debug)
    {
      printf ("vector=%d, sr=0x%x, pc=0x%x\n",
	      exceptionVector, registers[pc], registers[pc]);
    }

  if (addr_instr != 0){
    hex2mem(instr, (char *) (addr_instr), 4);
    addr_instr=0;
  }
  
  /* reply to host that an exception has occurred */
  sigval = computeSignal (exceptionVector);

  ptr = remcomOutBuffer;

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
    ptr = 0;
    putpacket ( (unsigned char *) remcomOutBuffer);
    
    
    	while (1) {
		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		switch (remcomInBuffer[0]) {
		case '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[sigval >> 4];
			remcomOutBuffer[2] = hexchars[sigval & 0xf];
			remcomOutBuffer[3] = 0;
			break;
#if 0
		case 'q': /* this screws up gdb for some reason...*/
		{
			extern long _start, sdata, __bss_start;
    
			ptr = &remcomInBuffer[1];
			if (strncmp(ptr, "Offsets", 7) != 0)
				break;

			ptr = remcomOutBuffer;
			sprintf(ptr, "Text=%8.8x;Data=%8.8x;Bss=%8.8x",
				&_start, &sdata, &__bss_start);
			break;
		}
#endif
		case 'd':
			/* toggle debug flag */
			////kdebug ^= 1;
			break;

		case 'g':	/* return the value of the CPU registers.
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
			int i;
			ptr = remcomOutBuffer;
			/* General Purpose Regs */
			ptr = mem2hex((char *)registers, ptr, 32 * 4);
			/* Floating Point registers - FIXME */
			/*ptr = mem2hex((char *), ptr, 32 * 8);*/
			for(i=0; i<(32*8*2); i++) { /* 2chars/byte */
				ptr[i] = '0';
			}
			ptr += 32*8*2;
			/* pc, msr, cr, lr, ctr, xer, (mq is unused) */
			ptr = mem2hex((char *)&registers[pc]/*[nip]*/, ptr, 4);
			ptr = mem2hex((char *)&registers[msr], ptr, 4);
			ptr = mem2hex((char *)&registers[cr]/*[ccr]*/, ptr, 4);
			ptr = mem2hex((char *)&registers[lr]/*[link]*/, ptr, 4);
			ptr = mem2hex((char *)&registers[ctr], ptr, 4);
			ptr = mem2hex((char *)&registers[xer], ptr, 4);
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
			/*ptr = hex2mem(ptr, ??, 32 * 8);*/
			ptr += 32*8*2;

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
		case 'H':
			/* don't do anything, yet, just acknowledge */
			hexToInt(&ptr, &addr);
			strcpy(remcomOutBuffer,"OK");
			break;

		case 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */
            printf("It's 'm'! \n");
			ptr = &remcomInBuffer[1];

			if (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				if (mem2hex((char *)addr, remcomOutBuffer,length))
					break;
				strcpy (remcomOutBuffer, "E03");
			} else {
				strcpy(remcomOutBuffer,"E01");
			}
			break;

		case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ptr = &remcomInBuffer[1];

			if (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				if (hex2mem(ptr, (char *)addr, length)) {
                    if (strncmp(ptr,"7d821008",8) == 0){
                        printf("\n\n                == 0 \n\n");
                        hex2mem(trap, (char *)addr, length);
                    }
					strcpy(remcomOutBuffer, "OK");
				} else {
					strcpy(remcomOutBuffer, "E03");
				}
				////flush_icache_range(addr, addr+length);
			} else {
				strcpy(remcomOutBuffer, "E02");
			}
			break;


		case 'k':    /* kill the program, actually just continue */
		case 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
            printf("\nContinue\n\n");
			ptr = &remcomInBuffer[1];
			if (hexToInt(&ptr, &addr)) {
				printf("\nIn if\n");
                registers[pc]/*nip*/ = addr;
			}
        
/* Need to flush the instruction cache here, as we may have deposited a
 * breakpoint, and the icache probably has no way of knowing that a data ref to
 * some location may have changed something that is in the instruction cache.
 */
////			kgdb_flush_cache_all();
////			set_msr(msr);
////			kgdb_interruptible(1);
////			unlock_kernel();
////			kgdb_active = 0;
			//my code
////            ea->srr0+=4;
            //my code end
            
            
            return;

		case 's':
            mem2hex((char *)(registers[pc]+4), instr,4);            
            hex2mem(trap, (char *)(registers[pc]+4), 4);
            addr_instr = registers[pc] + 4;
           
				////flush_icache_range(addr, addr+length);

                

////			kgdb_flush_cache_all();
////			registers[msr] |= MSR_SE;
            
////#if 0
////			set_msr(registers[msr]);
////#endif
////			unlock_kernel();
////			kgdb_active = 0;
			return;

		case 'r':		/* Reset (if user process..exit ???)*/
////			panic("kgdb reset.");
			break;
		}			/* switch */
		if (remcomOutBuffer[0] && 0 /*kdebug*/) {
			printf("remcomInBuffer: %s\n", remcomInBuffer);
			printf("remcomOutBuffer: %s\n", remcomOutBuffer);
		}
		/* reply to the request */
		putpacket((unsigned char *)remcomOutBuffer);
	} /* while(1) */
    printf("\n\n\n          End of handle_exeption\n\n");
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
