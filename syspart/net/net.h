#ifndef __NET_H__
#define __NET_H__

extern pok_netdevice_t *current_netdevice;
extern pok_bool_t initialized;

#define NETDEVICE_PTR current_netdevice
#define NETWORK_DRIVER_OPS (NETDEVICE_PTR->ops)

#endif
