#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <libc.h>

#include <asp/entries.h>


void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    (void) r0;
    (void) r1;
    (void) atags;

    jet_console_init_all ();

    printf("Hello world\n");
}
