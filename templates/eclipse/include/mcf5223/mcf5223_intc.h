/*
 * File:    mcf5223_intc.h
 * Purpose: Interrupt Controller (INTC) register definitions for MCF5223
 * Based on MCF52235 Reference Manual, Chapter 15
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_INTC_H
#define MCF5223_INTC_H

#include <stdint.h>

/*********************************************************************
 * Interrupt Controller (INTC)
 * Base Addresses:
 *   INTC0: IPSBAR + 0x000C00
 *   INTC1: IPSBAR + 0x000D00
 *********************************************************************/

#define MCF_INTC0_BASE          (IPSBAR + 0x000C00)
#define MCF_INTC1_BASE          (IPSBAR + 0x000D00)

/*********************************************************************
 * INTC0 Register definitions
 *********************************************************************/

#define MCF_INTC0_IPRH          (*(volatile uint32_t *)(MCF_INTC0_BASE + 0x00))
#define MCF_INTC0_IPRL          (*(volatile uint32_t *)(MCF_INTC0_BASE + 0x04))
#define MCF_INTC0_IMRH          (*(volatile uint32_t *)(MCF_INTC0_BASE + 0x08))
#define MCF_INTC0_IMRL          (*(volatile uint32_t *)(MCF_INTC0_BASE + 0x0C))
#define MCF_INTC0_INTFRCH       (*(volatile uint32_t *)(MCF_INTC0_BASE + 0x10))
#define MCF_INTC0_INTFRCL       (*(volatile uint32_t *)(MCF_INTC0_BASE + 0x14))
#define MCF_INTC0_IRLR          (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0x18))
#define MCF_INTC0_IACKLPR       (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0x19))
#define MCF_INTC0_SWIACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xE0))
#define MCF_INTC0_L1IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xE4))
#define MCF_INTC0_L2IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xE8))
#define MCF_INTC0_L3IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xEC))
#define MCF_INTC0_L4IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xF0))
#define MCF_INTC0_L5IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xF4))
#define MCF_INTC0_L6IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xF8))
#define MCF_INTC0_L7IACK        (*(volatile uint8_t  *)(MCF_INTC0_BASE + 0xFC))

/* INTC0 Interrupt Control Registers (ICR1-ICR62) */
#define MCF_INTC0_ICR(n)        (*(volatile uint8_t *)(MCF_INTC0_BASE + 0x41 + ((n) - 1)))

/*********************************************************************
 * INTC1 Register definitions
 *********************************************************************/

#define MCF_INTC1_IPRH          (*(volatile uint32_t *)(MCF_INTC1_BASE + 0x00))
#define MCF_INTC1_IPRL          (*(volatile uint32_t *)(MCF_INTC1_BASE + 0x04))
#define MCF_INTC1_IMRH          (*(volatile uint32_t *)(MCF_INTC1_BASE + 0x08))
#define MCF_INTC1_IMRL          (*(volatile uint32_t *)(MCF_INTC1_BASE + 0x0C))
#define MCF_INTC1_INTFRCH       (*(volatile uint32_t *)(MCF_INTC1_BASE + 0x10))
#define MCF_INTC1_INTFRCL       (*(volatile uint32_t *)(MCF_INTC1_BASE + 0x14))
#define MCF_INTC1_IRLR          (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0x18))
#define MCF_INTC1_IACKLPR       (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0x19))
#define MCF_INTC1_SWIACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xE0))
#define MCF_INTC1_L1IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xE4))
#define MCF_INTC1_L2IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xE8))
#define MCF_INTC1_L3IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xEC))
#define MCF_INTC1_L4IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xF0))
#define MCF_INTC1_L5IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xF4))
#define MCF_INTC1_L6IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xF8))
#define MCF_INTC1_L7IACK        (*(volatile uint8_t  *)(MCF_INTC1_BASE + 0xFC))

/* INTC1 Interrupt Control Registers (ICR1-ICR63) */
#define MCF_INTC1_ICR(n)        (*(volatile uint8_t *)(MCF_INTC1_BASE + 0x41 + ((n) - 1)))

/*********************************************************************
 * Generic INTC macros (x = 0 or 1)
 *********************************************************************/

