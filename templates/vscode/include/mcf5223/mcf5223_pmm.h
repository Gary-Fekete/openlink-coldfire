/*
 * File:    mcf5223_pmm.h
 * Purpose: Register and bit definitions for Power Management Module
 *
 * Based on MCF52235 Reference Manual, Chapter 9 - Power Management
 * Base Address: IPSBAR + 0x000000 (part of SCM)
 */

#ifndef __MCF5223_PMM_H__
#define __MCF5223_PMM_H__

#include <stdint.h>

/*********************************************************************
*
* Power Management Module (PMM)
*
*********************************************************************/

/* Base address (part of SCM) */
#define MCF_PMM_BASE            (IPSBAR + 0x000000)

/* Register definitions */
#define MCF_PMM_PPMRH           (*(volatile uint32_t *)(MCF_PMM_BASE + 0x0C))
#define MCF_PMM_PPMRL           (*(volatile uint32_t *)(MCF_PMM_BASE + 0x18))
#define MCF_PMM_LPICR           (*(volatile uint8_t  *)(MCF_PMM_BASE + 0x12))
#define MCF_PMM_LPCR            (*(volatile uint8_t  *)(MCF_PMM_BASE + 0x110007))

/* Bit definitions and macros for MCF_PMM_PPMRH */
#define MCF_PMM_PPMRH_CDPORTS   (0x00000001)
#define MCF_PMM_PPMRH_CDEPORT   (0x00000002)
#define MCF_PMM_PPMRH_CDPIT0    (0x00000008)
#define MCF_PMM_PPMRH_CDPIT1    (0x00000010)
#define MCF_PMM_PPMRH_CDADC     (0x00000080)
#define MCF_PMM_PPMRH_CDGPT     (0x00000100)
#define MCF_PMM_PPMRH_CDPWM     (0x00000200)
#define MCF_PMM_PPMRH_CDFCAN    (0x00000400)
#define MCF_PMM_PPMRH_CDCFM     (0x00000800)

/* Bit definitions and macros for MCF_PMM_PPMRL */
#define MCF_PMM_PPMRL_CDG       (0x00000002)
#define MCF_PMM_PPMRL_CDEIM     (0x00000008)
#define MCF_PMM_PPMRL_CDDMA     (0x00000010)
#define MCF_PMM_PPMRL_CDUART0   (0x00000020)
#define MCF_PMM_PPMRL_CDUART1   (0x00000040)
#define MCF_PMM_PPMRL_CDUART2   (0x00000080)
#define MCF_PMM_PPMRL_CDI2C     (0x00000200)
#define MCF_PMM_PPMRL_CDQSPI    (0x00000400)
#define MCF_PMM_PPMRL_CDDTIM0   (0x00002000)
#define MCF_PMM_PPMRL_CDDTIM1   (0x00004000)
#define MCF_PMM_PPMRL_CDDTIM2   (0x00008000)
#define MCF_PMM_PPMRL_CDDTIM3   (0x00010000)
#define MCF_PMM_PPMRL_CDINTC0   (0x00020000)

/* Bit definitions and macros for MCF_PMM_LPICR */
#define MCF_PMM_LPICR_XIPL(x)   (((x)&0x07)<<4)
#define MCF_PMM_LPICR_ENBSTOP   (0x80)

/* Bit definitions and macros for MCF_PMM_LPCR */
#define MCF_PMM_LPCR_LVDSE      (0x02)
#define MCF_PMM_LPCR_STPMD(x)   (((x)&0x03)<<3)
#define MCF_PMM_LPCR_LPMD(x)    (((x)&0x03)<<6)
#define MCF_PMM_LPCR_LPMD_STOP  (0xC0)
#define MCF_PMM_LPCR_LPMD_WAIT  (0x80)
#define MCF_PMM_LPCR_LPMD_DOZE  (0x40)
#define MCF_PMM_LPCR_LPMD_RUN   (0x00)

/********************************************************************/

#endif /* __MCF5223_PMM_H__ */
