/*
 * This file is based on u-boot driver. In new versions of u-boot they have added support 
 * of different endianess and 64-bit platforms. This file older version of u-boot. Be aware.
 *
 * Also this code expected to be running after u-boot correctly initialize all devices
 *
 *
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 *	Dave Liu <daveliu@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdio.h>
#include <ioports.h>
#include <string.h>
#include <net/network.h>
#include <memory.h>
#include "fm.h"

#define FM_PRAM_SIZE  sizeof(struct fm_port_global_pram)
#define FM_PRAM_ALIGN 256
#define PRAM_MODE_GLOBAL	0x20000000
#define PRAM_MODE_GRACEFUL_STOP	0x00800000


#define TCTRL_GTS       0x00000020 /* Graceful transmit stop */
#define RCTRL_GRS       0x00000020 /* graceful receive stop */
#define MACCFG1_RX_EN   0x00000004 /* Rx enable */
#define MACCFG1_TX_EN   0x00000001 /* Tx enable */
#define MACCFG1_RXTX_EN (MACCFG1_RX_EN | MACCFG1_TX_EN)


#define RX_BD_RING_SIZE  8
#define TX_BD_RING_SIZE  8
#define MAX_RXBUF_LOG2		11
#define MAX_RXBUF_LEN		(1 << MAX_RXBUF_LOG2)

/* Common BD flags */
#define BD_LAST			0x0800

/* Rx BD status flags */
#define RxBD_EMPTY		0x8000
#define RxBD_LAST		BD_LAST
#define RxBD_FIRST		0x0400
#define RxBD_PHYS_ERR		0x0008
#define RxBD_SIZE_ERR		0x0004
#define RxBD_ERROR		(RxBD_PHYS_ERR | RxBD_SIZE_ERR)

/* Tx BD status flags */
#define TxBD_READY		0x8000
#define TxBD_LAST		BD_LAST


#define FMBM_RCFG_EN            0x80000000 /* port is enabled to receive data */
#define FMBM_TCFG_EN            0x80000000 /* port is enabled to transmit data */


#define CONFIG_SYS_FM_MURAM_SIZE 0x28000

#define CONFIG_SYS_FSL_FM1_OFFSET 0x400000
#define CONFIG_SYS_IMMR 0xfe000000
#define CONFIG_SYS_FSL_FM1_ADDR (CONFIG_SYS_IMMR + CONFIG_SYS_FSL_FM1_OFFSET)
#define CONFIG_SYS_FSL_FM1_MURAM_ADDR CONFIG_SYS_FSL_FM1_ADDR
#define CONFIG_SYS_FM1_DTSEC1_ADDR (CONFIG_SYS_FSL_FM1_ADDR + 0xe0000)
#define CONFIG_SYS_FM1_EMI1_MDIO_ADDR (CONFIG_SYS_FSL_FM1_ADDR + 0xe1120)

#define CONFIG_SYS_FM1_DTSEC3_ADDR (CONFIG_SYS_FSL_FM1_ADDR + 0xe4000)


#define FM_MURAM_RES_SIZE 0x01000

struct fm_muram muram;

uint32_t fm_muram_base()
{
    return muram.base;
}

uint32_t fm_muram_alloc(uint32_t size, uint32_t align)
{
    uint32_t ret;
    uint32_t align_mask, off;
    uint32_t save;

    align_mask = align - 1;
    save = muram.alloc;

    off = save & align_mask;
    if (off != 0)
        muram.alloc += (align - off);
    off = size & align_mask;
    if (off != 0)
        size += (align - off);
    if ((muram.alloc + size) >= muram.top) {
        muram.alloc = save;
        printf("%s: run out of ram.\n", __func__);
    }

    ret = muram.alloc;
    muram.alloc += size;
    memset((void *)ret, 0, size);

    return ret;
}

static void fm_init_muram(int fm_idx, void *reg)
{
    uint32_t base = (uint32_t)reg;
    printf("MURAM base = %p\n", reg);

    muram.base = base;
    muram.size = CONFIG_SYS_FM_MURAM_SIZE;
    muram.alloc = base + FM_MURAM_RES_SIZE;
    muram.top = base + CONFIG_SYS_FM_MURAM_SIZE;
}