#define MCF_INTC_BASE(x)        (IPSBAR + 0x000C00 + ((x) * 0x100))
#define MCF_INTC_IPRH(x)        (*(volatile uint32_t *)(MCF_INTC_BASE(x) + 0x00))
#define MCF_INTC_IPRL(x)        (*(volatile uint32_t *)(MCF_INTC_BASE(x) + 0x04))
#define MCF_INTC_IMRH(x)        (*(volatile uint32_t *)(MCF_INTC_BASE(x) + 0x08))
#define MCF_INTC_IMRL(x)        (*(volatile uint32_t *)(MCF_INTC_BASE(x) + 0x0C))
#define MCF_INTC_INTFRCH(x)     (*(volatile uint32_t *)(MCF_INTC_BASE(x) + 0x10))
#define MCF_INTC_INTFRCL(x)     (*(volatile uint32_t *)(MCF_INTC_BASE(x) + 0x14))
#define MCF_INTC_IRLR(x)        (*(volatile uint8_t  *)(MCF_INTC_BASE(x) + 0x18))
#define MCF_INTC_IACKLPR(x)     (*(volatile uint8_t  *)(MCF_INTC_BASE(x) + 0x19))
#define MCF_INTC_ICR(x, n)      (*(volatile uint8_t  *)(MCF_INTC_BASE(x) + 0x41 + ((n) - 1)))

/*********************************************************************
 * IPRH/IPRL bit definitions - Interrupt Pending Register
 *********************************************************************/

#define MCF_INTC_IPRL_INT1      (0x00000002)
#define MCF_INTC_IPRL_INT2      (0x00000004)
#define MCF_INTC_IPRL_INT3      (0x00000008)
#define MCF_INTC_IPRL_INT(n)    (1 << (n))      /* n = 1-31 */

#define MCF_INTC_IPRH_INT32     (0x00000001)
#define MCF_INTC_IPRH_INT(n)    (1 << ((n) - 32))   /* n = 32-63 */

/*********************************************************************
 * IMRH/IMRL bit definitions - Interrupt Mask Register
 *********************************************************************/

#define MCF_INTC_IMRL_MASKALL   (0x00000001)    /* Mask all interrupts */
#define MCF_INTC_IMRL_MASK1     (0x00000002)
#define MCF_INTC_IMRL_MASK(n)   (1 << (n))      /* n = 1-31 */

#define MCF_INTC_IMRH_MASK32    (0x00000001)
#define MCF_INTC_IMRH_MASK(n)   (1 << ((n) - 32))   /* n = 32-63 */

/*********************************************************************
 * INTFRCH/INTFRCL bit definitions - Interrupt Force Register
 *********************************************************************/

#define MCF_INTC_INTFRCL_INTFRC(n)  (1 << (n))      /* n = 1-31 */
#define MCF_INTC_INTFRCH_INTFRC(n)  (1 << ((n) - 32))   /* n = 32-63 */

/*********************************************************************
 * ICR bit definitions - Interrupt Control Register
 *********************************************************************/

#define MCF_INTC_ICR_IP(x)      (((x) & 0x07) << 0) /* Interrupt priority */
#define MCF_INTC_ICR_IL(x)      (((x) & 0x07) << 3) /* Interrupt level */

/*********************************************************************
 * IRLR bit definitions - Interrupt Request Level Register
 *********************************************************************/

#define MCF_INTC_IRLR_IRQ(x)    (((x) & 0x7F) << 1)

/*********************************************************************
 * IACKLPR bit definitions - Interrupt Acknowledge Level/Priority
 *********************************************************************/

#define MCF_INTC_IACKLPR_PRI(x)     (((x) & 0x0F) << 0) /* Priority */
#define MCF_INTC_IACKLPR_LEVEL(x)   (((x) & 0x07) << 4) /* Level */

/*********************************************************************
 * Interrupt source definitions for INTC0
 * Vector = 64 + source number
 *********************************************************************/

/* Edge Port Interrupts */
#define MCF_INTC_SRC_EPORT_EPF1     1
#define MCF_INTC_SRC_EPORT_EPF2     2
#define MCF_INTC_SRC_EPORT_EPF3     3
#define MCF_INTC_SRC_EPORT_EPF4     4
#define MCF_INTC_SRC_EPORT_EPF5     5
#define MCF_INTC_SRC_EPORT_EPF6     6
#define MCF_INTC_SRC_EPORT_EPF7     7

