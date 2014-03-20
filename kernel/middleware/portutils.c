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

/**
 * \file    middleware/portutils.c
 * \date    2008-2009
 * \brief   Various functions for ports management.
 * \author  Julien Delange
 */

#if defined (POK_NEEDS_PORTS_SAMPLING) || defined (POK_NEEDS_PORTS_QUEUEING)
#include <types.h>
#include <libc.h>
#include <core/time.h>
#include <middleware/port.h>
#include <middleware/queue.h>

extern pok_port_t    pok_ports[POK_CONFIG_NB_PORTS];
extern pok_queue_t   pok_queue;
extern uint8_t       pok_current_partition;


// how many bytes are free
pok_port_size_t pok_port_available_size (uint32_t pid)
{
   if (pok_ports[pid].full == TRUE)
   {
      return 0;
   }

   if (pok_ports[pid].off_b < pok_ports[pid].off_e)
   {
      return (pok_ports[pid].off_b - pok_ports[pid].off_e);
   }
   else
   {
      return (pok_ports[pid].size - pok_ports[pid].off_e + pok_ports[pid].off_b);
   }
}

// how many bytes are used
pok_port_size_t pok_port_consumed_size (uint32_t pid)
{
   if (pok_ports[pid].empty == TRUE)
   {
      return 0;
   }

   if (pok_ports[pid].off_b < pok_ports[pid].off_e )
   {
      return (pok_ports[pid].off_e - pok_ports[pid].off_b);
   }
   else
   {
      return (pok_ports[pid].size - pok_ports[pid].off_b + pok_ports[pid].off_e);
   }
}

#ifdef POK_NEEDS_PORTS_QUEUEING
static pok_ret_t pok_port_get_queueing(uint32_t id, void *data, pok_port_size_t *size)
{
#error queuing ports are broken for now
    pok_port_size_t tmp_size;
    pok_port_size_t tmp_size2;

    if (pok_ports[id].empty == TRUE)
    {
        return POK_ERRNO_EINVAL;
    }

    if (pok_ports[id].size < size)
    {
        return POK_ERRNO_SIZE;
    }

    if ((pok_ports[id].off_b + size) > pok_ports[id].size)
    {
        tmp_size = pok_ports[id].size - pok_ports[id].off_b;
        memcpy (data, &pok_queue.data[pok_ports[id].index + pok_ports[id].off_b], tmp_size);
        tmp_size2 = size - tmp_size;
        memcpy (data + tmp_size, &pok_queue.data[pok_ports[id].index], tmp_size2);
    }
    else
    {
        memcpy (data, &pok_queue.data[pok_ports[id].index + pok_ports[id].off_b], size);
    }

    pok_ports[id].off_b = (pok_ports[id].off_b + size) % pok_ports[pid].size;

    if (pok_ports[pid].off_b == pok_ports[pid].off_e)
    {
        pok_ports[pid].empty = TRUE;
        pok_ports[pid].full  = FALSE;
    }

    return POK_ERRNO_OK;
    break;
}
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
static pok_ret_t pok_port_get_sampling(uint32_t id, void *destination, pok_port_size_t *size_ptr)
{
    if (pok_ports[id].empty == TRUE) {
        return POK_ERRNO_EMPTY;
    }

    // XXX off_b is always zero?
    const char *data = &pok_queue.data[pok_ports[id].index + pok_ports[id].off_b];

    pok_port_size_t size = *(pok_port_size_t*)data;
    data += sizeof(size);

    memcpy(destination, data, size);
    *size_ptr = size;

    return POK_ERRNO_OK;
    
}
#endif


pok_ret_t pok_port_get (uint32_t id, void *data, pok_port_size_t *size)
{
    switch (pok_ports[id].kind)
    {
#ifdef POK_NEEDS_PORTS_QUEUEING
    case POK_PORT_KIND_QUEUEING:
        return pok_port_get_queueing(id, data, size);
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
    case POK_PORT_KIND_SAMPLING:
        return pok_port_get_sampling(id, data, size); 
#endif
    default:
        return POK_ERRNO_EINVAL;
    }
}

