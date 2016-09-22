#include "uip.h"
#include <stdio.h>
#include <string.h>
#include <depl.h>
#include <net/byteorder.h>
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

static struct chpok_state s[2];
extern enum appstate uip_state;
extern char *msg_buffer;
extern size_t msg_buffer_size;
extern pok_bool_t (*received_callback)(
        uint32_t ip,
        uint16_t port,
        const char *payload,
        size_t length);

void uip_log(char *m)
{
  printf("uIP log message: %s\n", m);
}

void uip_appcall_init(void)
{
    uip_ipaddr_t addr;

    uip_ipaddr(addr, 255, 255, 255, 255);
    s[0].conn = uip_udp_new(&addr, HTONS(0));
    if(s[0].conn != NULL) 
    {
        uip_udp_bind(s[0].conn, HTONS(10001));
    }
    
    uip_ipaddr(addr, 192, 168, 56, 1);
    s[0].conn = uip_udp_new(&addr, HTONS(10000));
    if(s[0].conn != NULL) 
    {
        uip_udp_bind(s[0].conn, HTONS(10001));
    }

}

void chpok_appcall(void)
{
    printf( "           In chpok_appcall!\n" );
    if( uip_state == SEND_STATE && uip_udp_conn == s[0].conn )
    {
        uip_send( msg_buffer, msg_buffer_size );
    }
    if( uip_state == RECIEVE_STATE && uip_newdata() )
    {
        uint32_t dst_ip = IP_ADDR(192, 168, 56, 101);
        printf( "datalen: %d\n", uip_datalen() );
        hexdump( uip_appdata, uip_datalen() );
        received_callback(
                ntoh32(dst_ip),
                ntoh16(uip_udp_conn->lport),
                uip_appdata, 
                uip_datalen()
        );
    }
}
