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

/* Rx/Tx queue descriptor */
struct fm_port_qd {
    uint16_t gen;
    uint16_t bd_ring_base_hi;
    uint32_t bd_ring_base_lo;
    uint16_t bd_ring_size;
    uint16_t offset_in;
    uint16_t offset_out;
    uint16_t res0;
    uint32_t res1[0x4];
};

/* IM global parameter RAM */
struct fm_port_global_pram {
    uint32_t mode;      /* independent mode register */
    uint32_t rxqd_ptr;  /* Rx queue descriptor pointer */
    uint32_t txqd_ptr;  /* Tx queue descriptor pointer */
    uint16_t mrblr;     /* max Rx buffer length */
    uint16_t rxqd_bsy_cnt;      /* RxQD busy counter, should be cleared */
    uint32_t res0[0x4];
    struct fm_port_qd rxqd;     /* Rx queue descriptor */
    struct fm_port_qd txqd;     /* Tx queue descriptor */
    uint32_t res1[0x28];
};

#define FM_PRAM_SIZE  sizeof(struct fm_port_global_pram)
#define FM_PRAM_ALIGN 256
#define PRAM_MODE_GLOBAL	0x20000000
#define PRAM_MODE_GRACEFUL_STOP	0x00800000


/* Fman ethernet private struct */
struct fm_eth {
    int fm_index;                       /* Fman index */
    //uint32_t num;                     /* 0..n-1 for give type */
    struct fm_bmi_tx_port *tx_port;
    struct fm_bmi_rx_port *rx_port;
    //enum fm_eth_type type;            /* 1G or 10G ethernet */
    //phy_interface_t enet_if;
    //struct fsl_enet_mac *mac; /* MAC controller */
    //struct mii_dev *bus;
    //struct phy_device *phydev;
    //int phyaddr;
    //struct eth_device *dev;
    //int max_rx_len;
    struct fm_port_global_pram *rx_pram; /* Rx parameter table */
    struct fm_port_global_pram *tx_pram; /* Tx parameter table */
    void *rx_bd_ring;         /* Rx BD ring base */
    void *cur_rxbd;                   /* current Rx BD */
    void *rx_buf;                     /* Rx buffer base */

    void *tx_bd_ring;           /* Tx BD ring base */
    void *cur_txbd;                     /* current Tx BD */
};

typedef struct fm_bmi_common {
    uint32_t    fmbm_init;      /* BMI initialization */
    uint32_t    fmbm_cfg1;      /* BMI configuration1 */
    uint32_t    fmbm_cfg2;      /* BMI configuration2 */
    uint32_t    res0[0x5];
    uint32_t    fmbm_ievr;      /* interrupt event register */
    uint32_t    fmbm_ier;       /* interrupt enable register */
    uint32_t    fmbm_ifr;       /* interrupt force register */
    uint32_t    res1[0x5];
    uint32_t    fmbm_arb[0x8];  /* BMI arbitration */
    uint32_t    res2[0x28];
    uint32_t    fmbm_gde;       /* global debug enable */
    uint32_t    fmbm_pp[0x3f];  /* BMI port parameters */
    uint32_t    res3;
    uint32_t    fmbm_pfs[0x3f]; /* BMI port FIFO size */
    uint32_t    res4;
    uint32_t    fmbm_ppid[0x3f];/* port partition ID */
} fm_bmi_common_t;