/* DMA Interrupts */
#define MCF_INTC_SRC_DMA0           8
#define MCF_INTC_SRC_DMA1           9
#define MCF_INTC_SRC_DMA2           10
#define MCF_INTC_SRC_DMA3           11

/* UART Interrupts */
#define MCF_INTC_SRC_UART0          13
#define MCF_INTC_SRC_UART1          14
#define MCF_INTC_SRC_UART2          15

/* I2C Interrupt */
#define MCF_INTC_SRC_I2C            17

/* QSPI Interrupt */
#define MCF_INTC_SRC_QSPI           18

/* DMA Timer Interrupts */
#define MCF_INTC_SRC_DTIM0          19
#define MCF_INTC_SRC_DTIM1          20
#define MCF_INTC_SRC_DTIM2          21
#define MCF_INTC_SRC_DTIM3          22

/* FEC Interrupts */
#define MCF_INTC_SRC_FEC_TXF        23
#define MCF_INTC_SRC_FEC_TXB        24
#define MCF_INTC_SRC_FEC_UN         25
#define MCF_INTC_SRC_FEC_RL         26
#define MCF_INTC_SRC_FEC_RXF        27
#define MCF_INTC_SRC_FEC_RXB        28
#define MCF_INTC_SRC_FEC_MII        29
#define MCF_INTC_SRC_FEC_LC         30
#define MCF_INTC_SRC_FEC_HBERR      31
#define MCF_INTC_SRC_FEC_GRA        32
#define MCF_INTC_SRC_FEC_EBERR      33
#define MCF_INTC_SRC_FEC_BABT       34
#define MCF_INTC_SRC_FEC_BABR       35

/* GPT Interrupts */
#define MCF_INTC_SRC_GPT_TOF        43
#define MCF_INTC_SRC_GPT_PAIF       44
#define MCF_INTC_SRC_GPT_PAOVF      45
#define MCF_INTC_SRC_GPT_C0F        46
#define MCF_INTC_SRC_GPT_C1F        47
#define MCF_INTC_SRC_GPT_C2F        48
#define MCF_INTC_SRC_GPT_C3F        49

/* PWM Interrupt */
#define MCF_INTC_SRC_PWM            50

/* CAN Interrupts */
#define MCF_INTC_SRC_CAN_BOFF       51
#define MCF_INTC_SRC_CAN_ERR        52
#define MCF_INTC_SRC_CAN_WAKE       53
#define MCF_INTC_SRC_CAN_BUF0       54
#define MCF_INTC_SRC_CAN_BUF1       55
#define MCF_INTC_SRC_CAN_BUF2       56
#define MCF_INTC_SRC_CAN_BUF3       57
#define MCF_INTC_SRC_CAN_BUF4       58
#define MCF_INTC_SRC_CAN_BUF5       59
#define MCF_INTC_SRC_CAN_BUF6       60
#define MCF_INTC_SRC_CAN_BUF7       61
#define MCF_INTC_SRC_CAN_BUF8       62
#define MCF_INTC_SRC_CAN_BUF9       63

/*********************************************************************
 * Interrupt source definitions for INTC1
 *********************************************************************/

/* CAN Buffer Interrupts (continued) */
#define MCF_INTC1_SRC_CAN_BUF10     1
#define MCF_INTC1_SRC_CAN_BUF11     2
#define MCF_INTC1_SRC_CAN_BUF12     3
#define MCF_INTC1_SRC_CAN_BUF13     4
#define MCF_INTC1_SRC_CAN_BUF14     5
#define MCF_INTC1_SRC_CAN_BUF15     6

/* PIT Interrupts */
#define MCF_INTC1_SRC_PIT0          7
#define MCF_INTC1_SRC_PIT1          8

/* CFM Interrupt */
#define MCF_INTC1_SRC_CFM           11

/* RTC Interrupt */
#define MCF_INTC1_SRC_RTC           12

/* ADC Interrupts */
#define MCF_INTC1_SRC_ADCA          13
#define MCF_INTC1_SRC_ADCB          14

#endif /* MCF5223_INTC_H */
