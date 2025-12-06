/*
 * File:    mcf5223_can.h
 * Purpose: Register and bit definitions for FlexCAN Module
 *
 * Based on MCF52235 Reference Manual, Chapter 30 - FlexCAN
 * Base Address: IPSBAR + 0x1C0000
 */

#ifndef __MCF5223_CAN_H__
#define __MCF5223_CAN_H__

#include <stdint.h>

/*********************************************************************
*
* FlexCAN Module (CAN)
*
*********************************************************************/

/* Base address */
#define MCF_CAN_BASE            (IPSBAR + 0x1C0000)

/* Register definitions */
#define MCF_CAN_CANMCR          (*(volatile uint32_t *)(MCF_CAN_BASE + 0x00))
#define MCF_CAN_CANCTRL         (*(volatile uint32_t *)(MCF_CAN_BASE + 0x04))
#define MCF_CAN_TIMER           (*(volatile uint32_t *)(MCF_CAN_BASE + 0x08))
#define MCF_CAN_RXGMASK         (*(volatile uint32_t *)(MCF_CAN_BASE + 0x10))
#define MCF_CAN_RX14MASK        (*(volatile uint32_t *)(MCF_CAN_BASE + 0x14))
#define MCF_CAN_RX15MASK        (*(volatile uint32_t *)(MCF_CAN_BASE + 0x18))
#define MCF_CAN_ERRCNT          (*(volatile uint32_t *)(MCF_CAN_BASE + 0x1C))
#define MCF_CAN_ERRSTAT         (*(volatile uint32_t *)(MCF_CAN_BASE + 0x20))
#define MCF_CAN_IMASK           (*(volatile uint16_t *)(MCF_CAN_BASE + 0x2A))
#define MCF_CAN_IFLAG           (*(volatile uint16_t *)(MCF_CAN_BASE + 0x32))

/* Message Buffer base (16 message buffers, 16 bytes each) */
#define MCF_CAN_MB_BASE         (MCF_CAN_BASE + 0x80)
#define MCF_CAN_MB(n)           ((volatile uint32_t *)(MCF_CAN_MB_BASE + ((n) * 0x10)))

/* Bit definitions and macros for MCF_CAN_CANMCR */
#define MCF_CAN_CANMCR_MAXMB(x)          (((x)&0x0000000F)<<0)
#define MCF_CAN_CANMCR_SUPV              (0x00800000)
#define MCF_CAN_CANMCR_FRZACK            (0x01000000)
#define MCF_CAN_CANMCR_SOFTRST           (0x02000000)
#define MCF_CAN_CANMCR_NOTRDY            (0x08000000)
#define MCF_CAN_CANMCR_HALT              (0x10000000)
#define MCF_CAN_CANMCR_FRZ               (0x40000000)
#define MCF_CAN_CANMCR_MDIS              (0x80000000)

/* Bit definitions and macros for MCF_CAN_CANCTRL */
#define MCF_CAN_CANCTRL_PROPSEG(x)       (((x)&0x00000007)<<0)
#define MCF_CAN_CANCTRL_LOM              (0x00000008)
#define MCF_CAN_CANCTRL_LBUF             (0x00000010)
#define MCF_CAN_CANCTRL_TSYNC            (0x00000020)
#define MCF_CAN_CANCTRL_BOFFREC          (0x00000040)
#define MCF_CAN_CANCTRL_SAMP             (0x00000080)
#define MCF_CAN_CANCTRL_LPB              (0x00001000)
#define MCF_CAN_CANCTRL_CLKSRC           (0x00002000)
#define MCF_CAN_CANCTRL_ERRMSK           (0x00004000)
#define MCF_CAN_CANCTRL_BOFFMSK          (0x00008000)
#define MCF_CAN_CANCTRL_PSEG2(x)         (((x)&0x00000007)<<16)
#define MCF_CAN_CANCTRL_PSEG1(x)         (((x)&0x00000007)<<19)
#define MCF_CAN_CANCTRL_RJW(x)           (((x)&0x00000003)<<22)
#define MCF_CAN_CANCTRL_PRESDIV(x)       (((x)&0x000000FF)<<24)

/* Bit definitions and macros for MCF_CAN_TIMER */
#define MCF_CAN_TIMER_TIMER(x)           (((x)&0x0000FFFF)<<0)

/* Bit definitions and macros for MCF_CAN_RXGMASK */
#define MCF_CAN_RXGMASK_MI(x)            (((x)&0x1FFFFFFF)<<0)

