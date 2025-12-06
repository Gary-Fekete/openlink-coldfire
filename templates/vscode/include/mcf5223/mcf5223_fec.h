/*
 * File:    mcf5223_fec.h
 * Purpose: Register and bit definitions for Fast Ethernet Controller
 *
 * Based on MCF52235 Reference Manual, Chapter 18 - Fast Ethernet Controller (FEC)
 * Base Address: IPSBAR + 0x001000
 */

#ifndef __MCF5223_FEC_H__
#define __MCF5223_FEC_H__

#include <stdint.h>

/*********************************************************************
*
* Fast Ethernet Controller (FEC)
*
*********************************************************************/

/* Base address */
#define MCF_FEC_BASE            (IPSBAR + 0x001000)

/* Control/Status Registers */
#define MCF_FEC_EIR             (*(volatile uint32_t *)(MCF_FEC_BASE + 0x004))
#define MCF_FEC_EIMR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x008))
#define MCF_FEC_RDAR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x010))
#define MCF_FEC_TDAR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x014))
#define MCF_FEC_ECR             (*(volatile uint32_t *)(MCF_FEC_BASE + 0x024))
#define MCF_FEC_MMFR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x040))
#define MCF_FEC_MSCR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x044))
#define MCF_FEC_MIBC            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x064))
#define MCF_FEC_RCR             (*(volatile uint32_t *)(MCF_FEC_BASE + 0x084))
#define MCF_FEC_TCR             (*(volatile uint32_t *)(MCF_FEC_BASE + 0x0C4))
#define MCF_FEC_PALR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x0E4))
#define MCF_FEC_PAUR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x0E8))
#define MCF_FEC_OPD             (*(volatile uint32_t *)(MCF_FEC_BASE + 0x0EC))
#define MCF_FEC_IAUR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x118))
#define MCF_FEC_IALR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x11C))
#define MCF_FEC_GAUR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x120))
#define MCF_FEC_GALR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x124))
#define MCF_FEC_TFWR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x144))
#define MCF_FEC_FRBR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x14C))
#define MCF_FEC_FRSR            (*(volatile uint32_t *)(MCF_FEC_BASE + 0x150))
#define MCF_FEC_ERDSR           (*(volatile uint32_t *)(MCF_FEC_BASE + 0x180))
#define MCF_FEC_ETDSR           (*(volatile uint32_t *)(MCF_FEC_BASE + 0x184))
#define MCF_FEC_EMRBR           (*(volatile uint32_t *)(MCF_FEC_BASE + 0x188))

/* MIB Block Counters (RMON) - Transmit */
#define MCF_FEC_RMON_T_DROP         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x200))
#define MCF_FEC_RMON_T_PACKETS      (*(volatile uint32_t *)(MCF_FEC_BASE + 0x204))
#define MCF_FEC_RMON_T_BC_PKT       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x208))
#define MCF_FEC_RMON_T_MC_PKT       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x20C))
#define MCF_FEC_RMON_T_CRC_ALIGN    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x210))
#define MCF_FEC_RMON_T_UNDERSIZE    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x214))
#define MCF_FEC_RMON_T_OVERSIZE     (*(volatile uint32_t *)(MCF_FEC_BASE + 0x218))
#define MCF_FEC_RMON_T_FRAG         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x21C))
#define MCF_FEC_RMON_T_JAB          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x220))
#define MCF_FEC_RMON_T_COL          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x224))
#define MCF_FEC_RMON_T_P64          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x228))
#define MCF_FEC_RMON_T_P65TO127     (*(volatile uint32_t *)(MCF_FEC_BASE + 0x22C))
#define MCF_FEC_RMON_T_P128TO255    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x230))
#define MCF_FEC_RMON_T_P256TO511    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x234))
#define MCF_FEC_RMON_T_P512TO1023   (*(volatile uint32_t *)(MCF_FEC_BASE + 0x238))
#define MCF_FEC_RMON_T_P1024TO2047  (*(volatile uint32_t *)(MCF_FEC_BASE + 0x23C))
#define MCF_FEC_RMON_T_P_GTE2048    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x240))
#define MCF_FEC_RMON_T_OCTETS       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x244))

/* IEEE TX Counters */
#define MCF_FEC_IEEE_T_DROP         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x248))
#define MCF_FEC_IEEE_T_FRAME_OK     (*(volatile uint32_t *)(MCF_FEC_BASE + 0x24C))
#define MCF_FEC_IEEE_T_1COL         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x250))
#define MCF_FEC_IEEE_T_MCOL         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x254))
#define MCF_FEC_IEEE_T_DEF          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x258))
#define MCF_FEC_IEEE_T_LCOL         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x25C))
#define MCF_FEC_IEEE_T_EXCOL        (*(volatile uint32_t *)(MCF_FEC_BASE + 0x260))
#define MCF_FEC_IEEE_T_MACERR       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x264))
#define MCF_FEC_IEEE_T_CSERR        (*(volatile uint32_t *)(MCF_FEC_BASE + 0x268))
#define MCF_FEC_IEEE_T_SQE          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x26C))
#define MCF_FEC_IEEE_T_FDXFC        (*(volatile uint32_t *)(MCF_FEC_BASE + 0x270))
#define MCF_FEC_IEEE_T_OCTETS_OK    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x274))

