/*
 * File:    mcf5223_dtim.h
 * Purpose: DMA Timer (DTIM) register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_DTIM_H
#define MCF5223_DTIM_H

#include <stdint.h>

/*********************************************************************
 * DMA Timer (DTIM)
 * Base Addresses:
 *   DTIM0: IPSBAR + 0x000400
 *   DTIM1: IPSBAR + 0x000440
 *   DTIM2: IPSBAR + 0x000480
 *   DTIM3: IPSBAR + 0x0004C0
 *********************************************************************/

#define MCF_DTIM0_BASE          (IPSBAR + 0x000400)
#define MCF_DTIM1_BASE          (IPSBAR + 0x000440)
#define MCF_DTIM2_BASE          (IPSBAR + 0x000480)
#define MCF_DTIM3_BASE          (IPSBAR + 0x0004C0)

/* Generic DTIM base address macro */
#define MCF_DTIM_BASE(n)        (IPSBAR + 0x000400 + ((n) * 0x40))

/*********************************************************************
 * Register definitions for DTIM0
 *********************************************************************/

#define MCF_DTIM0_DTMR          (*(volatile uint16_t *)(MCF_DTIM0_BASE + 0x00))
#define MCF_DTIM0_DTXMR         (*(volatile uint8_t  *)(MCF_DTIM0_BASE + 0x02))
#define MCF_DTIM0_DTER          (*(volatile uint8_t  *)(MCF_DTIM0_BASE + 0x03))
#define MCF_DTIM0_DTRR          (*(volatile uint32_t *)(MCF_DTIM0_BASE + 0x04))
#define MCF_DTIM0_DTCR          (*(volatile uint32_t *)(MCF_DTIM0_BASE + 0x08))
#define MCF_DTIM0_DTCN          (*(volatile uint32_t *)(MCF_DTIM0_BASE + 0x0C))

/*********************************************************************
 * Register definitions for DTIM1
 *********************************************************************/

#define MCF_DTIM1_DTMR          (*(volatile uint16_t *)(MCF_DTIM1_BASE + 0x00))
#define MCF_DTIM1_DTXMR         (*(volatile uint8_t  *)(MCF_DTIM1_BASE + 0x02))
#define MCF_DTIM1_DTER          (*(volatile uint8_t  *)(MCF_DTIM1_BASE + 0x03))
#define MCF_DTIM1_DTRR          (*(volatile uint32_t *)(MCF_DTIM1_BASE + 0x04))
#define MCF_DTIM1_DTCR          (*(volatile uint32_t *)(MCF_DTIM1_BASE + 0x08))
#define MCF_DTIM1_DTCN          (*(volatile uint32_t *)(MCF_DTIM1_BASE + 0x0C))

/*********************************************************************
 * Register definitions for DTIM2
 *********************************************************************/

#define MCF_DTIM2_DTMR          (*(volatile uint16_t *)(MCF_DTIM2_BASE + 0x00))
#define MCF_DTIM2_DTXMR         (*(volatile uint8_t  *)(MCF_DTIM2_BASE + 0x02))
#define MCF_DTIM2_DTER          (*(volatile uint8_t  *)(MCF_DTIM2_BASE + 0x03))
#define MCF_DTIM2_DTRR          (*(volatile uint32_t *)(MCF_DTIM2_BASE + 0x04))
#define MCF_DTIM2_DTCR          (*(volatile uint32_t *)(MCF_DTIM2_BASE + 0x08))
#define MCF_DTIM2_DTCN          (*(volatile uint32_t *)(MCF_DTIM2_BASE + 0x0C))

/*********************************************************************
 * Register definitions for DTIM3
 *********************************************************************/

#define MCF_DTIM3_DTMR          (*(volatile uint16_t *)(MCF_DTIM3_BASE + 0x00))
#define MCF_DTIM3_DTXMR         (*(volatile uint8_t  *)(MCF_DTIM3_BASE + 0x02))
#define MCF_DTIM3_DTER          (*(volatile uint8_t  *)(MCF_DTIM3_BASE + 0x03))
#define MCF_DTIM3_DTRR          (*(volatile uint32_t *)(MCF_DTIM3_BASE + 0x04))
#define MCF_DTIM3_DTCR          (*(volatile uint32_t *)(MCF_DTIM3_BASE + 0x08))
#define MCF_DTIM3_DTCN          (*(volatile uint32_t *)(MCF_DTIM3_BASE + 0x0C))