typedef struct fm_qmi_common {
    uint32_t    fmqm_gc;        /* general configuration register */
    uint32_t    res0;
    uint32_t    fmqm_eie;       /* error interrupt event register */
    uint32_t    fmqm_eien;      /* error interrupt enable register */
    uint32_t    fmqm_eif;       /* error interrupt force register */
    uint32_t    fmqm_ie;        /* interrupt event register */
    uint32_t    fmqm_ien;       /* interrupt enable register */
    uint32_t    fmqm_if;        /* interrupt force register */
    uint32_t    fmqm_gs;        /* global status register */
    uint32_t    fmqm_ts;        /* task status register */
    uint32_t    fmqm_etfc;      /* enqueue total frame counter */
    uint32_t    fmqm_dtfc;      /* dequeue total frame counter */
    uint32_t    fmqm_dc0;       /* dequeue counter 0 */
    uint32_t    fmqm_dc1;       /* dequeue counter 1 */
    uint32_t    fmqm_dc2;       /* dequeue counter 2 */
    uint32_t    fmqm_dc3;       /* dequeue counter 3 */
    uint32_t    fmqm_dfnoc;     /* dequeue FQID not override counter */
    uint32_t    fmqm_dfcc;      /* dequeue FQID from context counter */
    uint32_t    fmqm_dffc;      /* dequeue FQID from FD counter */
    uint32_t    fmqm_dcc;       /* dequeue confirm counter */
    uint32_t    res1[0xc];
    uint32_t    fmqm_dtrc;      /* debug trap configuration register */
    uint32_t    fmqm_efddd;     /* enqueue frame descriptor dynamic debug */
    uint32_t    res3[0x2];
    uint32_t    res4[0xdc];     /* missing debug regs */
} fm_qmi_common_t;

typedef struct fm_bmi {
    uint8_t     res[1024];
} fm_bmi_t;

typedef struct fm_qmi {
    uint8_t     res[1024];
} fm_qmi_t;

struct fm_bmi_rx_port {
    uint32_t fmbm_rcfg; /* Rx configuration */
    uint32_t fmbm_rst;  /* Rx status */
    uint32_t fmbm_rda;  /* Rx DMA attributes */
    uint32_t fmbm_rfp;  /* Rx FIFO parameters */
    uint32_t fmbm_rfed; /* Rx frame end data */
    uint32_t fmbm_ricp; /* Rx internal context parameters */
    uint32_t fmbm_rim;  /* Rx internal margins */
    uint32_t fmbm_rebm; /* Rx external buffer margins */
    uint32_t fmbm_rfne; /* Rx frame next engine */
    uint32_t fmbm_rfca; /* Rx frame command attributes */
    uint32_t fmbm_rfpne;        /* Rx frame parser next engine */
    uint32_t fmbm_rpso; /* Rx parse start offset */
    uint32_t fmbm_rpp;  /* Rx policer profile */
    uint32_t fmbm_rccb; /* Rx coarse classification base */
    uint32_t res1[0x2];
    uint32_t fmbm_rprai[0x8];   /* Rx parse results array Initialization */
    uint32_t fmbm_rfqid;                /* Rx frame queue ID */
    uint32_t fmbm_refqid;       /* Rx error frame queue ID */
    uint32_t fmbm_rfsdm;                /* Rx frame status discard mask */
    uint32_t fmbm_rfsem;                /* Rx frame status error mask */
    uint32_t fmbm_rfene;                /* Rx frame enqueue next engine */
    uint32_t res2[0x23];
    uint32_t fmbm_ebmpi[0x8];   /* buffer manager pool information */
    uint32_t fmbm_acnt[0x8];    /* allocate counter */
    uint32_t res3[0x8];
    uint32_t fmbm_cgm[0x8];     /* congestion group map */
    uint32_t fmbm_mpd;          /* BMan pool depletion */
    uint32_t res4[0x1F];
    uint32_t fmbm_rstc;         /* Rx statistics counters */
    uint32_t fmbm_rfrc;         /* Rx frame counters */
    uint32_t fmbm_rfbc;         /* Rx bad frames counter */
    uint32_t fmbm_rlfc;         /* Rx large frames counter */
    uint32_t fmbm_rffc;         /* Rx filter frames counter */
    uint32_t fmbm_rfdc;         /* Rx frame discard counter */
    uint32_t fmbm_rfldec;       /* Rx frames list DMA error counter */
    uint32_t fmbm_rodc;         /* Rx out of buffers discard counter */
    uint32_t fmbm_rbdc;         /* Rx buffers deallocate counter */
    uint32_t res5[0x17];
    uint32_t fmbm_rpc;          /* Rx performance counters */
    uint32_t fmbm_rpcp;         /* Rx performance count parameters */
    uint32_t fmbm_rccn;         /* Rx cycle counter */
    uint32_t fmbm_rtuc;         /* Rx tasks utilization counter */
    uint32_t fmbm_rrquc;                /* Rx receive queue utilization counter */
    uint32_t fmbm_rduc;         /* Rx DMA utilization counter */
    uint32_t fmbm_rfuc;         /* Rx FIFO utilization counter */
    uint32_t fmbm_rpac;         /* Rx pause activation counter */
    uint32_t res6[0x18];
    uint32_t fmbm_rdbg;         /* Rx debug configuration */
};
struct fm_bmi_tx_port {
    uint32_t fmbm_tcfg; /* Tx configuration */
    uint32_t fmbm_tst;  /* Tx status */
    uint32_t fmbm_tda;  /* Tx DMA attributes */
    uint32_t fmbm_tfp;  /* Tx FIFO parameters */
    uint32_t fmbm_tfed; /* Tx frame end data */
    uint32_t fmbm_ticp; /* Tx internal context parameters */
    uint32_t fmbm_tfne; /* Tx frame next engine */
    uint32_t fmbm_tfca; /* Tx frame command attributes */
    uint32_t fmbm_tcfqid;/* Tx confirmation frame queue ID */
    uint32_t fmbm_tfeqid;/* Tx error frame queue ID */
    uint32_t fmbm_tfene;        /* Tx frame enqueue next engine */
    uint32_t fmbm_trlmts;/* Tx rate limiter scale */
    uint32_t fmbm_trlmt;        /* Tx rate limiter */
    uint32_t res0[0x73];
    uint32_t fmbm_tstc; /* Tx statistics counters */
    uint32_t fmbm_tfrc; /* Tx frame counter */
    uint32_t fmbm_tfdc; /* Tx frames discard counter */
    uint32_t fmbm_tfledc;/* Tx frame length error discard counter */
    uint32_t fmbm_tfufdc;/* Tx frame unsupported format discard counter */
    uint32_t fmbm_tbdc; /* Tx buffers deallocate counter */
    uint32_t res1[0x1a];
    uint32_t fmbm_tpc;  /* Tx performance counters */
    uint32_t fmbm_tpcp; /* Tx performance count parameters */
    uint32_t fmbm_tccn; /* Tx cycle counter */
    uint32_t fmbm_ttuc; /* Tx tasks utilization counter */
    uint32_t fmbm_ttcquc;/* Tx transmit confirm queue utilization counter */
    uint32_t fmbm_tduc; /* Tx DMA utilization counter */
    uint32_t fmbm_tfuc; /* Tx FIFO utilization counter */
    uint32_t res2[0x19];
    uint32_t fmbm_tdcfg;        /* Tx debug configuration */
};


