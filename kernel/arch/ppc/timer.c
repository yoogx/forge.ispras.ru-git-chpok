/*
 *                               POK header
 * 
 * The following file is a part of the POK project. Any modification should
 * made according to the POK licence. You CANNOT use this file or a part of
 * this file is this part of a file for your own project
 *
 * For more information on the POK licence, please see our LICENCE FILE
 *
 * Please follow the coding guidelines described in doc/CODING_GUIDELINES
 *
 *                                      Copyright (c) 2007-2009 POK team 
 *
 * Created by julien on Thu Jan 15 23:34:13 2009 
 */


#include <errno.h>
#include <bsp.h>
#include <core/time.h>
#include <core/sched.h>
#include <core/debug.h>
#include "reg.h"
#include "timer.h"

/* From platform.  */
#define BUS_FREQ (100 * 1000000U)

#define FREQ_DIV 40

/* Last time when decr was set.  */
static uint64_t time_last;

/* Decrementer optimal value.  */
static uint32_t time_inter;

static uint64_t get_timebase(void)
{
    while (1) {
        uint32_t upper = mfspr(SPRN_TBRU);
        uint32_t lower = mfspr(SPRN_TBRL);

        // since it's two part register,
        // there's a possible race condition
        // we avoid it here with a loop
        if (upper == mfspr(SPRN_TBRU)) {
            return (((uint64_t) upper) << 32) | lower;
        }
    }
}

/* Compute new value for the decrementer.  If the value is in the future,
   sets the decrementer else returns an error.  */
static int set_decrementer(void)
{
  uint64_t time_new = time_last + time_inter;
  uint64_t time_cur = get_timebase();
  int32_t delta = time_new - time_cur;

  time_last = time_new;

  if (delta < 0)
  {
    // that delta already expired
    return POK_ERRNO_EINVAL;
  }
  else
  {
    mtspr(SPRN_DEC, delta);
    return POK_ERRNO_OK;
  }
}

/* Called by the interrupt handled.  */
void pok_arch_decr_int (void)
{
  int err;

  // clear pending intrerrupt
  mtspr(SPRN_TSR, TSR_DIS);

  do
  {
    err = set_decrementer();
    pok_tick_counter += FREQ_DIV;
  } while (err != POK_ERRNO_OK);


  pok_sched ();
}

pok_ret_t pok_bsp_time_init ()
{
  time_inter = (BUS_FREQ * FREQ_DIV) / POK_TIMER_FREQUENCY;
  time_last = get_timebase ();
  set_decrementer();

  mtspr(SPRN_TCR, TCR_DIE); // enable decrementer

  return (POK_ERRNO_OK);
}