char *name = "FM1_DTSEC3";

void udelay(unsigned long usec) {
    (void) usec;
    //TODO
    for (int i; i<0x1000000; i++) {
        usec ++;
    }
}

static int fm_eth_send(struct fm_eth *fm_eth, void *buf, int len)
{
    struct fm_port_global_pram *pram;
    struct fm_port_bd *txbd, *txbd_base;
    uint16_t offset_in;
    int i;

    pram = fm_eth->tx_pram;
    txbd = fm_eth->cur_txbd;

    /* find one empty TxBD */
    for (i = 0; txbd->status & TxBD_READY; i++) {
        udelay(100);
        if (i > 0x1000) {
            printf("%s: Tx buffer not ready\n", name);
            return 0;
        }
    }
    /* setup TxBD */
    txbd->buf_ptr_hi = 0;
    txbd->buf_ptr_lo = pok_virt_to_phys(buf);
    txbd->len = len;
    sync();
    txbd->status = TxBD_READY | TxBD_LAST;
    sync();

    /* update TxQD, let RISC to send the packet */
    offset_in = in_be16(&pram->txqd.offset_in);
    offset_in += sizeof(struct fm_port_bd);
    if (offset_in >= in_be16(&pram->txqd.bd_ring_size))
        offset_in = 0;
    out_be16(&pram->txqd.offset_in, offset_in);
    sync();

    /* wait for buffer to be transmitted */
    for (i = 0; txbd->status & TxBD_READY; i++) {
        udelay(100);
        /*
        if (i > 0x10000) {
            printf("%s: Tx error\n", name);
            return 0;

        }*/
        if (i % 10000000 == 0) {
            printf("offset_out = 0x%x\n", pram->txqd.offset_out);
            printf("offset_in = 0x%x\n", pram->txqd.offset_in);
        }
    }

    printf("NO error\n");
    /* advance the TxBD */
    txbd++;
    txbd_base = (struct fm_port_bd *)fm_eth->tx_bd_ring;
    if (txbd >= (txbd_base + TX_BD_RING_SIZE))
        txbd = txbd_base;
    /* update current txbd */
    fm_eth->cur_txbd = (void *)txbd;

    return 1;
}

int net_process_received_packet(void *data, int len) {
    printf("====================\n");
    hexdump(data, len);
    printf("====================\n");
    return 0;
}

static int fm_eth_recv(struct fm_eth *fm_eth)
{
    struct fm_port_global_pram *pram;
    struct fm_port_bd *rxbd, *rxbd_base;
    u16 status, len;
    u8 *data;
    u16 offset_out;
    int ret = 1;

    pram = fm_eth->rx_pram;
    rxbd = fm_eth->cur_rxbd;
    status = rxbd->status;

    printf("%s: status 0x%x\n", __func__, status);
    while (!(status & RxBD_EMPTY)) {
        if (!(status & RxBD_ERROR)) {
            //XXX phys_to_virt?
            data = (u8 *)pok_phys_to_virt(rxbd->buf_ptr_lo);
            len = rxbd->len;
            net_process_received_packet(data, len);
        } else {
            printf("%s: Rx error\n", name);
            ret = 0;
        }

        /* clear the RxBDs */
        rxbd->status = RxBD_EMPTY;
        rxbd->len = 0;
        sync();

        /* advance RxBD */
        rxbd++;
        rxbd_base = (struct fm_port_bd *)fm_eth->rx_bd_ring;
        if (rxbd >= (rxbd_base + RX_BD_RING_SIZE))
            rxbd = rxbd_base;
        /* read next status */
        status = rxbd->status;

        /* update RxQD */
        offset_out = in_be16(&pram->rxqd.offset_out);
        offset_out += sizeof(struct fm_port_bd);
        if (offset_out >= in_be16(&pram->rxqd.bd_ring_size))
            offset_out = 0;
        out_be16(&pram->rxqd.offset_out, offset_out);
        sync();
    }
    fm_eth->cur_rxbd = (void *)rxbd;

    return ret;
}