/* MIB Block Counters (RMON) - Receive */
#define MCF_FEC_RMON_R_DROP         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x280))
#define MCF_FEC_RMON_R_PACKETS      (*(volatile uint32_t *)(MCF_FEC_BASE + 0x284))
#define MCF_FEC_RMON_R_BC_PKT       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x288))
#define MCF_FEC_RMON_R_MC_PKT       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x28C))
#define MCF_FEC_RMON_R_CRC_ALIGN    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x290))
#define MCF_FEC_RMON_R_UNDERSIZE    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x294))
#define MCF_FEC_RMON_R_OVERSIZE     (*(volatile uint32_t *)(MCF_FEC_BASE + 0x298))
#define MCF_FEC_RMON_R_FRAG         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x29C))
#define MCF_FEC_RMON_R_JAB          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2A0))
#define MCF_FEC_RMON_R_P64          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2A8))
#define MCF_FEC_RMON_R_P65TO127     (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2AC))
#define MCF_FEC_RMON_R_P128TO255    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2B0))
#define MCF_FEC_RMON_R_P256TO511    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2B4))
#define MCF_FEC_RMON_R_512TO1023    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2B8))
#define MCF_FEC_RMON_R_1024TO2047   (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2BC))
#define MCF_FEC_RMON_R_P_GTE2048    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2C0))
#define MCF_FEC_RMON_R_OCTETS       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2C4))

/* IEEE RX Counters */
#define MCF_FEC_IEEE_R_DROP         (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2C8))
#define MCF_FEC_IEEE_R_FRAME_OK     (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2CC))
#define MCF_FEC_IEEE_R_CRC          (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2D0))
#define MCF_FEC_IEEE_R_ALIGN        (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2D4))
#define MCF_FEC_IEEE_R_MACERR       (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2D8))
#define MCF_FEC_IEEE_R_FDXFC        (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2DC))
#define MCF_FEC_IEEE_R_OCTETS_OK    (*(volatile uint32_t *)(MCF_FEC_BASE + 0x2E0))

/* Bit definitions and macros for MCF_FEC_EIR */
#define MCF_FEC_EIR_UN                (0x00080000)
#define MCF_FEC_EIR_RL                (0x00100000)
#define MCF_FEC_EIR_LC                (0x00200000)
#define MCF_FEC_EIR_EBERR             (0x00400000)
#define MCF_FEC_EIR_MII               (0x00800000)
#define MCF_FEC_EIR_RXB               (0x01000000)
#define MCF_FEC_EIR_RXF               (0x02000000)
#define MCF_FEC_EIR_TXB               (0x04000000)
#define MCF_FEC_EIR_TXF               (0x08000000)
#define MCF_FEC_EIR_GRA               (0x10000000)
#define MCF_FEC_EIR_BABT              (0x20000000)
#define MCF_FEC_EIR_BABR              (0x40000000)
#define MCF_FEC_EIR_HBERR             (0x80000000)
#define MCF_FEC_EIR_CLEAR_ALL         (0xFFFFFFFF)

/* Bit definitions and macros for MCF_FEC_EIMR */
#define MCF_FEC_EIMR_UN               (0x00080000)
#define MCF_FEC_EIMR_RL               (0x00100000)
#define MCF_FEC_EIMR_LC               (0x00200000)
#define MCF_FEC_EIMR_EBERR            (0x00400000)
#define MCF_FEC_EIMR_MII              (0x00800000)
#define MCF_FEC_EIMR_RXB              (0x01000000)
#define MCF_FEC_EIMR_RXF              (0x02000000)
#define MCF_FEC_EIMR_TXB              (0x04000000)
#define MCF_FEC_EIMR_TXF              (0x08000000)
#define MCF_FEC_EIMR_GRA              (0x10000000)
#define MCF_FEC_EIMR_BABT             (0x20000000)
#define MCF_FEC_EIMR_BABR             (0x40000000)
#define MCF_FEC_EIMR_HBERR            (0x80000000)
#define MCF_FEC_EIMR_MASK_ALL         (0x00000000)
#define MCF_FEC_EIMR_UNMASK_ALL       (0xFFFFFFFF)

/* Bit definitions and macros for MCF_FEC_RDAR */
#define MCF_FEC_RDAR_R_DES_ACTIVE     (0x01000000)

