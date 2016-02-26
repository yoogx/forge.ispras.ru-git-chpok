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

/* Fman ethernet private struct */
struct fm_eth {
    //int fm_index;                       /* Fman index */
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

struct fm_muram {
    uint32_t base;
    uint32_t top;
    uint32_t size;
    uint32_t alloc;
};

/* Rx/Tx buffer descriptor */
struct fm_port_bd {
    uint16_t status;
    uint16_t len;
    uint32_t res0;
    uint16_t res1;
    uint16_t buf_ptr_hi;
    uint32_t buf_ptr_lo;
};