char tx_buffer_pseudo_malloc[sizeof(struct fm_port_bd) * TX_BD_RING_SIZE];
char rx_ring_pseudo_malloc  [sizeof(struct fm_port_bd) * RX_BD_RING_SIZE];
char rx_pool_pseudo_malloc  [MAX_RXBUF_LEN * RX_BD_RING_SIZE];

static int fm_eth_tx_port_parameter_init(struct fm_eth *fm_eth)
{
    struct fm_port_global_pram *pram;
    uint32_t pram_page_offset;
    void *tx_bd_ring_base;
    struct fm_port_bd *txbd;
    struct fm_port_qd *txqd;
    struct fm_bmi_tx_port *bmi_tx_port = fm_eth->tx_port;
    int i;

    /* alloc global parameter ram at MURAM */
    pram = (struct fm_port_global_pram *)fm_muram_alloc(FM_PRAM_SIZE,
                                                        FM_PRAM_ALIGN);
    if (!pram) {
        printf("%s: No muram for Tx global parameter\n", __func__);
        return 0;
    }
    fm_eth->tx_pram = pram;

    /* parameter page offset to MURAM */
    pram_page_offset = (uintptr_t)pram - fm_muram_base();

    /* enable global mode- snooping data buffers and BDs */
    out_be32(&pram->mode, PRAM_MODE_GLOBAL);

    /* init the Tx queue descriptor pionter */
    out_be32(&pram->txqd_ptr, pram_page_offset + 0x40);

    /* alloc Tx buffer descriptors from main memory */
    tx_bd_ring_base = tx_buffer_pseudo_malloc;
    if (!tx_bd_ring_base)
        return 0;

    memset(tx_bd_ring_base, 0, sizeof(struct fm_port_bd)
            * TX_BD_RING_SIZE);
    /* save it to fm_eth */
    fm_eth->tx_bd_ring = tx_bd_ring_base;
    fm_eth->cur_txbd = tx_bd_ring_base;

    /* init Tx BDs ring */
    txbd = (struct fm_port_bd *)tx_bd_ring_base;
    for (i = 0; i < TX_BD_RING_SIZE; i++) {
        out_be16(&txbd->status, TxBD_LAST);
        out_be16(&txbd->len, 0);
        out_be16(&txbd->buf_ptr_hi, 0);
        out_be32(&txbd->buf_ptr_lo, 0);
        txbd++;
    }

    /* set the Tx queue decriptor */
    txqd = &pram->txqd;
    out_be16(&txqd->bd_ring_base_hi, 0);
    txqd->bd_ring_base_lo = pok_virt_to_phys(tx_bd_ring_base);
    out_be16(&txqd->bd_ring_size, sizeof(struct fm_port_bd)
            * TX_BD_RING_SIZE);
    out_be16(&txqd->offset_in, 0);
    out_be16(&txqd->offset_out, 0);

    /* set IM parameter ram pointer to Tx Confirmation Frame Queue ID */
    out_be32(&bmi_tx_port->fmbm_tcfqid, pram_page_offset);

    return 1;
}