/* Bit definitions and macros for MCF_FEC_TDAR */
#define MCF_FEC_TDAR_X_DES_ACTIVE     (0x01000000)

/* Bit definitions and macros for MCF_FEC_ECR */
#define MCF_FEC_ECR_RESET             (0x00000001)
#define MCF_FEC_ECR_ETHER_EN          (0x00000002)

/* Bit definitions and macros for MCF_FEC_MMFR */
#define MCF_FEC_MMFR_DATA(x)          (((x)&0x0000FFFF)<<0)
#define MCF_FEC_MMFR_TA(x)            (((x)&0x00000003)<<16)
#define MCF_FEC_MMFR_RA(x)            (((x)&0x0000001F)<<18)
#define MCF_FEC_MMFR_PA(x)            (((x)&0x0000001F)<<23)
#define MCF_FEC_MMFR_OP(x)            (((x)&0x00000003)<<28)
#define MCF_FEC_MMFR_ST(x)            (((x)&0x00000003)<<30)
#define MCF_FEC_MMFR_ST_01            (0x40000000)
#define MCF_FEC_MMFR_OP_READ          (0x20000000)
#define MCF_FEC_MMFR_OP_WRITE         (0x10000000)
#define MCF_FEC_MMFR_TA_10            (0x00020000)

/* Bit definitions and macros for MCF_FEC_MSCR */
#define MCF_FEC_MSCR_MII_SPEED(x)     (((x)&0x0000003F)<<1)
#define MCF_FEC_MSCR_DIS_PREAMBLE     (0x00000080)

/* Bit definitions and macros for MCF_FEC_MIBC */
#define MCF_FEC_MIBC_MIB_IDLE         (0x40000000)
#define MCF_FEC_MIBC_MIB_DISABLE      (0x80000000)

/* Bit definitions and macros for MCF_FEC_RCR */
#define MCF_FEC_RCR_LOOP              (0x00000001)
#define MCF_FEC_RCR_DRT               (0x00000002)
#define MCF_FEC_RCR_MII_MODE          (0x00000004)
#define MCF_FEC_RCR_PROM              (0x00000008)
#define MCF_FEC_RCR_BC_REJ            (0x00000010)
#define MCF_FEC_RCR_FCE               (0x00000020)
#define MCF_FEC_RCR_MAX_FL(x)         (((x)&0x000007FF)<<16)

/* Bit definitions and macros for MCF_FEC_TCR */
#define MCF_FEC_TCR_GTS               (0x00000001)
#define MCF_FEC_TCR_HBC               (0x00000002)
#define MCF_FEC_TCR_FDEN              (0x00000004)
#define MCF_FEC_TCR_TFC_PAUSE         (0x00000008)
#define MCF_FEC_TCR_RFC_PAUSE         (0x00000010)

/* Bit definitions and macros for MCF_FEC_PALR */
#define MCF_FEC_PALR_PADDR1(x)        (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_FEC_PAUR */
#define MCF_FEC_PAUR_TYPE(x)          (((x)&0x0000FFFF)<<0)
#define MCF_FEC_PAUR_PADDR2(x)        (((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for MCF_FEC_OPD */
#define MCF_FEC_OPD_PAUSE_DUR(x)      (((x)&0x0000FFFF)<<0)
#define MCF_FEC_OPD_OPCODE(x)         (((x)&0x0000FFFF)<<16)

/* Bit definitions and macros for MCF_FEC_IAUR */
#define MCF_FEC_IAUR_IADDR1(x)        (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_FEC_IALR */
#define MCF_FEC_IALR_IADDR2(x)        (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_FEC_GAUR */
#define MCF_FEC_GAUR_GADDR1(x)        (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_FEC_GALR */
#define MCF_FEC_GALR_GADDR2(x)        (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_FEC_TFWR */
#define MCF_FEC_TFWR_X_WMRK(x)        (((x)&0x00000003)<<0)

/* Bit definitions and macros for MCF_FEC_FRBR */
#define MCF_FEC_FRBR_R_BOUND(x)       (((x)&0x000000FF)<<2)

/* Bit definitions and macros for MCF_FEC_FRSR */
#define MCF_FEC_FRSR_R_FSTART(x)      (((x)&0x000000FF)<<2)

/* Bit definitions and macros for MCF_FEC_ERDSR */
#define MCF_FEC_ERDSR_R_DES_START(x)  (((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for MCF_FEC_ETDSR */
#define MCF_FEC_ETDSR_X_DES_START(x)  (((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for MCF_FEC_EMRBR */
#define MCF_FEC_EMRBR_R_BUF_SIZE(x)   (((x)&0x0000007F)<<4)

/********************************************************************/

#endif /* __MCF5223_FEC_H__ */
