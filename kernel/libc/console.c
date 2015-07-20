
#include <libc.h>
#include <bsp.h>


/*
*	Function described in cons.c:
*	data_to_read() - return 1 if there is data to read, or 0 otherwise
*	read_serial()  - read and return 1 word from receiver buffer
*/


static int
serial_proc_data(void) //Read data from DUART (1 word)
{
	if (!data_to_read())
		return -1;
	return read_serial();
}

/***** General device-independent console code *****/
// Here we manage the console input buffer,
// where we stash characters received from the keyboard or serial port
// whenever the corresponding interrupt occurs.

#define CONSBUFSIZE 512

static struct {
	uint8_t buf[CONSBUFSIZE];
	uint32_t rpos;
	uint32_t wpos;
} cons;

// called by device interrupt routines to feed input characters
// into the circular console input buffer.
static void
cons_intr() //read all elements from DUART
{
	int c;

	while ((c = serial_proc_data()) != -1) {
		if (c == 0)
			continue;
		cons.buf[cons.wpos++] = c;
		if (cons.wpos == CONSBUFSIZE)
			cons.wpos = 0;
	}
}

// return the next input character from the console, or 0 if none waiting
int
cons_getc(void) //Get 1 element from cons
{
	int c;

	// poll for any pending input characters,
	// so that this function works even when interrupts are disabled
	// (e.g., when called from the kernel monitor).
	cons_intr();

	// grab the next character from the input buffer.
	if (cons.rpos != cons.wpos) {
		c = cons.buf[cons.rpos++];
		if (cons.rpos == CONSBUFSIZE)
			cons.rpos = 0;
		return c;
	}
	return 0;
}

// `High'-level console I/O.  Used by readline and cprintf.

int
getchar(void)
{
	int c;
	
	while ((c = cons_getc()) == 0)
		/* do nothing */;
	return c;
}

