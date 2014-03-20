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
    // XXX there's something wrong with this function
   if (pok_ports[pid].full == TRUE)
   {
      return 0;
   }

   if (pok_ports[pid].off_b < pok_ports[pid].off_e)
   {
      return (pok_ports[pid].off_b - pok_ports[pid].off_e);
   } else {
      return (pok_ports[pid].size - pok_ports[pid].off_e + pok_ports[pid].off_b);
   }
}

// how many bytes are used
pok_port_size_t pok_port_consumed_size (uint32_t pid)
{
    // XXX there's something wrong with this function
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
static pok_ret_t pok_port_get_queueing(uint32_t id, void *dst, pok_port_size_t *size)
{
    if (pok_ports[id].empty == TRUE)
    {
        return POK_ERRNO_EINVAL;
    }

    // read message from the buffer
    // starting with off_b

    const char *buffer = &pok_queue.data[pok_ports[id].index + pok_ports[id].off_b];
    *size = *(pok_port_size_t*)buffer;
    buffer += sizeof(pok_port_size_t);

    memcpy(dst, buffer, *size);

    pok_ports[id].off_b = (pok_ports[id].off_b + pok_ports[id].message_size) % pok_ports[id].size;

    // XXX recheck this logic
    if (pok_ports[id].off_b == pok_ports[id].off_e)
    {
        pok_ports[id].empty = TRUE;
        pok_ports[id].full  = FALSE;
    }

    return POK_ERRNO_OK;
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
static pok_ret_t pok_port_write_queueing(uint32_t pid, const void *src, pok_port_size_t size)
{
    if (pok_ports[pid].full == TRUE) {
        return POK_ERRNO_SIZE;
    }

    pok_port_size_t required_size = size + sizeof(size);
    if (required_size > pok_port_available_size(pid)) {
        return POK_ERRNO_SIZE;
    }

    // put message into buffer, starting with off_e
    // since messages are of fixed size, 
    // (excess data is padded)
    // we don't have to bother with wrapping
    
    char *buffer = &pok_queue.data[pok_ports[pid].index + pok_ports[pid].off_e];
    *(pok_port_size_t*)buffer = size;
    buffer += sizeof(pok_port_size_t);
    memcpy(buffer, src, size);

    // XXX
    // empty-full status?
    // last receive stamp?

    pok_ports[pid].off_e = (pok_ports[pid].off_e + pok_ports[pid].message_size) % pok_ports[pid].size;

    return POK_ERRNO_OK;
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
    char *buffer = &pok_queue.data[pok_ports[id].index + pok_ports[id].off_e];
    *(pok_port_size_t*)buffer = size;
    buffer += sizeof(pok_port_size_t);
    memcpy(buffer, src, size);

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
    (void) dst;
    (void) src;
    return 0;

    // TODO implement
#if 0
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
#endif
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
