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


#ifdef POK_NEEDS_PORTS_SAMPLING
#include <errno.h>
#include <types.h>
#include <core/lockobj.h>
#include <core/sched.h>
#include <core/time.h>
#include <middleware/port.h>
#include <middleware/queue.h>
#include <libc.h>

extern pok_port_t    pok_ports[POK_CONFIG_NB_PORTS];

pok_ret_t pok_port_sampling_read (pok_port_id_t id, 
                                  void*               data, 
                                  pok_port_size_t*    len,
                                  bool_t*             valid)
{
   pok_ret_t ret;

   if (data == NULL)
   {
      return POK_ERRNO_EINVAL;
   }

   if (id == 0 || id > POK_CONFIG_NB_PORTS)
   {
      return POK_ERRNO_EINVAL;
   }

   id--;

   if (! pok_own_port (POK_SCHED_CURRENT_PARTITION, id))
   {
      return POK_ERRNO_PORT;
   }

   if (pok_ports[id].ready != TRUE)
   {
      return POK_ERRNO_PORT;
   }

   if (pok_ports[id].kind != POK_PORT_KIND_SAMPLING)
   {
      return POK_ERRNO_EINVAL;
   }

   if (pok_ports[id].direction != POK_PORT_DIRECTION_IN)
   {
      return POK_ERRNO_DIRECTION;
   }

   pok_lockobj_lock (&pok_ports[id].lock, NULL);

   ret = pok_port_get (id, data, len);

   if (ret == POK_ERRNO_EMPTY)
   {
      pok_lockobj_unlock (&pok_ports[id].lock, NULL);
      *len = 0;
      *valid = 0;
      return POK_ERRNO_EMPTY;
   }

   if ( (pok_ports[id].last_receive + pok_ports[id].refresh) < POK_GETTICK ())
   {
      *valid = FALSE;
   }
   else
   {
      *valid = TRUE;
   }

   pok_lockobj_unlock (&pok_ports[id].lock, NULL);

   return POK_ERRNO_OK;
}

#endif