static int fm_eth_rx_port_parameter_init(struct fm_eth *fm_eth)
{
    struct fm_port_global_pram *pram;
    u32 pram_page_offset;
    void *rx_bd_ring_base;
    void *rx_buf_pool;
    struct fm_port_bd *rxbd;
    struct fm_port_qd *rxqd;
    struct fm_bmi_rx_port *bmi_rx_port = fm_eth->rx_port;
    int i;

    /* alloc global parameter ram at MURAM */
    pram = (struct fm_port_global_pram *)fm_muram_alloc(FM_PRAM_SIZE,
                                                        FM_PRAM_ALIGN);
    fm_eth->rx_pram = pram;

    /* parameter page offset to MURAM */
    pram_page_offset = (u32)pram - fm_muram_base(0);

    /* enable global mode- snooping data buffers and BDs */
    pram->mode = PRAM_MODE_GLOBAL;

    /* init the Rx queue descriptor pionter */
    pram->rxqd_ptr = pram_page_offset + 0x20;

    /* set the max receive buffer length, power of 2 */
    out_be16(&pram->mrblr, MAX_RXBUF_LOG2);

    /* alloc Rx buffer descriptors from main memory */
    rx_bd_ring_base = rx_ring_pseudo_malloc;
    if (!rx_bd_ring_base)
        return 0;
    memset(rx_bd_ring_base, 0, sizeof(struct fm_port_bd)
            * RX_BD_RING_SIZE);

    /* alloc Rx buffer from main memory */
    rx_buf_pool = rx_pool_pseudo_malloc;
    if (!rx_buf_pool)
        return 0;
    memset(rx_buf_pool, 0, MAX_RXBUF_LEN * RX_BD_RING_SIZE);

    /* save them to fm_eth */
    fm_eth->rx_bd_ring = rx_bd_ring_base;
    fm_eth->cur_rxbd = rx_bd_ring_base;
    fm_eth->rx_buf = rx_buf_pool;

    /* init Rx BDs ring */
    rxbd = (struct fm_port_bd *)rx_bd_ring_base;
    for (i = 0; i < RX_BD_RING_SIZE; i++) {
        rxbd->status = RxBD_EMPTY;
        rxbd->len = 0;
        rxbd->buf_ptr_hi = 0;
        rxbd->buf_ptr_lo = pok_virt_to_phys(rx_buf_pool + i * MAX_RXBUF_LEN);
        rxbd++;
    }
    printf("rx offset_in  addr %p\n", &pram->rxqd.offset_out);
    printf("rx offset_out addr %p\n", &pram->rxqd.offset_out);

    /* set the Rx queue descriptor */
    rxqd = &pram->rxqd;
    out_be16(&rxqd->gen, 0);
    out_be16(&rxqd->bd_ring_base_hi, 0);
    rxqd->bd_ring_base_lo = pok_virt_to_phys(rx_bd_ring_base);
    out_be16(&rxqd->bd_ring_size, sizeof(struct fm_port_bd)
            * RX_BD_RING_SIZE);
    out_be16(&rxqd->offset_in, 0);
    out_be16(&rxqd->offset_out, 0);

    /* set IM parameter ram pointer to Rx Frame Queue ID */
    out_be32(&bmi_rx_port->fmbm_rfqid, pram_page_offset);

    return 1;
}

static void fmc_tx_port_graceful_stop_disable(struct fm_eth *fm_eth)
{
    struct fm_port_global_pram *pram;

    pram = fm_eth->tx_pram;
    /* re-enable transmission of frames */
    clrbits_be32(&pram->mode, PRAM_MODE_GRACEFUL_STOP);
    sync();
}

static void dtsec_enable_mac(struct dtsec *regs)
{
    /* enable Rx/Tx MAC */
    setbits_be32(&regs->maccfg1, MACCFG1_RXTX_EN);

    /* clear the graceful Rx stop */
    clrbits_be32(&regs->rctrl, RCTRL_GRS);

    /* clear the graceful Tx stop */
    clrbits_be32(&regs->tctrl, TCTRL_GTS);
}

static void dtsec_get_mac_addr(struct dtsec *regs, uint8_t *mac_addr)
{
    uint32_t mac1, mac2;
    mac1 = in_be32(&regs->macstnaddr1);
    mac2 = in_be32(&regs->macstnaddr2);


    mac_addr[0] = mac2>>16 & 0xff;
    mac_addr[1] = mac2>>24 & 0xff;
    mac_addr[2] = mac1     & 0xff;
    mac_addr[3] = mac1>>8  & 0xff;
    mac_addr[4] = mac1>>16 & 0xff;
    mac_addr[5] = mac1>>24 & 0xff;
}

static void fm_eth_open(struct fm_eth *fm_eth, void *regs)
{
    /* enable bmi Rx port */
    setbits_be32(&fm_eth->rx_port->fmbm_rcfg, FMBM_RCFG_EN);
    /* enable MAC rx/tx port */
    dtsec_enable_mac(regs);
    /* enable bmi Tx port */
    setbits_be32(&fm_eth->tx_port->fmbm_tcfg, FMBM_TCFG_EN);
    /* re-enable transmission of frame */
    fmc_tx_port_graceful_stop_disable(fm_eth);
}



#define FM_HARDWARE_PORTS 0x80000
#define DTSEC3_RX_PORT    0x0a
#define DTSEC3_TX_PORT    0x2a
#define DTSEC4_RX_PORT    0x0b
#define DTSEC4_TX_PORT    0x2b