typedef struct fm_parser {
    uint8_t res[1024];
} fm_parser_t;

typedef struct fm_policer {
    uint8_t res[4*1024];
} fm_policer_t;

typedef struct fm_keygen {
    uint8_t res[4*1024];
} fm_keygen_t;


typedef struct fm_dma {
    uint32_t fmdmsr;            /* status register */
    uint32_t fmdmmr;            /* mode register */
    uint32_t fmdmtr;            /* bus threshold register */
    uint32_t fmdmhy;            /* bus hysteresis register */
    uint32_t fmdmsetr;  /* SOS emergency threshold register */
    uint32_t fmdmtah;   /* transfer bus address high register */
    uint32_t fmdmtal;   /* transfer bus address low register */
    uint32_t fmdmtcid;  /* transfer bus communication ID register */
    uint32_t fmdmra;            /* DMA bus internal ram address register */
    uint32_t fmdmrd;            /* DMA bus internal ram data register */
    uint32_t res0[0xb];
    uint32_t fmdmdcr;   /* debug counter */
    uint32_t fmdmemsr;  /* emrgency smoother register */
    uint32_t res1;
    uint32_t fmdmplr[32];       /* FM DMA PID-LIODN # register */
    uint32_t res[0x3c8];
} fm_dma_t;

typedef struct fm_fpm {
    uint32_t fpmtnc;            /* TNUM control */
    uint32_t fpmprc;            /* Port_ID control */
    uint32_t res0;
    uint32_t fpmflc;            /* flush control */
    uint32_t fpmdis1;   /* dispatch thresholds1 */
    uint32_t fpmdis2;   /* dispatch thresholds2 */
    uint32_t fmepi;             /* error pending interrupts */
    uint32_t fmrie;             /* rams interrupt enable */
    uint32_t fpmfcevent[0x4];/* FMan controller event 0-3 */
    uint32_t res1[0x4];
    uint32_t fpmfcmask[0x4];    /* FMan controller mask 0-3 */
    uint32_t res2[0x4];
    uint32_t fpmtsc1;   /* timestamp control1 */
    uint32_t fpmtsc2;   /* timestamp control2 */
    uint32_t fpmtsp;            /* time stamp */
    uint32_t fpmtsf;            /* time stamp fraction */
    uint32_t fpmrcr;            /* rams control and event */
    uint32_t res3[0x3];
    uint32_t fpmdrd[0x4];       /* data_ram data 0-3 */
    uint32_t res4[0xc];
    uint32_t fpmdra;            /* data ram access */
    uint32_t fm_ip_rev_1;       /* IP block revision 1 */
    uint32_t fm_ip_rev_2;       /* IP block revision 2 */
    uint32_t fmrstc;            /* reset command */
    uint32_t fmcld;             /* classifier debug control */
    uint32_t fmnpi;             /* normal pending interrupts */
    uint32_t res5;
    uint32_t fmfpee;            /* event and enable */
    uint32_t fpmcev[0x4];       /* CPU event 0-3 */
    uint32_t res6[0x4];
    uint32_t fmfp_ps[0x40];     /* port status */
    uint32_t res7[0x260];
    uint32_t fpmts[0x80];       /* task status */
    uint32_t res8[0xa0];
} fm_fpm_t;

