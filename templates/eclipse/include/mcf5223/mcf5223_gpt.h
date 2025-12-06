/*
 * File:    mcf5223_gpt.h
 * Purpose: Register and bit definitions for General Purpose Timer
 *
 * Based on MCF52235 Reference Manual, Chapter 23 - General Purpose Timer (GPT)
 * Base Address: IPSBAR + 0x1A0000
 */

#ifndef __MCF5223_GPT_H__
#define __MCF5223_GPT_H__

#include <stdint.h>

/*********************************************************************
*
* General Purpose Timer Module (GPT)
*
*********************************************************************/

/* Base address */
#define MCF_GPT_BASE            (IPSBAR + 0x1A0000)

/* Register definitions */
#define MCF_GPT_GPTIOS          (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x00))
#define MCF_GPT_GPTCFORC        (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x01))
#define MCF_GPT_GPTOC3M         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x02))
#define MCF_GPT_GPTOC3D         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x03))
#define MCF_GPT_GPTCNT          (*(volatile uint16_t *)(MCF_GPT_BASE + 0x04))
#define MCF_GPT_GPTSCR1         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x06))
#define MCF_GPT_GPTTOV          (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x08))
#define MCF_GPT_GPTCTL1         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x09))
#define MCF_GPT_GPTCTL2         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x0B))
#define MCF_GPT_GPTIE           (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x0C))
#define MCF_GPT_GPTSCR2         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x0D))
#define MCF_GPT_GPTFLG1         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x0E))
#define MCF_GPT_GPTFLG2         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x0F))
#define MCF_GPT_GPTC0           (*(volatile uint16_t *)(MCF_GPT_BASE + 0x10))
#define MCF_GPT_GPTC1           (*(volatile uint16_t *)(MCF_GPT_BASE + 0x12))
#define MCF_GPT_GPTC2           (*(volatile uint16_t *)(MCF_GPT_BASE + 0x14))
#define MCF_GPT_GPTC3           (*(volatile uint16_t *)(MCF_GPT_BASE + 0x16))
#define MCF_GPT_GPTPACTL        (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x18))
#define MCF_GPT_GPTPAFLG        (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x19))
#define MCF_GPT_GPTPACNT        (*(volatile uint16_t *)(MCF_GPT_BASE + 0x1A))
#define MCF_GPT_GPTPORT         (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x1D))
#define MCF_GPT_GPTDDR          (*(volatile uint8_t  *)(MCF_GPT_BASE + 0x1E))

/* Bit definitions and macros for MCF_GPT_GPTIOS */
#define MCF_GPT_GPTIOS_IOS(x)           (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTCFORC */
#define MCF_GPT_GPTCFORC_FOC(x)         (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTOC3M */
#define MCF_GPT_GPTOC3M_OC3M(x)         (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTOC3D */
#define MCF_GPT_GPTOC3D_OC3D(x)         (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTCNT */
#define MCF_GPT_GPTCNT_CNTR(x)          (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_GPT_GPTSCR1 */
#define MCF_GPT_GPTSCR1_TFFCA           (0x10)
#define MCF_GPT_GPTSCR1_GPTEN           (0x80)

/* Bit definitions and macros for MCF_GPT_GPTTOV */
#define MCF_GPT_GPTTOV_TOV(x)           (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTCTL1 */
#define MCF_GPT_GPTCTL1_OL0             (0x01)
#define MCF_GPT_GPTCTL1_OM0             (0x02)
#define MCF_GPT_GPTCTL1_OL1             (0x04)
#define MCF_GPT_GPTCTL1_OM1             (0x08)
#define MCF_GPT_GPTCTL1_OL2             (0x10)
#define MCF_GPT_GPTCTL1_OM2             (0x20)
#define MCF_GPT_GPTCTL1_OL3             (0x40)
#define MCF_GPT_GPTCTL1_OM3             (0x80)

/* Bit definitions and macros for MCF_GPT_GPTCTL2 */
#define MCF_GPT_GPTCTL2_EDG0A           (0x01)
#define MCF_GPT_GPTCTL2_EDG0B           (0x02)
#define MCF_GPT_GPTCTL2_EDG1A           (0x04)
#define MCF_GPT_GPTCTL2_EDG1B           (0x08)
#define MCF_GPT_GPTCTL2_EDG2A           (0x10)
#define MCF_GPT_GPTCTL2_EDG2B           (0x20)
#define MCF_GPT_GPTCTL2_EDG3A           (0x40)
#define MCF_GPT_GPTCTL2_EDG3B           (0x80)

/* Bit definitions and macros for MCF_GPT_GPTIE */
#define MCF_GPT_GPTIE_CI(x)             (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTSCR2 */
#define MCF_GPT_GPTSCR2_PR(x)           (((x)&0x07)<<0)
#define MCF_GPT_GPTSCR2_TCRE            (0x08)
#define MCF_GPT_GPTSCR2_RDPT            (0x10)
#define MCF_GPT_GPTSCR2_PUPT            (0x20)
#define MCF_GPT_GPTSCR2_TOI             (0x80)

/* Bit definitions and macros for MCF_GPT_GPTFLG1 */
#define MCF_GPT_GPTFLG1_CF(x)           (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTFLG2 */
#define MCF_GPT_GPTFLG2_CF(x)           (((x)&0x0F)<<0)
#define MCF_GPT_GPTFLG2_TOF             (0x80)

/* Bit definitions and macros for MCF_GPT_GPTC0 */
#define MCF_GPT_GPTC0_CCNT(x)           (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_GPT_GPTC1 */
#define MCF_GPT_GPTC1_CCNT(x)           (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_GPT_GPTC2 */
#define MCF_GPT_GPTC2_CCNT(x)           (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_GPT_GPTC3 */
#define MCF_GPT_GPTC3_CCNT(x)           (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_GPT_GPTPACTL */
#define MCF_GPT_GPTPACTL_PAI            (0x01)
#define MCF_GPT_GPTPACTL_PAOVI          (0x02)
#define MCF_GPT_GPTPACTL_CLK(x)         (((x)&0x03)<<2)
#define MCF_GPT_GPTPACTL_PEDGE          (0x10)
#define MCF_GPT_GPTPACTL_PAMOD          (0x20)
#define MCF_GPT_GPTPACTL_PAE            (0x40)

/* Bit definitions and macros for MCF_GPT_GPTPAFLG */
#define MCF_GPT_GPTPAFLG_PAIF           (0x01)
#define MCF_GPT_GPTPAFLG_PAOVF          (0x02)

/* Bit definitions and macros for MCF_GPT_GPTPACNT */
#define MCF_GPT_GPTPACNT_PACNT(x)       (((x)&0xFFFF)<<0)

/* Bit definitions and macros for MCF_GPT_GPTPORT */
#define MCF_GPT_GPTPORT_PORTT(x)        (((x)&0x0F)<<0)

/* Bit definitions and macros for MCF_GPT_GPTDDR */
#define MCF_GPT_GPTDDR_DDRT(x)          (((x)&0x0F)<<0)

/********************************************************************/

#endif /* __MCF5223_GPT_H__ */
