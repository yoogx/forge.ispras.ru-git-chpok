/*  
 *  Copyright (C) 2014 Maxim Malkov, ISPRAS <malkov@ispras.ru> 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <net/network.h>
#include <net/byteorder.h>
#include <net/ether.h>
#include <net/ip.h>
#include <net/udp.h>
#include <pci.h>
#include <depl.h>

#include <stdio.h>
#include <string.h>
#include <net/netdevices.h>
#include <channel_driver.h>

//NKK
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
#include "chpok-conf.h"
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

extern udp_data_t ipnet_data_0, ipnet_data_1;
enum appstate uip_state;
char *msg_buffer;
size_t msg_buffer_size;

static pok_bool_t initialized = FALSE;

pok_netdevice_t *current_netdevice;
#define NETDEVICE_PTR current_netdevice
#define NETWORK_DRIVER_OPS (NETDEVICE_PTR->ops)

pok_bool_t (*received_callback)(
        uint32_t ip,
        uint16_t port,
        const char *payload,
        size_t length);

static void flush_send();
static void reclaim_send_buffers();

#define POK_NEEDS_ARP_ANSWER

#ifdef POK_NEEDS_ARP_ANSWER
static void stub_callback(void *arg) 
{
    printf("ARP: we have sent an answer.\n");
}
#endif // POK_NEEDS_ARP_ANSWER

//-******************************************************************
//-********************** packet_received_callback ******************
//-******************************************************************
static void packet_received_callback(const char *data, size_t len)
{
    printf("in packet_received_callback!\n");
    uip_state = RECIEVE_STATE;
    memcpy( uip_buf, data, len );
    uip_len = len;
    if(uip_len > 0) 
    {
        if(BUF->type == HTONS(UIP_ETHTYPE_IP))  //Just packet
        {
            uip_arp_ipin();
            uip_input();
            //Возможно дописать отправку
        } 
#ifdef POK_NEEDS_ARP_ANSWER 
        else if(BUF->type == HTONS(UIP_ETHTYPE_ARP)) //ARP-packet
        {
            uip_arp_arpin();
            if(uip_len > 0)
            {
                NETWORK_DRIVER_OPS->send_frame(
                NETDEVICE_PTR,
                uip_buf,
                uip_len + POK_NETWORK_OVERHEAD,
                stub_callback,
                NULL
                );
                flush_send();
                reclaim_send_buffers();
            }
        }
#endif // POK_NEEDS_ARP_ANSWER
    }
}

uint8_t default_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

extern struct mac_ip mac_addr_mapping[];
uint8_t* find_mac_by_ip(uint32_t dst_ip)
{
    for (int i=0; i < mac_addr_mapping_nb; i++) {
        if (mac_addr_mapping[i].ip == dst_ip)
            return mac_addr_mapping[i].mac;
    }
    return default_mac;
}

//-******************************************************************
//-********************** pok_network_init **************************
//-******************************************************************
void pok_network_init(void) 
{
    printf("in pok_network_init!\n");
    uip_ipaddr_t ipaddr;
    
    //Not NKK
    current_netdevice = get_netdevice(ipnet_netdev_name);

    if (current_netdevice == NULL) {
        printf("ipnet ERROR: netdevice '%s' not found\n", ipnet_netdev_name);
        return;
    }
    
    NETWORK_DRIVER_OPS->set_packet_received_callback(NETDEVICE_PTR, packet_received_callback);
    initialized = TRUE;
    
    //NKK: Set up ip and mac: use real values! 
    struct uip_eth_addr myethaddr = {{NETDEVICE_PTR->mac[0],
                                      NETDEVICE_PTR->mac[1],
                                      NETDEVICE_PTR->mac[2],
                                      NETDEVICE_PTR->mac[3],
                                      NETDEVICE_PTR->mac[4],
                                      NETDEVICE_PTR->mac[5]
                                    }};
    uip_ipaddr( ipaddr,
                ((ipnet_data_1.ip) & 0xff000000) >> 24,
                ((ipnet_data_1.ip) & 0xff0000) >> 16,
                ((ipnet_data_1.ip) & 0xff00) >> 8,
                ((ipnet_data_1.ip) & 0xff)
              );
    uip_sethostaddr(ipaddr);                //Host IP (own)
    uip_ipaddr(ipaddr,
                ((ipnet_data_0.ip) & 0xff000000) >> 24,
                ((ipnet_data_0.ip) & 0xff0000) >> 16,
                ((ipnet_data_0.ip) & 0xff00) >> 8,
                ((ipnet_data_0.ip) & 0xff)
              );
    uip_setdraddr(ipaddr);                  //Router IP
    uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);
    uip_setethaddr(myethaddr);              //My MAC
    printf( "DONE SETUP IP!\n" );
    //NKK: uip init
    uip_init();
    uip_appcall_init();
    uip_state = IDLE_STATE;
}

//-******************************************************************
//-********************** fill_in_udp_header ************************
//-******************************************************************
static void fill_in_udp_header(
        char *buffer, 
        size_t size, // size of UDP data
        uint32_t dst_ip, 
        uint16_t dst_port)
{
    struct {
        struct ether_hdr ether_hdr;
        struct ip_hdr ip_hdr;
        struct udp_hdr udp_hdr;
        char data[];
    } __attribute__((packed)) *real_buffer = (void*) buffer;

    // fill in Ethernet header
    int i;
    uint8_t *dst_mac = find_mac_by_ip(dst_ip);
    for (i = 0; i < ETH_ALEN; i++) {
        real_buffer->ether_hdr.src[i] = NETDEVICE_PTR->mac[i];   //My mac
        real_buffer->ether_hdr.dst[i] = dst_mac[i];
    }
    real_buffer->ether_hdr.ethertype = hton16(ETH_P_IP);

    // ...next, IP heaader
    real_buffer->ip_hdr.version_len = (4 << 4) | 5;
    real_buffer->ip_hdr.dscp = 0;
    real_buffer->ip_hdr.length = hton16(
            sizeof(struct ip_hdr) +
            sizeof(struct udp_hdr) +
            size
    );
    real_buffer->ip_hdr.checksum = 0; // it's filled in just below
    real_buffer->ip_hdr.id = 0;
    real_buffer->ip_hdr.offset = 0;
    real_buffer->ip_hdr.ttl = 32; 
    real_buffer->ip_hdr.proto = IPPROTO_UDP;
    real_buffer->ip_hdr.src = hton32(pok_network_ip_address);
    real_buffer->ip_hdr.dst = hton32(dst_ip);
    
    real_buffer->ip_hdr.checksum = ip_hdr_checksum(&real_buffer->ip_hdr);

    // ... and UDP header
    real_buffer->udp_hdr.src_port = hton16(dst_port);
    real_buffer->udp_hdr.dst_port = hton16(dst_port); 
    real_buffer->udp_hdr.length = hton16(size + sizeof(struct udp_hdr));
    real_buffer->udp_hdr.checksum = 0; // no checksum
}

//-******************************************************************
//-********************** pok_network_send_udp **********************
//-******************************************************************
pok_bool_t pok_network_send_udp(
    char *buffer,
    size_t size,
    uint32_t dst_ip,
    uint16_t dst_port,
    pok_network_buffer_callback_t callback,
    void *callback_arg)
{
    printf("in pok_network_send_udp!\n");
    printf("size is: %d, it was buffer\n", size);
    int ii;
    uip_ipaddr_t addr;

    if (!initialized) 
        return FALSE;

    uip_state = SEND_STATE;
    msg_buffer = buffer + POK_NETWORK_UDP;
    //memcpy( msg_buffer, buffer, size );
    msg_buffer_size = size;
                                                                    //memset( msg_buffer, 's', 600 );
                                                                    //msg_buffer_size = 600;
    addr[0] =  (dst_ip&0xFFFF0000)>>16;
    addr[1] = dst_ip&0xFFFF;
    
    for(ii = 0; ii < UIP_UDP_CONNS; ++ii) 
    {
        printf( "befor uip_udp_periodic(%d)\n", ii );
        uip_udp_periodic(ii);
        if ((uip_len > 0)&&
            (uip_udp_conns[ii].ripaddr[0] == addr[0])&&
            (uip_udp_conns[ii].rport == dst_port)
           )
        {
            //hexdump(uip_buf, uip_len+POK_NETWORK_OVERHEAD);
            uip_arp_update( addr, (struct uip_eth_addr *)find_mac_by_ip( dst_ip ) );//New ARP table entry
            uip_arp_out ();                                 //Create Ethernet header, sendidng ARP
            
            return NETWORK_DRIVER_OPS->send_frame(
                NETDEVICE_PTR,
                uip_buf,
                size + POK_NETWORK_OVERHEAD,
                callback,
                callback_arg
                ); 
        }
    }
    printf("pok_network_send_udp: No matching connection found!\n");
    return;
}

pok_bool_t pok_network_send_udp_gather(
    const pok_network_sg_list_t *sg_list,
    size_t sg_list_len,
    uint32_t dst_ip,
    uint16_t dst_port,
    pok_network_buffer_callback_t callback,
    void *callback_arg)
{
    if (!initialized) return FALSE;

    if (sg_list_len == 0) {
        printf("pok_network_send_udp_gather: list_len should not be zero");
        return FALSE;
    }

    if (sg_list[0].size != POK_NETWORK_OVERHEAD) {
        printf("pok_network_send_udp_gather: wrong size of list element");
        return FALSE;
    }

    size_t payload_length = 0;
    size_t i;
    for (i = 1; i < sg_list_len; i++) {
        payload_length += sg_list[i].size;
    }

    fill_in_udp_header(
        sg_list[0].buffer, 
        payload_length, 
        dst_ip, 
        dst_port
    );

    return NETWORK_DRIVER_OPS->send_frame_gather(
        NETDEVICE_PTR,
        sg_list,
        sg_list_len,
        callback,
        callback_arg
    );
}

static pok_bool_t send(
        char *buffer,
        size_t buffer_size,
        void *driver_data,
        pok_network_buffer_callback_t callback,
        void *callback_arg
    )
{
    printf("in send()\n");
    udp_data_t *udp_data = driver_data;
    return pok_network_send_udp(
            buffer,
            buffer_size,
            udp_data->ip,
            udp_data->port,
            callback,
            callback_arg
            );
}

static void reclaim_send_buffers(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->reclaim_send_buffers(NETDEVICE_PTR);
    }
}

static void receive(void)
{
    if (initialized) {
        NETWORK_DRIVER_OPS->reclaim_receive_buffers(NETDEVICE_PTR);
    }
}

static void flush_send(void) {
    if (initialized) {
        NETWORK_DRIVER_OPS->flush_send(NETDEVICE_PTR);
    }
}

void register_received_callback(
            pok_bool_t (*callback)(
                uint32_t ip,
                uint16_t port,
                const char *payload,
                size_t length
                )
            )
{
    received_callback = callback;
}

struct channel_driver ipnet_channel_driver = {
    .send = send,
    .reclaim_send_buffers = reclaim_send_buffers,
    .receive = receive,
    .flush_send = flush_send,
    .register_received_callback = register_received_callback
};
