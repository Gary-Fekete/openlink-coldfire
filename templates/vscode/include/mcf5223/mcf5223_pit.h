/*
 * File:    mcf5223_pit.h
 * Purpose: Programmable Interrupt Timer (PIT) register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_PIT_H
#define MCF5223_PIT_H

#include <stdint.h>

/*********************************************************************
 * Programmable Interrupt Timer (PIT)
 * Base Addresses:
 *   PIT0: IPSBAR + 0x150000
 *   PIT1: IPSBAR + 0x160000
 *********************************************************************/

#define MCF_PIT0_BASE           (IPSBAR + 0x150000)
#define MCF_PIT1_BASE           (IPSBAR + 0x160000)

/* Generic PIT base address macro */
#define MCF_PIT_BASE(n)         (IPSBAR + 0x150000 + ((n) * 0x10000))

/*********************************************************************
 * PIT0 Register definitions
 *********************************************************************/

#define MCF_PIT0_PCSR           (*(volatile uint16_t *)(MCF_PIT0_BASE + 0x00))
#define MCF_PIT0_PMR            (*(volatile uint16_t *)(MCF_PIT0_BASE + 0x02))
#define MCF_PIT0_PCNTR          (*(volatile uint16_t *)(MCF_PIT0_BASE + 0x04))

/*********************************************************************
 * PIT1 Register definitions
 *********************************************************************/

#define MCF_PIT1_PCSR           (*(volatile uint16_t *)(MCF_PIT1_BASE + 0x00))
#define MCF_PIT1_PMR            (*(volatile uint16_t *)(MCF_PIT1_BASE + 0x02))
#define MCF_PIT1_PCNTR          (*(volatile uint16_t *)(MCF_PIT1_BASE + 0x04))

/*********************************************************************
 * Generic PIT register macros (n = 0 or 1)
 *********************************************************************/

#define MCF_PIT_PCSR(n)         (*(volatile uint16_t *)(MCF_PIT_BASE(n) + 0x00))
#define MCF_PIT_PMR(n)          (*(volatile uint16_t *)(MCF_PIT_BASE(n) + 0x02))
#define MCF_PIT_PCNTR(n)        (*(volatile uint16_t *)(MCF_PIT_BASE(n) + 0x04))

/*********************************************************************
 * PCSR bit definitions - PIT Control and Status Register
 *********************************************************************/

#define MCF_PIT_PCSR_EN         (0x0001)    /* PIT Enable */
#define MCF_PIT_PCSR_RLD        (0x0002)    /* Reload */
#define MCF_PIT_PCSR_PIF        (0x0004)    /* PIT Interrupt Flag */
#define MCF_PIT_PCSR_PIE        (0x0008)    /* PIT Interrupt Enable */
#define MCF_PIT_PCSR_OVW        (0x0010)    /* Overwrite */
#define MCF_PIT_PCSR_DBG        (0x0020)    /* Debug */
#define MCF_PIT_PCSR_DOZE       (0x0040)    /* Doze mode */
#define MCF_PIT_PCSR_HALTED     (0x0080)    /* Halted (read-only) */
#define MCF_PIT_PCSR_PRE(x)     (((x) & 0x0F) << 8)  /* Prescaler */

/* Prescaler values */
#define MCF_PIT_PCSR_PRE_1      (0x0000)    /* Divide by 1 */
#define MCF_PIT_PCSR_PRE_2      (0x0100)    /* Divide by 2 */
#define MCF_PIT_PCSR_PRE_4      (0x0200)    /* Divide by 4 */
#define MCF_PIT_PCSR_PRE_8      (0x0300)    /* Divide by 8 */
#define MCF_PIT_PCSR_PRE_16     (0x0400)    /* Divide by 16 */
#define MCF_PIT_PCSR_PRE_32     (0x0500)    /* Divide by 32 */
#define MCF_PIT_PCSR_PRE_64     (0x0600)    /* Divide by 64 */
#define MCF_PIT_PCSR_PRE_128    (0x0700)    /* Divide by 128 */
#define MCF_PIT_PCSR_PRE_256    (0x0800)    /* Divide by 256 */
#define MCF_PIT_PCSR_PRE_512    (0x0900)    /* Divide by 512 */
#define MCF_PIT_PCSR_PRE_1024   (0x0A00)    /* Divide by 1024 */
#define MCF_PIT_PCSR_PRE_2048   (0x0B00)    /* Divide by 2048 */
#define MCF_PIT_PCSR_PRE_4096   (0x0C00)    /* Divide by 4096 */
#define MCF_PIT_PCSR_PRE_8192   (0x0D00)    /* Divide by 8192 */
#define MCF_PIT_PCSR_PRE_16384  (0x0E00)    /* Divide by 16384 */
#define MCF_PIT_PCSR_PRE_32768  (0x0F00)    /* Divide by 32768 */

/*********************************************************************
 * Timer calculation helpers
 *
 * Timer period = (PMR + 1) * Prescaler / System Clock
 *
 * For 60MHz system clock and 1ms period with PRE=1:
 *   PMR = (60MHz * 1ms / 1) - 1 = 60000 - 1 = 59999
 *
 * For 60MHz system clock and 1s period with PRE=32768:
 *   PMR = (60MHz * 1s / 32768) - 1 = 1831 - 1 = 1830
 *********************************************************************/

/* Calculate PMR value for desired period in microseconds */
#define MCF_PIT_CALC_PMR_US(sysclk_mhz, prescaler, period_us) \
    ((((uint32_t)(sysclk_mhz) * (period_us)) / (prescaler)) - 1)

/* Calculate PMR value for desired period in milliseconds */
#define MCF_PIT_CALC_PMR_MS(sysclk_mhz, prescaler, period_ms) \
    ((((uint32_t)(sysclk_mhz) * 1000 * (period_ms)) / (prescaler)) - 1)

#endif /* MCF5223_PIT_H */