/*********************************************************************
 * Generic DTIM register macros (n = 0, 1, 2, or 3)
 *********************************************************************/

#define MCF_DTIM_DTMR(n)        (*(volatile uint16_t *)(MCF_DTIM_BASE(n) + 0x00))
#define MCF_DTIM_DTXMR(n)       (*(volatile uint8_t  *)(MCF_DTIM_BASE(n) + 0x02))
#define MCF_DTIM_DTER(n)        (*(volatile uint8_t  *)(MCF_DTIM_BASE(n) + 0x03))
#define MCF_DTIM_DTRR(n)        (*(volatile uint32_t *)(MCF_DTIM_BASE(n) + 0x04))
#define MCF_DTIM_DTCR(n)        (*(volatile uint32_t *)(MCF_DTIM_BASE(n) + 0x08))
#define MCF_DTIM_DTCN(n)        (*(volatile uint32_t *)(MCF_DTIM_BASE(n) + 0x0C))

/*********************************************************************
 * DTMR bit definitions - DMA Timer Mode Register
 *********************************************************************/

#define MCF_DTIM_DTMR_RST       (0x0001)    /* Reset */
#define MCF_DTIM_DTMR_CLK(x)    (((x) & 0x03) << 1)  /* Clock source */
#define MCF_DTIM_DTMR_CLK_STOP  (0x0000)    /* Stop count */
#define MCF_DTIM_DTMR_CLK_DIV1  (0x0002)    /* System clock / 1 */
#define MCF_DTIM_DTMR_CLK_DIV16 (0x0004)    /* System clock / 16 */
#define MCF_DTIM_DTMR_CLK_TIN   (0x0006)    /* TINn input */

#define MCF_DTIM_DTMR_FRR       (0x0008)    /* Free run/restart */
#define MCF_DTIM_DTMR_ORRI      (0x0010)    /* Output reference request/interrupt enable */
#define MCF_DTIM_DTMR_OM        (0x0020)    /* Output mode */
#define MCF_DTIM_DTMR_CE(x)     (((x) & 0x03) << 6)  /* Capture edge */
#define MCF_DTIM_DTMR_CE_NONE   (0x0000)    /* Disable capture */
#define MCF_DTIM_DTMR_CE_RISE   (0x0040)    /* Capture on rising edge */
#define MCF_DTIM_DTMR_CE_FALL   (0x0080)    /* Capture on falling edge */
#define MCF_DTIM_DTMR_CE_ANY    (0x00C0)    /* Capture on any edge */

#define MCF_DTIM_DTMR_PS(x)     (((x) & 0xFF) << 8)  /* Prescaler */

/*********************************************************************
 * DTXMR bit definitions - DMA Timer Extended Mode Register
 *********************************************************************/

#define MCF_DTIM_DTXMR_MODE16   (0x01)      /* 16-bit mode */
#define MCF_DTIM_DTXMR_HALTED   (0x40)      /* Halted (read-only) */
#define MCF_DTIM_DTXMR_DMAEN    (0x80)      /* DMA request enable */

/*********************************************************************
 * DTER bit definitions - DMA Timer Event Register
 *********************************************************************/

#define MCF_DTIM_DTER_CAP       (0x01)      /* Capture event */
#define MCF_DTIM_DTER_REF       (0x02)      /* Output reference event */

/*********************************************************************
 * Timer calculation helpers
 *
 * Timer period = (DTRR + 1) * (PS + 1) * CLK_DIV / System Clock
 *
 * For 60MHz system clock, 1ms period, CLK_DIV=1, PS=0:
 *   DTRR = (60MHz * 1ms / 1 / 1) - 1 = 60000 - 1 = 59999
 *
 * Maximum period with PS=255, CLK_DIV=16, DTRR=0xFFFFFFFF:
 *   = (2^32) * 256 * 16 / 60MHz = ~4660 seconds
 *********************************************************************/

/* Calculate DTRR for desired period in microseconds (PS=0, CLK_DIV=1) */
#define MCF_DTIM_CALC_DTRR_US(sysclk_mhz, period_us) \
    (((uint32_t)(sysclk_mhz) * (period_us)) - 1)

/* Calculate DTRR for desired period in milliseconds (PS=0, CLK_DIV=1) */
#define MCF_DTIM_CALC_DTRR_MS(sysclk_mhz, period_ms) \
    (((uint32_t)(sysclk_mhz) * 1000 * (period_ms)) - 1)

#endif /* MCF5223_DTIM_H */