/* Bit definitions and macros for MCF_CAN_RX14MASK */
#define MCF_CAN_RX14MASK_MI(x)           (((x)&0x1FFFFFFF)<<0)

/* Bit definitions and macros for MCF_CAN_RX15MASK */
#define MCF_CAN_RX15MASK_MI(x)           (((x)&0x1FFFFFFF)<<0)

/* Bit definitions and macros for MCF_CAN_ERRCNT */
#define MCF_CAN_ERRCNT_TXECTR(x)         (((x)&0x000000FF)<<0)
#define MCF_CAN_ERRCNT_RXECTR(x)         (((x)&0x000000FF)<<8)

/* Bit definitions and macros for MCF_CAN_ERRSTAT */
#define MCF_CAN_ERRSTAT_WAKINT           (0x00000001)
#define MCF_CAN_ERRSTAT_ERRINT           (0x00000002)
#define MCF_CAN_ERRSTAT_BOFFINT          (0x00000004)
#define MCF_CAN_ERRSTAT_FLTCONF(x)       (((x)&0x00000003)<<4)
#define MCF_CAN_ERRSTAT_TXRX             (0x00000040)
#define MCF_CAN_ERRSTAT_IDLE             (0x00000080)
#define MCF_CAN_ERRSTAT_RXWRN            (0x00000100)
#define MCF_CAN_ERRSTAT_TXWRN            (0x00000200)
#define MCF_CAN_ERRSTAT_STFERR           (0x00000400)
#define MCF_CAN_ERRSTAT_FRMERR           (0x00000800)
#define MCF_CAN_ERRSTAT_CRCERR           (0x00001000)
#define MCF_CAN_ERRSTAT_ACKERR           (0x00002000)
#define MCF_CAN_ERRSTAT_BITERR(x)        (((x)&0x00000003)<<14)
#define MCF_CAN_ERRSTAT_FLTCONF_ACTIVE   (0x00000000)
#define MCF_CAN_ERRSTAT_FLTCONF_PASSIVE  (0x00000010)
#define MCF_CAN_ERRSTAT_FLTCONF_BUSOFF   (0x00000020)

/* Bit definitions and macros for MCF_CAN_IMASK */
#define MCF_CAN_IMASK_BUF0M              (0x0001)
#define MCF_CAN_IMASK_BUF1M              (0x0002)
#define MCF_CAN_IMASK_BUF2M              (0x0004)
#define MCF_CAN_IMASK_BUF3M              (0x0008)
#define MCF_CAN_IMASK_BUF4M              (0x0010)
#define MCF_CAN_IMASK_BUF5M              (0x0020)
#define MCF_CAN_IMASK_BUF6M              (0x0040)
#define MCF_CAN_IMASK_BUF7M              (0x0080)
#define MCF_CAN_IMASK_BUF8M              (0x0100)
#define MCF_CAN_IMASK_BUF9M              (0x0200)
#define MCF_CAN_IMASK_BUF10M             (0x0400)
#define MCF_CAN_IMASK_BUF11M             (0x0800)
#define MCF_CAN_IMASK_BUF12M             (0x1000)
#define MCF_CAN_IMASK_BUF13M             (0x2000)
#define MCF_CAN_IMASK_BUF14M             (0x4000)
#define MCF_CAN_IMASK_BUF15M             (0x8000)

/* Bit definitions and macros for MCF_CAN_IFLAG */
#define MCF_CAN_IFLAG_BUF0I              (0x0001)
#define MCF_CAN_IFLAG_BUF1I              (0x0002)
#define MCF_CAN_IFLAG_BUF2I              (0x0004)
#define MCF_CAN_IFLAG_BUF3I              (0x0008)
#define MCF_CAN_IFLAG_BUF4I              (0x0010)
#define MCF_CAN_IFLAG_BUF5I              (0x0020)
#define MCF_CAN_IFLAG_BUF6I              (0x0040)
#define MCF_CAN_IFLAG_BUF7I              (0x0080)
#define MCF_CAN_IFLAG_BUF8I              (0x0100)
#define MCF_CAN_IFLAG_BUF9I              (0x0200)
#define MCF_CAN_IFLAG_BUF10I             (0x0400)
#define MCF_CAN_IFLAG_BUF11I             (0x0800)
#define MCF_CAN_IFLAG_BUF12I             (0x1000)
#define MCF_CAN_IFLAG_BUF13I             (0x2000)
#define MCF_CAN_IFLAG_BUF14I             (0x4000)
#define MCF_CAN_IFLAG_BUF15I             (0x8000)

/********************************************************************/

#endif /* __MCF5223_CAN_H__ */
