/*
 * File:    mcf5223_scm.h
 * Purpose: System Control Module (SCM) register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
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
#define MCF_SCM_CWSR            (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x13))
#define MCF_SCM_DMAREQC         (*(volatile uint32_t *)(MCF_SCM_BASE + 0x14))
#define MCF_SCM_MPARK           (*(volatile uint32_t *)(MCF_SCM_BASE + 0x1C))
#define MCF_SCM_MPR             (*(volatile uint8_t  *)(MCF_SCM_BASE + 0x20))
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

/* CWSR bit definitions - Core Watchdog Service Register */
#define MCF_SCM_CWSR_SEQ1       (0x55)  /* First write sequence */
#define MCF_SCM_CWSR_SEQ2       (0xAA)  /* Second write sequence */

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
