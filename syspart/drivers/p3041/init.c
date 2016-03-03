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

#ifdef __PPC__
#include <stdio.h>
#include <ioports.h>
#include <string.h>
#include <net/network.h>
#include <memory.h>
#include "fm.h"

#define DRV_NAME "P3041-NET"

struct {
    unsigned send_last_seen; //Truly points to first nonseen element
    unsigned receive__last_seen;
    struct send_callback send_callbacks[TX_BD_RING_SIZE];
} current_state;

char tx_buffer_pseudo_malloc[sizeof(struct fm_port_bd) * TX_BD_RING_SIZE];
char rx_ring_pseudo_malloc  [sizeof(struct fm_port_bd) * RX_BD_RING_SIZE];
char rx_pool_pseudo_malloc  [MAX_RXBUF_LEN * RX_BD_RING_SIZE];

struct fm_eth dtsec3;
struct fm_eth dtsec4;
struct fm_eth *current;

struct {
    uint8_t macaddr[6];
    void (*packet_received_callback)(const char *, size_t);
}internal_device_struct;

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

    muram.base = base;
    muram.size = CONFIG_SYS_FM_MURAM_SIZE;
    muram.alloc = base + FM_MURAM_RES_SIZE;
    muram.top = base + CONFIG_SYS_FM_MURAM_SIZE;
}



static int fm_eth_send(struct fm_eth *fm_eth,
                       void *buf,
                       int len,
                       pok_network_buffer_callback_t callback,
                       void *callback_arg
                      )
{
    struct fm_port_global_pram *pram;
    struct fm_port_bd *txbd, *txbd_base;
    uint16_t offset_in;
    int c_idx;

    pram = fm_eth->tx_pram;
    txbd = fm_eth->cur_txbd;

    if (txbd->status & TxBD_READY) {
        //This is not a typo. If status[READY] = 1, then we can't use this buffer now
        printf("%s: Tx buffer not ready\n", DRV_NAME);
        return 0;
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

    c_idx = offset_in / sizeof(struct fm_port_bd);
    current_state.send_callbacks[c_idx].func = callback;
    current_state.send_callbacks[c_idx].argument = callback_arg;

    offset_in += sizeof(struct fm_port_bd);
    if (offset_in >= in_be16(&pram->txqd.bd_ring_size))
        offset_in = 0;

    // XXX This is "flushing"
    out_be16(&pram->txqd.offset_in, offset_in);
    sync();

    /* advance the TxBD */
    txbd++;
    txbd_base = (struct fm_port_bd *)fm_eth->tx_bd_ring;
    if (txbd >= (txbd_base + TX_BD_RING_SIZE))
        txbd = txbd_base;
    /* update current txbd */
    fm_eth->cur_txbd = (void *)txbd;

    return 1;
}

static void reclaim_send_buffers(void)
{
    //send_last_seen is initialized by zero as member of static global structure

    struct fm_eth *fm_eth = current;
    size_t bd_size = sizeof(struct fm_port_bd);
    size_t bd_ring_size = bd_size * TX_BD_RING_SIZE;

    // Sets by NIC
    uint16_t offset_out = in_be16(&fm_eth->tx_pram->txqd.offset_out);

    while ((offset_out + bd_ring_size - current_state.send_last_seen * bd_size)%bd_ring_size >= bd_size) {
        uint16_t idx = current_state.send_last_seen;
        struct send_callback cb = current_state.send_callbacks[idx];
        cb.func(cb.argument);
        current_state.send_last_seen = (current_state.send_last_seen + 1) % TX_BD_RING_SIZE;
    }

}

static int fm_eth_recv(struct fm_eth *fm_eth)
{
    struct fm_port_global_pram *pram;
    struct fm_port_bd *rxbd, *rxbd_base;
    uint16_t status, len;
    char *data;
    uint16_t offset_out;
    int ret = 1;

    pram = fm_eth->rx_pram;
    rxbd = fm_eth->cur_rxbd;
    status = rxbd->status;

    while (!(status & RxBD_EMPTY)) {
        if (!(status & RxBD_ERROR)) {
            //XXX phys_to_virt?
            data = (void *)pok_phys_to_virt(rxbd->buf_ptr_lo);
            len = rxbd->len;
            internal_device_struct.packet_received_callback(data, len);
        } else {
            printf("%s: Rx error\n", DRV_NAME);
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

static void reclaim_receive_buffers(void)
{
    fm_eth_recv(current);
}

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
    //fm_eth->rx_buf = rx_buf_pool;

    /* init Rx BDs ring */
    rxbd = (struct fm_port_bd *)rx_bd_ring_base;
    for (i = 0; i < RX_BD_RING_SIZE; i++) {
        rxbd->status = RxBD_EMPTY;
        rxbd->len = 0;
        rxbd->buf_ptr_hi = 0;
        rxbd->buf_ptr_lo = pok_virt_to_phys(rx_buf_pool + i * MAX_RXBUF_LEN);
        rxbd++;
    }

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



void p3041_init(void)
{
    dtsec3.rx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC3_RX_PORT*0x1000);
    dtsec3.tx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC3_TX_PORT*0x1000);
    dtsec4.rx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC4_RX_PORT*0x1000);
    dtsec4.tx_port = (void *)(CONFIG_SYS_FSL_FM1_ADDR + FM_HARDWARE_PORTS + DTSEC4_TX_PORT*0x1000);
    current = &dtsec3;

    dtsec_get_mac_addr((void *)CONFIG_SYS_FM1_DTSEC3_ADDR, internal_device_struct.macaddr);
    fm_init_muram(0, (void *)CONFIG_SYS_FSL_FM1_MURAM_ADDR);

    /* qmi_common and bmi_common are initialized in this memory by u-boot */
    muram.alloc = muram.base + 0x21000;

    fm_eth_rx_port_parameter_init(current);
    fm_eth_tx_port_parameter_init(current);

    fm_eth_open(current, (void *)CONFIG_SYS_FM1_DTSEC3_ADDR);
}


static pok_bool_t send_frame(
        char *buffer,
        size_t size,
        pok_network_buffer_callback_t callback,
        void *callback_arg)
{
    //pok_network_sg_list_t sg_list[1] = {{.buffer=buffer, .size=size}};
    //return send_frame_gather(sg_list, 1, callback, callback_arg);
    fm_eth_send(current, buffer, size, callback, callback_arg);

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

void set_packet_received_callback(void (*f)(const char *, size_t)) {
    internal_device_struct.packet_received_callback = f;
}

void dummy_flush_send(void) {
}

static const pok_network_driver_ops_t driver_ops = {
    .send_frame = send_frame,
    .send_frame_gather =            dummy_send_frame_gather,
    .set_packet_received_callback = set_packet_received_callback,
    .reclaim_send_buffers =         reclaim_send_buffers,
    .reclaim_receive_buffers =      reclaim_receive_buffers,
    .flush_send =                   dummy_flush_send,
};

pok_network_driver_device_t pok_network_p3041_device = {
    .ops = &driver_ops,
    .mac = internal_device_struct.macaddr
};

#endif
