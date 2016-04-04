#ifndef __POK_NET_NETDEV_H__
#define __POK_NET_NETDEV_H__

void register_netdevice(char *name, pok_netdevice_t *dev);
pok_netdevice_t* get_netdevice(char *name);

#endif
