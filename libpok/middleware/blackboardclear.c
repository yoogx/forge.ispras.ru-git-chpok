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


#ifdef POK_NEEDS_MIDDLEWARE
#ifdef POK_NEEDS_BLACKBOARDS

#include <errno.h>
#include <types.h>
#include <libc/string.h>
#include <middleware/blackboard.h>

extern pok_blackboard_t    pok_blackboards[POK_CONFIG_NB_BLACKBOARDS];

pok_ret_t pok_blackboard_clear (const pok_blackboard_id_t id)
{
   if (id >= POK_CONFIG_NB_BLACKBOARDS)
   {
      return POK_ERRNO_EINVAL;
   }

   if (!pok_blackboards[id].ready) {
      return POK_ERRNO_EINVAL;
   }

   pok_event_lock(pok_blackboards[id].lock);
   pok_blackboards[id].empty = TRUE;
   pok_event_unlock(pok_blackboards[id].lock);
   return POK_ERRNO_OK;
}

#endif /* POK_NEEDS_BLACKBOARDS */
#endif
