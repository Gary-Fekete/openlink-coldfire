/*
 * File:    mcf5223_clock.h
 * Purpose: Clock Module register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_CLOCK_H
#define MCF5223_CLOCK_H

#include <stdint.h>

/*********************************************************************
 * Clock Module
 * Base Address: IPSBAR + 0x120000
 *********************************************************************/

#define MCF_CLOCK_BASE          (IPSBAR + 0x120000)

/* Register definitions */
#define MCF_CLOCK_SYNCR         (*(volatile uint16_t *)(MCF_CLOCK_BASE + 0x00))
#define MCF_CLOCK_SYNSR         (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x02))
#define MCF_CLOCK_ROCR          (*(volatile uint16_t *)(MCF_CLOCK_BASE + 0x04))
#define MCF_CLOCK_LPCR          (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x07))
#define MCF_CLOCK_CCHR          (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x08))
#define MCF_CLOCK_CCLR          (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x09))
#define MCF_CLOCK_OCHR          (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x0A))
#define MCF_CLOCK_OCLR          (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x0B))
#define MCF_CLOCK_RTCCR         (*(volatile uint8_t  *)(MCF_CLOCK_BASE + 0x12))

/* SYNCR bit definitions - Synthesizer Control Register */
#define MCF_CLOCK_SYNCR_PLLEN   (0x0001)    /* PLL enable */
#define MCF_CLOCK_SYNCR_PLLMODE (0x0002)    /* PLL mode select */
#define MCF_CLOCK_SYNCR_CLKSRC  (0x0004)    /* Clock source */
#define MCF_CLOCK_SYNCR_FWKUP   (0x0020)    /* Fast wakeup */
#define MCF_CLOCK_SYNCR_DISCLK  (0x0040)    /* Disable CLKOUT */
#define MCF_CLOCK_SYNCR_LOCEN   (0x0080)    /* Loss of clock enable */
#define MCF_CLOCK_SYNCR_RFD(x)  (((x) & 0x07) << 8)     /* Reduced frequency divider */
#define MCF_CLOCK_SYNCR_LOCRE   (0x0800)    /* Loss of clock reset enable */
#define MCF_CLOCK_SYNCR_MFD(x)  (((x) & 0x07) << 12)    /* Multiplication factor divider */
#define MCF_CLOCK_SYNCR_LOLRE   (0x8000)    /* Loss of lock reset enable */

/* SYNSR bit definitions - Synthesizer Status Register */
#define MCF_CLOCK_SYNSR_LOCS    (0x04)      /* Loss of clock status */
#define MCF_CLOCK_SYNSR_LOCK    (0x08)      /* PLL lock status */
#define MCF_CLOCK_SYNSR_LOCKS   (0x10)      /* Sticky lock status */
#define MCF_CLOCK_SYNSR_CRYOSC  (0x20)      /* Crystal oscillator output */
#define MCF_CLOCK_SYNSR_OCOSC   (0x40)      /* On-chip oscillator output */
#define MCF_CLOCK_SYNSR_EXTOSC  (0x80)      /* External oscillator status */

/* ROCR bit definitions - Relaxation Oscillator Control Register */
#define MCF_CLOCK_ROCR_TRIM(x)  ((x) & 0x03FF)

/* LPCR bit definitions - Low Power Control Register */
#define MCF_CLOCK_LPCR_LVDSE    (0x02)      /* Low voltage detect stop enable */
#define MCF_CLOCK_LPCR_STPMD(x) (((x) & 0x03) << 3)     /* Stop mode select */
#define MCF_CLOCK_LPCR_LPMD(x)  (((x) & 0x03) << 6)     /* Low power mode select */

/* Low power mode values */
#define MCF_CLOCK_LPCR_LPMD_RUN     (0x00 << 6)
#define MCF_CLOCK_LPCR_LPMD_DOZE    (0x01 << 6)
#define MCF_CLOCK_LPCR_LPMD_WAIT    (0x02 << 6)
#define MCF_CLOCK_LPCR_LPMD_STOP    (0x03 << 6)

/* CCHR bit definitions - Clock Control High Register */
#define MCF_CLOCK_CCHR_CCHR(x)  ((x) & 0x07)

/* CCLR bit definitions - Clock Control Low Register */
#define MCF_CLOCK_CCLR_OSCSEL0  (0x01)      /* Oscillator select 0 */
#define MCF_CLOCK_CCLR_OSCSEL1  (0x02)      /* Oscillator select 1 */

/* RTCCR bit definitions - RTC Clock Control Register */
#define MCF_CLOCK_RTCCR_RTCSEL  (0x01)      /* RTC clock select */
#define MCF_CLOCK_RTCCR_LPEN    (0x02)      /* RTC low power enable */
#define MCF_CLOCK_RTCCR_REFS    (0x04)      /* Reference select */
#define MCF_CLOCK_RTCCR_KHZEN   (0x08)      /* 1kHz enable */
#define MCF_CLOCK_RTCCR_OSCEN   (0x10)      /* Oscillator enable */
#define MCF_CLOCK_RTCCR_EXTALEN (0x40)      /* External clock enable */

/*********************************************************************
 * Clock calculation helpers
 *
 * System Clock = (Crystal Freq * (MFD + 2)) / ((RFD + 1) * 2)
 *
 * For 25MHz crystal with MFD=4, RFD=0:
 * Fsys = (25MHz * 6) / 2 = 75MHz (max for MCF52235)
 *
 * For 60MHz system clock with 25MHz crystal:
 * MFD=3, RFD=0: Fsys = (25 * 5) / 2 = 62.5MHz
 *********************************************************************/

#define MCF_CLOCK_MFD_2X    (0)     /* Multiply by 2 */
#define MCF_CLOCK_MFD_3X    (1)     /* Multiply by 3 */
#define MCF_CLOCK_MFD_4X    (2)     /* Multiply by 4 */
#define MCF_CLOCK_MFD_5X    (3)     /* Multiply by 5 */
#define MCF_CLOCK_MFD_6X    (4)     /* Multiply by 6 */
#define MCF_CLOCK_MFD_7X    (5)     /* Multiply by 7 */
#define MCF_CLOCK_MFD_8X    (6)     /* Multiply by 8 */
#define MCF_CLOCK_MFD_9X    (7)     /* Multiply by 9 */

#define MCF_CLOCK_RFD_DIV1  (0)     /* Divide by 1 */
#define MCF_CLOCK_RFD_DIV2  (1)     /* Divide by 2 */
#define MCF_CLOCK_RFD_DIV4  (2)     /* Divide by 4 */
#define MCF_CLOCK_RFD_DIV8  (3)     /* Divide by 8 */
#define MCF_CLOCK_RFD_DIV16 (4)     /* Divide by 16 */
#define MCF_CLOCK_RFD_DIV32 (5)     /* Divide by 32 */
#define MCF_CLOCK_RFD_DIV64 (6)     /* Divide by 64 */
#define MCF_CLOCK_RFD_DIV128 (7)    /* Divide by 128 */

#endif /* MCF5223_CLOCK_H */
