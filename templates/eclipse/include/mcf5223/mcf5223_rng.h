/*
 * File:    mcf5223_rng.h
 * Purpose: Register and bit definitions for Random Number Generator
 *
 * Based on MCF52235 Reference Manual, Chapter 6 - Random Number Generator (RNG)
 * Base Address: IPSBAR + 0x1F0000
 */

#ifndef __MCF5223_RNG_H__
#define __MCF5223_RNG_H__

#include <stdint.h>

/*********************************************************************
*
* Random Number Generator (RNG)
*
*********************************************************************/

/* Base address */
#define MCF_RNG_BASE            (IPSBAR + 0x1F0000)

/* Register definitions */
#define MCF_RNG_RNGCR           (*(volatile uint32_t *)(MCF_RNG_BASE + 0x00))
#define MCF_RNG_RNGSR           (*(volatile uint32_t *)(MCF_RNG_BASE + 0x04))
#define MCF_RNG_RNGER           (*(volatile uint32_t *)(MCF_RNG_BASE + 0x08))
#define MCF_RNG_RNGOUT          (*(volatile uint32_t *)(MCF_RNG_BASE + 0x0C))

/* Bit definitions and macros for MCF_RNG_RNGCR */
#define MCF_RNG_RNGCR_GO          (0x00000001)  /* Go - Start RNG */
#define MCF_RNG_RNGCR_HA          (0x00000002)  /* High Assurance */
#define MCF_RNG_RNGCR_IM          (0x00000004)  /* Interrupt Mask */
#define MCF_RNG_RNGCR_CI          (0x00000008)  /* Clear Interrupt */

/* Bit definitions and macros for MCF_RNG_RNGSR */
#define MCF_RNG_RNGSR_SV          (0x00000001)  /* Security Violation */
#define MCF_RNG_RNGSR_LRS         (0x00000002)  /* Last Read Status */
#define MCF_RNG_RNGSR_FUF         (0x00000004)  /* FIFO Underflow */
#define MCF_RNG_RNGSR_EI          (0x00000008)  /* Error Interrupt */
#define MCF_RNG_RNGSR_OFL(x)      (((x)&0x000000FF)<<8)   /* Output FIFO Level */
#define MCF_RNG_RNGSR_OFS(x)      (((x)&0x000000FF)<<16)  /* Output FIFO Size */

/* Bit definitions and macros for MCF_RNG_RNGER */
#define MCF_RNG_RNGER_ENTROPY(x)  (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_RNG_RNGOUT */
#define MCF_RNG_RNGOUT_OUTPUT(x)  (((x)&0xFFFFFFFF)<<0)

/********************************************************************/

#endif /* __MCF5223_RNG_H__ */