typedef struct fm_imem {
    uint32_t iadd;              /* instruction address register */
    uint32_t idata;             /* instruction data register */
    uint32_t itcfg;             /* timing config register */
    uint32_t iready;            /* ready register */
    uint8_t  res[0xff0];
} fm_imem_t;

typedef struct fm_soft_parser {
    uint8_t     res[4*1024];
} fm_soft_parser_t;

typedef struct fm_dtsec {
    uint8_t     res[4*1024];
} fm_dtsec_t;

typedef struct fm_mdio {
    uint8_t     res0[0x120];
    uint32_t    miimcfg;        /* MII management configuration reg */
    uint32_t    miimcom;        /* MII management command reg */
    uint32_t    miimadd;        /* MII management address reg */
    uint32_t    miimcon;        /* MII management control reg */
    uint32_t    miimstat;       /* MII management status reg  */
    uint32_t    miimind;        /* MII management indication reg */
    uint8_t     res1[0x1000 - 0x138];
} fm_mdio_t;

typedef struct fm_10gec {
    uint8_t     res[4*1024];
} fm_10gec_t;

typedef struct fm_10gec_mdio {
    uint8_t     res[4*1024];
} fm_10gec_mdio_t;

typedef struct fm_1588 {
    uint8_t     res[4*1024];
} fm_1588_t;



#define TCTRL_GTS       0x00000020 /* Graceful transmit stop */
#define RCTRL_GRS       0x00000020 /* graceful receive stop */
#define MACCFG1_RX_EN   0x00000004 /* Rx enable */
#define MACCFG1_TX_EN   0x00000001 /* Tx enable */
#define MACCFG1_RXTX_EN (MACCFG1_RX_EN | MACCFG1_TX_EN)

