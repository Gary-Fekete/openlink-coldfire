/*
 * File:    mcf5223_scm.h
 * Purpose: System Control Module (SCM) register definitions for MCF5223
 * Based on MCF52235 Reference Manual, Chapter 13
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_SCM_H
#define MCF5223_SCM_H

#include <stdint.h>

/*********************************************************************
 * System Control Module (SCM)
 * Base Address: IPSBAR + 0x000000
 *********************************************************************/

#define MCF_SCM_BASE            (IPSBAR + 0x000000)

/* Register definitions */
#define MCF_SCM_IPSBAR          (*(volatile uint32_t *)(MCF_SCM_BASE + 0x00))
#define MCF_SCM_RAMBAR          (*(volatile uint32_t *)(MCF_SCM_BASE + 0x08))
#define MCF_SCM_CRSR            (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x10))
#define MCF_SCM_CWCR            (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x11))
#define MCF_SCM_LPICR           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x12))
#define MCF_SCM_CWSR            (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x13))
#define MCF_SCM_PPMRH			  (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x0C))
#define MCF_SCM_PPMRL			  (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x18))
#define MCF_SCM_DMAREQC         (*(volatile uint32_t *)(MCF_SCM_BASE + 0x14))
#define MCF_SCM_MPARK           (*(volatile uint32_t *)(MCF_SCM_BASE + 0x1C))
#define MCF_SCM_MPR             (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x20))
#define MCF_SCM_PPMRS           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x21))
#define MCF_SCM_PPMRC           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x22))
#define MCF_SCM_PACR0           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x24))
#define MCF_SCM_PACR1           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x25))
#define MCF_SCM_PACR2           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x26))
#define MCF_SCM_PACR3           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x27))
#define MCF_SCM_PACR4           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x28))
#define MCF_SCM_PACR5           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x29))
#define MCF_SCM_PACR6           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x2A))
#define MCF_SCM_PACR7           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x2B))
#define MCF_SCM_PACR8           (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x2C))
#define MCF_SCM_GPACR0          (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x30))
#define MCF_SCM_GPACR1          (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x31))

/* IPSBAR bit definitions */
#define MCF_SCM_IPSBAR_BA(x)    ((x) & 0xC0000000)
#define MCF_SCM_IPSBAR_V        (0x00000001)

/* RAMBAR bit definitions */
#define MCF_SCM_RAMBAR_BA(x)    ((x) & 0xFFFF0000)
#define MCF_SCM_RAMBAR_BDE      (0x00000200)

/* CRSR bit definitions - Core Reset Status Register */
#define MCF_SCM_CRSR_EXT        (0x80)  /* External reset */
#define MCF_SCM_CRSR_CWDR       (0x20)  /* Core watchdog reset */

/* CWCR bit definitions - Core Watchdog Control Register */
#define MCF_SCM_CWCR_CWE        (0x80)  /* Core watchdog enable */
#define MCF_SCM_CWCR_CWRI       (0x40)  /* Core watchdog reset/interrupt */
#define MCF_SCM_CWCR_CWT(x)     (((x) & 0x07) << 3)
#define MCF_SCM_CWCR_CWTA       (0x04)  /* Core watchdog transfer acknowledge */
#define MCF_SCM_CWCR_CWTAVAL    (0x02)  /* Core watchdog transfer acknowledge valid */
#define MCF_SCM_CWCR_CWTIF      (0x01)  /* Core watchdog timer interrupt flag */

/* LPICR bit definitions - Low Power Interrupt Control Register (see Chapter 9) */
#define MCF_SCM_LPICR_XIPL(x)        (((x)&0x07)<<4)
#define MCF_SCM_LPICR_ENBSTOP        (0x80)

/* CWSR bit definitions - Core Watchdog Service Register must be performed in this order 
   before time-out to prevent the core watchdog timer from interrupting */