#ifdef POK_NEEDS_PORTS_QUEUEING
static pok_ret_t pok_port_write_queueing(uint32_t pid, const void *data, pok_port_size_t size)
{
    #erro
    pok_port_size_t tmp_size;
    pok_port_size_t tmp_size2;
         if (pok_ports[pid].full == TRUE)
         {
            return POK_ERRNO_SIZE;
         }

         if (size > pok_ports[pid].size)
         {
            return POK_ERRNO_SIZE;
         }

         if ((pok_ports[pid].off_e + size) > pok_ports[pid].size)
         {
            tmp_size = pok_ports[pid].size - pok_ports[pid].off_e;
            memcpy (&pok_queue.data[pok_ports[pid].index + pok_ports[pid].off_e], data, tmp_size);

            tmp_size2 = size - tmp_size;
            memcpy (&pok_queue.data[pok_ports[pid].index], data + tmp_size, tmp_size2);
         }
         else
         {
            memcpy (&pok_queue.data[pok_ports[pid].index + pok_ports[pid].off_e], data, size);
         }

         pok_ports[pid].off_e = (pok_ports[pid].off_e + size) % pok_ports[pid].size;

         if (pok_ports[pid].off_e == pok_ports[pid].off_b)
         {
            pok_ports[pid].full = TRUE;
         }

         pok_ports[pid].empty = FALSE;

         return POK_ERRNO_OK;

         break;
}
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
static pok_ret_t pok_port_write_sampling(uint32_t id, const void *src, pok_port_size_t size)
{
    if (size + sizeof(pok_port_size_t) > pok_ports[id].size) {
        return POK_ERRNO_SIZE;
    }

    // write past the end
    // XXX for sampling ports off_e is always zero?
    char *data = &pok_queue.data[pok_ports[id].index + pok_ports[id].off_e];
    *(pok_port_size_t*)data = size;
    data += sizeof(pok_port_size_t);
    memcpy(data, src, size);

    pok_ports[id].empty = FALSE;
    pok_ports[id].last_receive = POK_GETTICK();

    return POK_ERRNO_OK;
}
#endif

pok_ret_t  pok_port_write (uint32_t id, const void *data, pok_port_size_t size)
{
   switch (pok_ports[id].kind)
   {
#ifdef POK_NEEDS_PORTS_QUEUEING
      case POK_PORT_KIND_QUEUEING:
          return pok_port_write_queueing(id, data, size);
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
      case POK_PORT_KIND_SAMPLING:
          return pok_port_write_sampling(id, data, size);
#endif
      default:
         return POK_ERRNO_EINVAL;
   }
}


/*
 * This function is designed to transfer data from one port to another
 * It is called when we transfer all data from one partition to the
 * others.
 */

#ifdef POK_NEEDS_PORTS_QUEUEING
static pok_ret_t pok_port_transfer_queueing(uint32_t dst, uint32_t src)
{
#error
    pok_port_size_t len = pok_port_available_size (dst);
        
    src_len_consumed = pok_port_consumed_size (pid_src);

      if (src_len_consumed == 0)
      {
         return POK_ERRNO_SIZE;
      }

      if (len > src_len_consumed)
      {
         len = src_len_consumed;
      }
      /*
       * Here, we check the size of data produced in the source port.
       * If there is more free space in the destination port, the size
       * of copied data will be the occupied size in the source port.
       */
      
      pok_ports[pid_dst].off_e =  (pok_ports[pid_dst].off_e + len) % pok_ports[pid_dst].size;
      pok_ports[pid_src].off_b =  (pok_ports[pid_src].off_b + len) % pok_ports[pid_src].size;

      if (pok_ports[pid_src].off_b == pok_ports[pid_src].off_e)
      {
         pok_ports[pid_src].empty = TRUE;
         pok_ports[pid_src].full  = FALSE;
      }
      
      pok_ports[src].full = FALSE;
    pok_ports[dst].empty = FALSE;
}
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
static pok_ret_t pok_port_transfer_sampling(uint32_t dst, uint32_t src)
{
    if (pok_ports[src].size != pok_ports[dst].size) {
        return POK_ERRNO_SIZE;
    }
    if (pok_ports[src].empty == TRUE) {
        return POK_ERRNO_EMPTY;
    }

    pok_port_size_t len = pok_ports[src].size;

    // XXX are offsets always zero for sampling ports?
    const char *src_data = &pok_queue.data[pok_ports[src].index + pok_ports[src].off_b];
    char       *dst_data = &pok_queue.data[pok_ports[dst].index + pok_ports[dst].off_e];

    memcpy (dst_data, src_data, len);

    pok_ports[src].empty = TRUE;
    pok_ports[src].full = FALSE;
    pok_ports[dst].empty = FALSE;

    return POK_ERRNO_OK;
}
#endif

pok_ret_t pok_port_transfer(const uint32_t dst, const uint32_t src)
{
   switch (pok_ports[src].kind)
   {
#ifdef POK_NEEDS_PORTS_QUEUEING
      case POK_PORT_KIND_QUEUEING:
          return pok_port_transfer_queueing(dst, src);
#endif

#ifdef POK_NEEDS_PORTS_SAMPLING
      case POK_PORT_KIND_SAMPLING:
          return pok_port_transfer_sampling(dst, src);
#endif
      default:
         // XXX virtual ports?
         return POK_ERRNO_EINVAL;
   } 
}

bool_t pok_own_port (const uint8_t partition, const uint32_t port)
{
   if (port > POK_CONFIG_NB_PORTS)
   {
      return FALSE;
   }

#ifdef POK_CONFIG_PARTITIONS_PORTS
   if ((((uint8_t[]) POK_CONFIG_PARTITIONS_PORTS)[port]) == partition)
   {
      return TRUE;
   }
#endif

   return FALSE;
}

#endif