struct dtsec {
    uint32_t tsec_id;	/* controller ID and version */
    uint32_t tsec_id2;	/* controller ID and configuration */
    uint32_t ievent;		/* interrupt event */
    uint32_t imask;		/* interrupt mask */
    uint32_t res0;
    uint32_t ecntrl;		/* ethernet control and configuration */
    uint32_t ptv;		/* pause time value */
    uint32_t tbipa;		/* TBI PHY address */
    uint32_t res1[8];
    uint32_t tctrl;		/* Transmit control register */
    uint32_t res2[3];
    uint32_t rctrl;		/* Receive control register */
    uint32_t res3[11];
    uint32_t igaddr[8];	/* Individual group address */
    uint32_t gaddr[8];	/* group address */
    uint32_t res4[16];
    uint32_t maccfg1;	/* MAC configuration register 1 */
    uint32_t maccfg2;	/* MAC configuration register 2 */
    uint32_t ipgifg;		/* inter-packet/inter-frame gap */
    uint32_t hafdup;		/* half-duplex control */
    uint32_t maxfrm;		/* Maximum frame size */
    uint32_t res5[3];
    uint32_t miimcfg;	/* MII management configuration */
    uint32_t miimcom;	/* MII management command */
    uint32_t miimadd;	/* MII management address */
    uint32_t miimcon;	/* MII management control */
    uint32_t miimstat;	/* MII management status */
    uint32_t miimind;	/* MII management indicator */
    uint32_t res6;
    uint32_t ifstat;		/* Interface status */
    uint32_t macstnaddr1;	/* MAC station address 1 */
    uint32_t macstnaddr2;	/* MAC station address 2 */
    uint32_t res7[46];
    /* transmit and receive counter */
    uint32_t r64;		/* Tx and Rx 64 bytes frame */
    uint32_t r127;		/* Tx and Rx 65 to 127 bytes frame */
    uint32_t r255;		/* Tx and Rx 128 to 255 bytes frame */
    uint32_t r511;		/* Tx and Rx 256 to 511 bytes frame */
    uint32_t r1k;		/* Tx and Rx 512 to 1023 bytes frame */
    uint32_t rmax;		/* Tx and Rx 1024 to 1518 bytes frame */
    uint32_t rmgv;		/* Tx and Rx 1519 to 1522 good VLAN frame */
    /* receive counters */
    uint32_t rbyt;		/* Receive byte counter */
    uint32_t rpkt;		/* Receive packet counter */
    uint32_t rfcs;		/* Receive FCS error */
    uint32_t rmca;		/* Receive multicast packet */
    uint32_t rbca;		/* Receive broadcast packet */
    uint32_t rxcf;		/* Receive control frame */
    uint32_t rxpf;		/* Receive pause frame */
    uint32_t rxuo;		/* Receive unknown OP code */
    uint32_t raln;		/* Receive alignment error */
    uint32_t rflr;		/* Receive frame length error */
    uint32_t rcde;		/* Receive code error */
    uint32_t rcse;		/* Receive carrier sense error */
    uint32_t rund;		/* Receive undersize packet */
    uint32_t rovr;		/* Receive oversize packet */
    uint32_t rfrg;		/* Receive fragments counter */
    uint32_t rjbr;		/* Receive jabber counter */
    uint32_t rdrp;		/* Receive drop counter */
    /* transmit counters */
    uint32_t tbyt;		/* Transmit byte counter */
    uint32_t tpkt;		/* Transmit packet */
    uint32_t tmca;		/* Transmit multicast packet */
    uint32_t tbca;		/* Transmit broadcast packet */
    uint32_t txpf;		/* Transmit pause control frame */
    uint32_t tdfr;		/* Transmit deferral packet */
    uint32_t tedf;		/* Transmit excessive deferral pkt */
    uint32_t tscl;		/* Transmit single collision pkt */
    uint32_t tmcl;		/* Transmit multiple collision pkt */
    uint32_t tlcl;		/* Transmit late collision pkt */
    uint32_t txcl;		/* Transmit excessive collision */
    uint32_t tncl;		/* Transmit total collision */
    uint32_t res8;
    uint32_t tdrp;		/* Transmit drop frame */
    uint32_t tjbr;		/* Transmit jabber frame */
    uint32_t tfcs;		/* Transmit FCS error */
    uint32_t txcf;		/* Transmit control frame */
    uint32_t tovr;		/* Transmit oversize frame */
    uint32_t tund;		/* Transmit undersize frame */
    uint32_t tfrg;		/* Transmit fragments frame */
    /* counter controls */
    uint32_t car1;		/* carry register 1 */
    uint32_t car2;		/* carry register 2 */
    uint32_t cam1;		/* carry register 1 mask */
    uint32_t cam2;		/* carry register 2 mask */
    uint32_t res9[80];
};