#define MCF_SCM_CWSR_SEQ1       (0x55)  /* First write sequence */
#define MCF_SCM_CWSR_SEQ2       (0xAA)  /* Second write sequence */

/* Bit definitions and macros for MCF_SCM_PPMRH */
#define MCF_SCM_PPMRH_CDPORTS        (0x00000001)
#define MCF_SCM_PPMRH_CDEPORT        (0x00000002)
#define MCF_SCM_PPMRH_CDPIT0         (0x00000008)
#define MCF_SCM_PPMRH_CDPIT1         (0x00000010)
#define MCF_SCM_PPMRH_CDADC          (0x00000080)
#define MCF_SCM_PPMRH_CDGPT          (0x00000100)
#define MCF_SCM_PPMRH_CDPWN          (0x00000200)
#define MCF_SCM_PPMRH_CDFCAN         (0x00000400)
#define MCF_SCM_PPMRH_CDCFM          (0x00000800)

/* Bit definitions and macros for MCF_SCM_PPMRL */
#define MCF_SCM_PPMRL_CDG            (0x00000002)
#define MCF_SCM_PPMRL_CDEIM          (0x00000008)
#define MCF_SCM_PPMRL_CDDMA          (0x00000010)
#define MCF_SCM_PPMRL_CDUART0        (0x00000020)
#define MCF_SCM_PPMRL_CDUART1        (0x00000040)
#define MCF_SCM_PPMRL_CDUART2        (0x00000080)
#define MCF_SCM_PPMRL_CDI2C          (0x00000200)
#define MCF_SCM_PPMRL_CDQSPI         (0x00000400)
#define MCF_SCM_PPMRL_CDDTIM0        (0x00002000)
#define MCF_SCM_PPMRL_CDDTIM1        (0x00004000)
#define MCF_SCM_PPMRL_CDDTIM2        (0x00008000)
#define MCF_SCM_PPMRL_CDDTIM3        (0x00010000)
#define MCF_SCM_PPMRL_CDINTC0        (0x00020000)

/* Bit definitions and macros for MCF_SCM_PPMRS */
#define MCF_SCM_PPMRS_DISABLE_ALL    (64)
#define MCF_SCM_PPMRS_DISABLE_CFM    (43)
#define MCF_SCM_PPMRS_DISABLE_CAN    (42)
#define MCF_SCM_PPMRS_DISABLE_PWM    (41)
#define MCF_SCM_PPMRS_DISABLE_GPT    (40)
#define MCF_SCM_PPMRS_DISABLE_ADC    (39)
#define MCF_SCM_PPMRS_DISABLE_PIT1   (36)
#define MCF_SCM_PPMRS_DISABLE_PIT0   (35)
#define MCF_SCM_PPMRS_DISABLE_EPORT  (33)
#define MCF_SCM_PPMRS_DISABLE_PORTS  (32)
#define MCF_SCM_PPMRS_DISABLE_INTC   (17)
#define MCF_SCM_PPMRS_DISABLE_DTIM3  (16)
#define MCF_SCM_PPMRS_DISABLE_DTIM2  (15)
#define MCF_SCM_PPMRS_DISABLE_DTIM1  (14)
#define MCF_SCM_PPMRS_DISABLE_DTIM0  (13)
#define MCF_SCM_PPMRS_DISABLE_QSPI   (10)
#define MCF_SCM_PPMRS_DISABLE_I2C    (9)
#define MCF_SCM_PPMRS_DISABLE_UART2  (7)
#define MCF_SCM_PPMRS_DISABLE_UART1  (6)
#define MCF_SCM_PPMRS_DISABLE_UART0  (5)
#define MCF_SCM_PPMRS_DISABLE_DMA    (4)
#define MCF_SCM_PPMRS_SET_CDG        (1)

