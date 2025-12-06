/*
 * File:    mcf5223_intc_iack.h
 * Purpose: Register and bit definitions for Global IACK registers
 *
 * Based on MCF52235 Reference Manual, Chapter 15 (Sec. 15.3.8)
 */

#ifndef __MCF5223_INTC_IACK_H__
#define __MCF5223_INTC_IACK_H__

#include <stdint.h>

/*********************************************************************
*
* Interrupt Controller (INTC_IACK)
*
*********************************************************************/

/* Base address of Global IACK registers */
#define MCF_INTC_IACK_GSWIACK_BASE            (IPSBAR + 0x000FE0)

/* Register read/write macros */
#define MCF_INTC_IACK_GL1IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x04))
#define MCF_INTC_IACK_GL2IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x08))
#define MCF_INTC_IACK_GL3IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x0C))
#define MCF_INTC_IACK_GL4IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x10))
#define MCF_INTC_IACK_GL5IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x14))
#define MCF_INTC_IACK_GL6IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x18))
#define MCF_INTC_IACK_GL7IACK            (*(volatile uint8_t *)(MCF_INTC_IACK_GSWIACK_BASE + 0x1C))

#define MCF_INTC_IACK_GLIACK(x)       (*(volatile uint8_t *)( ( MCF_PWM_BASE + 0x04 ) + (x-1)*0x04 ) )

/* Bit definitions and macros for MCF_INTC_IACK_GSWIACK */
#define MCF_INTC_IACK_GSWIACK_VECTOR(x)  ( ( (x) & 0xFF ) << 0 )

/* Bit definitions and macros for MCF_INTC_IACK_GLIACK */
#define MCF_INTC_IACK_GLIACK_VECTOR(x)   ( ( (x) & 0xFF ) << 0 )

/********************************************************************/

#endif /* __MCF5223_INTC_IACK_H__ */