struct fm_eth dtsec3;
struct fm_eth dtsec4;
struct fm_eth *current;
uint8_t macaddr[6];

void p3041_init(void)
{
    dtsec3.rx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC3_RX_PORT*0x1000);
    dtsec3.tx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC3_TX_PORT*0x1000);
    dtsec4.rx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC4_RX_PORT*0x1000);
    dtsec4.tx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC4_TX_PORT*0x1000);
    current = &dtsec3;

    dtsec_get_mac_addr((void *)CONFIG_SYS_FM1_DTSEC3_ADDR, macaddr);
    fm_init_muram(0, (void *)CONFIG_SYS_FSL_FM1_MURAM_ADDR);

    /* qmi_common and bmi_common are initialized in this memory by u-boot */
    muram.alloc = muram.base + 0x21000;

    fm_eth_rx_port_parameter_init(current);
    fm_eth_tx_port_parameter_init(current);

    {
        struct fm_port_global_pram *dtsec3_rx_pram = (void *) muram.base + 0x21000;
        struct fm_port_global_pram *dtsec3_tx_pram = (void *) muram.base + 0x21100;

        printf("UBOOT rx offset_in:  [%p]= 0x%x\n",
                &dtsec3_rx_pram->rxqd.offset_in,
                dtsec3_rx_pram->rxqd.offset_in
                );
        printf("UBOOT rx offset_out: [%p]= 0x%x\n",
                &dtsec3_rx_pram->rxqd.offset_out,
                dtsec3_rx_pram->rxqd.offset_out
                );

        printf("UBOOT tx offset_in:  [%p]= 0x%x\n",
                &dtsec3_tx_pram->txqd.offset_in,
                dtsec3_tx_pram->txqd.offset_in
                );
        printf("UBOOT tx offset_out: [%p]= 0x%x\n",
                &dtsec3_tx_pram->txqd.offset_out,
                dtsec3_tx_pram->txqd.offset_out
                );
    }

    fm_eth_open(current, (void *)CONFIG_SYS_FM1_DTSEC3_ADDR);
    printf("dtsec3 bmi[tcfqid] %lx\n", in_be32(&dtsec3.tx_port->fmbm_tcfqid));

#if 0
    while (1) {
        fm_eth_recv(current);
        for (int i = 0; i<10000000; i++) {
            udelay(100);
            /*
            if (i > 0x10000) {
                printf("%s: Tx error\n", name);
                return 0;

            }*/
        }
        printf("A\n");
    }
#endif
}


static pok_bool_t send_frame(
        char *buffer,
        size_t size,
        pok_network_buffer_callback_t callback,
        void *callback_arg)
{
    //pok_network_sg_list_t sg_list[1] = {{.buffer=buffer, .size=size}};
    //return send_frame_gather(sg_list, 1, callback, callback_arg);
    fm_eth_send(current, buffer, size);

    return 1;
}


pok_bool_t dummy_send_frame_gather(const pok_network_sg_list_t *sg_list,
                                size_t sg_list_len,
                                pok_network_buffer_callback_t callback,
                                void *callback_arg)
{
    printf("%s\n", __func__);
    return 0;
}

void dummy_set_packet_received_callback(void (*f)(const char *, size_t)) {
    //printf("%s\n", __func__);
}
void dummy_reclaim_send_buffers(void) {
    //printf("%s\n", __func__);
}
void dummy_reclaim_receive_buffers(void) {
    //printf("%s\n", __func__);
}
void dummy_flush_send(void) {
    //printf("%s\n", __func__);
}

static const pok_network_driver_ops_t driver_ops = {
    .send_frame = send_frame,
    .send_frame_gather =            dummy_send_frame_gather,
    .set_packet_received_callback = dummy_set_packet_received_callback,
    .reclaim_send_buffers =         dummy_reclaim_send_buffers,
    .reclaim_receive_buffers =      dummy_reclaim_receive_buffers,
    .flush_send =                   dummy_flush_send,
};

pok_network_driver_device_t pok_network_p3041_device = {
    .ops = &driver_ops,
    .mac = macaddr
};