/* Bit definitions and macros for MCF_SCM_PPMRC */
#define MCF_SCM_PPMRC_ENABLE_ALL     (64)
#define MCF_SCM_PPMRC_ENABLE_CFM     (43)
#define MCF_SCM_PPMRC_ENABLE_CAN     (42)
#define MCF_SCM_PPMRC_ENABLE_PWM     (41)
#define MCF_SCM_PPMRC_ENABLE_GPT     (40)
#define MCF_SCM_PPMRC_ENABLE_ADC     (39)
#define MCF_SCM_PPMRC_ENABLE_PIT1    (36)
#define MCF_SCM_PPMRC_ENABLE_PIT0    (35)
#define MCF_SCM_PPMRC_ENABLE_EPORT   (33)
#define MCF_SCM_PPMRC_ENABLE_PORTS   (32)
#define MCF_SCM_PPMRC_ENABLE_INTC    (17)
#define MCF_SCM_PPMRC_ENABLE_DTIM3   (16)
#define MCF_SCM_PPMRC_ENABLE_DTIM2   (15)
#define MCF_SCM_PPMRC_ENABLE_DTIM1   (14)
#define MCF_SCM_PPMRC_ENABLE_DTIM0   (13)
#define MCF_SCM_PPMRC_ENABLE_QSPI    (10)
#define MCF_SCM_PPMRC_ENABLE_I2C     (9)
#define MCF_SCM_PPMRC_ENABLE_UART2   (7)
#define MCF_SCM_PPMRC_ENABLE_UART1   (6)
#define MCF_SCM_PPMRC_ENABLE_UART0   (5)
#define MCF_SCM_PPMRC_ENABLE_DMA     (4)
#define MCF_SCM_PPMRC_CLEAR_CDG      (1)

/* DMAREQC bit definitions - DMA Request Control */
#define MCF_SCM_DMAREQC_DMAC0(x)    (((x) & 0x0F) << 0)
#define MCF_SCM_DMAREQC_DMAC1(x)    (((x) & 0x0F) << 4)
#define MCF_SCM_DMAREQC_DMAC2(x)    (((x) & 0x0F) << 8)
#define MCF_SCM_DMAREQC_DMAC3(x)    (((x) & 0x0F) << 12)

/* MPARK bit definitions - Bus Master Park Register */
#define MCF_SCM_MPARK_LCKOUT_TIME(x)    (((x) & 0x0F) << 8)
#define MCF_SCM_MPARK_PRKLAST           (0x00001000)
#define MCF_SCM_MPARK_TIMEOUT           (0x00002000)
#define MCF_SCM_MPARK_FIXED             (0x00004000)
#define MCF_SCM_MPARK_M0_PRTY(x)        (((x) & 0x03) << 18)
#define MCF_SCM_MPARK_M2_PRTY(x)        (((x) & 0x03) << 22)
#define MCF_SCM_MPARK_M3_PRTY(x)        (((x) & 0x03) << 24)
#define MCF_SCM_MPARK_BCR24BIT          (0x01000000)
#define MCF_SCM_MPARK_M2_P_EN           (0x02000000)

/* MPR bit definitions - Master Privilege Register */
#define MCF_SCM_MPR_MPR0        (0x01)
#define MCF_SCM_MPR_MPR1        (0x02)
#define MCF_SCM_MPR_MPR2        (0x04)
#define MCF_SCM_MPR_MPR3        (0x08)

/* PACR bit definitions - Peripheral Access Control Register */
#define MCF_SCM_PACR_ACCESS_CTRL0(x)    (((x) & 0x07) << 0)
#define MCF_SCM_PACR_LOCK0              (0x08)
#define MCF_SCM_PACR_ACCESS_CTRL1(x)    (((x) & 0x07) << 4)
#define MCF_SCM_PACR_LOCK1              (0x80)

/* GPACR bit definitions - Grouped Peripheral Access Control Register */
#define MCF_SCM_GPACR_ACCESS_CTRL(x)    ((x) & 0x0F)
#define MCF_SCM_GPACR_LOCK              (0x80)

#endif /* MCF5223_SCM_H */
