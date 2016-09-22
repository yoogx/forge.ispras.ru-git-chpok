#ifndef __CHPOK_CONF_H__
#define __CHPOK_CONF_H__

#include <string.h>

enum appstate { IDLE_STATE, SEND_STATE, RECIEVE_STATE } uip_state;

struct chpok_state { 
  //char state;
  struct uip_udp_conn *conn; //Зависит от TCP/UDP!!!
  //struct uip_conn *conn;
  char *buffer;
  size_t size;
};
typedef struct chpok_state uip_tcp_appstate_t;
typedef struct chpok_state uip_udp_appstate_t;

#define UIP_UDP_APPCALL chpok_appcall
#define UIP_APPCALL chpok_appcall

void chpok_appcall(void);
void uip_appcall_init(void);

#endif /* __CHPOK_CONF_H__ */