typedef struct ccsr_fman {
    uint8_t             muram[0x80000];
    fm_bmi_common_t     fm_bmi_common;
    fm_qmi_common_t     fm_qmi_common;
    uint8_t             res0[2048];
    struct {
        fm_bmi_t        fm_bmi;
        fm_qmi_t        fm_qmi;
        fm_parser_t     fm_parser;
        uint8_t         res[1024];
    } port[63];
    fm_policer_t        fm_policer;
    fm_keygen_t         fm_keygen;
    fm_dma_t            fm_dma;
    fm_fpm_t            fm_fpm;
    fm_imem_t           fm_imem;
    uint8_t             res1[8*1024];
    fm_soft_parser_t    fm_soft_parser;
    uint8_t             res2[96*1024];
    struct {
        fm_dtsec_t      fm_dtsec;
        fm_mdio_t       fm_mdio;
    } mac_1g[8];        /* support up to 8 1g controllers */
    struct {
        fm_10gec_t      fm_10gec;
        fm_10gec_mdio_t fm_10gec_mdio;
    } mac_10g[1];
    uint8_t             res4[48*1024];
    fm_1588_t           fm_1588;
    uint8_t             res5[4*1024];
} ccsr_fman_t;

struct fm_muram {
    uint32_t base;
    uint32_t top;
    uint32_t size;
    uint32_t alloc;
};
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


/* Rx/Tx buffer descriptor */
struct fm_port_bd {
    uint16_t status;
    uint16_t len;
    uint32_t res0;
    uint16_t res1;
    uint16_t buf_ptr_hi;
    uint32_t buf_ptr_lo;
};

#define FMBM_RCFG_EN            0x80000000 /* port is enabled to receive data */
#define FMBM_TCFG_EN            0x80000000 /* port is enabled to transmit data */


#define CONFIG_SYS_FM_MURAM_SIZE 0x28000

#define CONFIG_SYS_FSL_FM1_OFFSET 0x400000
#define CONFIG_SYS_IMMR 0xfe000000
#define CONFIG_SYS_FSL_FM1_ADDR (CONFIG_SYS_IMMR + CONFIG_SYS_FSL_FM1_OFFSET)
#define CONFIG_SYS_FM1_DTSEC1_ADDR (CONFIG_SYS_FSL_FM1_ADDR + 0xe0000)
#define CONFIG_SYS_FM1_EMI1_MDIO_ADDR (CONFIG_SYS_FSL_FM1_ADDR + 0xe1120)

#define CONFIG_SYS_FM1_DTSEC3_ADDR (CONFIG_SYS_FSL_FM1_ADDR + 0xe4000)


#define FM_MURAM_RES_SIZE 0x01000

struct fm_muram muram;

uint32_t fm_muram_base()
{
    return muram.base;
}

uint32_t fm_muram_alloc(int fm_idx, uint32_t size, uint32_t align)
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

char tx_buffer_pseudo_malloc[ sizeof(struct fm_port_bd) * TX_BD_RING_SIZE ];
char rx_ring_pseudo_malloc[ sizeof(struct fm_port_bd) * RX_BD_RING_SIZE ];
char rx_pool_pseudo_malloc[MAX_RXBUF_LEN * RX_BD_RING_SIZE];

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
    pram = (struct fm_port_global_pram *)fm_muram_alloc(fm_eth->fm_index,
            FM_PRAM_SIZE, FM_PRAM_ALIGN);
    if (!pram) {
        printf("%s: No muram for Tx global parameter\n", __func__);
        return 0;
    }
    fm_eth->tx_pram = pram;

    /* parameter page offset to MURAM */
    pram_page_offset = (uintptr_t)pram - fm_muram_base(fm_eth->fm_index);

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
    printf("STATUS_init=%x\n", ((struct fm_port_bd *)fm_eth->cur_txbd)->status);

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
    pram = (struct fm_port_global_pram *)fm_muram_alloc(fm_eth->fm_index,
            FM_PRAM_SIZE, FM_PRAM_ALIGN);
    fm_eth->rx_pram = pram;

    /* parameter page offset to MURAM */
    pram_page_offset = (u32)pram - fm_muram_base(fm_eth->fm_index);

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


