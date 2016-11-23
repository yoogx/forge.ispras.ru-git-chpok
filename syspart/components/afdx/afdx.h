#ifndef __AFDX_H_
#define __AFDX_H_
/********************************************/
/*
 * This structure  describes packeges types
 * The packet may be unicast or multicast
 */
typedef enum
{
   UNICAST_PACKET       = 0,
   MULTICAST_PACKET     = 1,
} PACKET_TYPE;

#endif