#define RX_PORT_1G_BASE  0x08
#define TX_PORT_1G_BASE  0x28

struct fm_eth dtsec1;
struct fm_eth dtsec2;
struct fm_eth dtsec3;
struct fm_eth dtsec4;
struct fm_eth *current;

void p3041_init(void)
{
    ccsr_fman_t *reg = (void *)CONFIG_SYS_FSL_FM1_ADDR;

    printf("MACCFG1: %lx\n", in_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x100)));
    printf("MACCFG2: %lx\n", in_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x104)));

    printf("MAC1: %lx\n", in_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x140)));
    printf("MAC2: %lx\n", in_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x144)));
    //TCTRL 0x40
    uint32_t rctrl = in_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x50));
    //out_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x50), rctrl|8);
    out_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x50), 0x9);

    printf("RCTRL old %lx new %lx \n", rctrl,
            in_be32((void *)(CONFIG_SYS_FM1_DTSEC3_ADDR+0x50)));




    dtsec1.rx_port = (void *)&reg->port[RX_PORT_1G_BASE - 1].fm_bmi;
    dtsec1.tx_port = (void *)&reg->port[TX_PORT_1G_BASE - 1].fm_bmi;

    dtsec2.rx_port = (void *)&reg->port[RX_PORT_1G_BASE    ].fm_bmi;
    dtsec2.tx_port = (void *)&reg->port[TX_PORT_1G_BASE    ].fm_bmi;

    dtsec3.rx_port = (void *)&reg->port[RX_PORT_1G_BASE + 1].fm_bmi;
    dtsec3.tx_port = (void *)&reg->port[TX_PORT_1G_BASE + 1].fm_bmi;

    dtsec4.rx_port = (void *)&reg->port[RX_PORT_1G_BASE + 2].fm_bmi;
    dtsec4.tx_port = (void *)&reg->port[TX_PORT_1G_BASE + 2].fm_bmi;

    printf("dtsec3 bmi tx[tcfqid] %lx\n", in_be32(&dtsec3.tx_port->fmbm_tcfqid));
    printf("dtsec4 bmi tx[tcfqid] %lx\n", in_be32(&dtsec4.tx_port->fmbm_tcfqid));
    printf("dtsec3 rx %lx\n", in_be32(&dtsec3.rx_port->fmbm_rfqid));
    printf("dtsec4 rx %lx\n", in_be32(&dtsec4.rx_port->fmbm_rfqid));

    dtsec1.fm_index = 0;
    dtsec2.fm_index = 0;
    dtsec3.fm_index = 0;
    dtsec4.fm_index = 0;
    current = &dtsec3;

    fm_init_muram(0, &reg->muram);
    //fm_init_qmi(&reg->fm_qmi_common);
    //fm_init_bmi(0, &reg->fm_bmi_common);

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



    //fm_muram_alloc(0, 0x20100, 256);
    muram.alloc = muram.base + 0x21000;
    fm_eth_rx_port_parameter_init(current);
    fm_eth_tx_port_parameter_init(current);


    printf("dtsec ADDR = %p\n", &(reg->mac_1g[2].fm_dtsec));
    printf("dtsec ADDR = %p\n", (void *)CONFIG_SYS_FM1_DTSEC3_ADDR);
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
uint8_t macaddr[] = { 0x2, 0x0, 0xa9, 0xa1, 0x56, 0x57};

pok_network_driver_device_t pok_network_p3041_device = {
    .ops = &driver_ops,
    .mac = macaddr
};

