/*
 * File:    mcf5223_cim.h
 * Purpose: Register and bit definitions for ColdFire Integration Module
 *
 * Based on MCF52235 Reference Manual, Chapter 7 - Reset Controller
 * The CIM registers are part of the System Control Module (SCM)
 * Base Address: IPSBAR + 0x000000
 */

#ifndef __MCF5223_CIM_H__
#define __MCF5223_CIM_H__

#include <stdint.h>

/*********************************************************************
*
* ColdFire Integration Module (CIM)
*
*********************************************************************/

/* Base address (part of SCM) */
#define MCF_CIM_BASE            (IPSBAR + 0x110000)

/* Register definitions */
#define MCF_CIM_RCR             (*(volatile uint8_t  *)(MCF_CIM_BASE + 0x00))
#define MCF_CIM_RSR             (*(volatile uint8_t  *)(MCF_CIM_BASE + 0x01))
#define MCF_CIM_CCR             (*(volatile uint16_t *)(MCF_CIM_BASE + 0x04))
#define MCF_CIM_LPCR            (*(volatile uint8_t  *)(MCF_CIM_BASE + 0x07))
#define MCF_CIM_RCON            (*(volatile uint16_t *)(MCF_CIM_BASE + 0x08))
#define MCF_CIM_CIR             (*(volatile uint16_t *)(MCF_CIM_BASE + 0x0A))

/* Bit definitions and macros for MCF_CIM_RCR */
#define MCF_CIM_RCR_LVDE        (0x01)
#define MCF_CIM_RCR_LVDRE       (0x04)
#define MCF_CIM_RCR_LVDIE       (0x08)
#define MCF_CIM_RCR_LVDF        (0x10)
#define MCF_CIM_RCR_FRCRSTOUT   (0x40)
#define MCF_CIM_RCR_SOFTRST     (0x80)

/* Bit definitions and macros for MCF_CIM_RSR */
#define MCF_CIM_RSR_LOL         (0x01)
#define MCF_CIM_RSR_LOC         (0x02)
#define MCF_CIM_RSR_EXT         (0x04)
#define MCF_CIM_RSR_POR         (0x08)
#define MCF_CIM_RSR_WDR         (0x10)
#define MCF_CIM_RSR_SOFT        (0x20)
#define MCF_CIM_RSR_LVD         (0x40)

/* Bit definitions and macros for MCF_CIM_CCR */
#define MCF_CIM_CCR_LOAD        (0x8000)

/* Bit definitions and macros for MCF_CIM_LPCR */
#define MCF_CIM_LPCR_LVDSE      (0x02)
#define MCF_CIM_LPCR_STPMD(x)   (((x)&0x03)<<3)
#define MCF_CIM_LPCR_LPMD(x)    (((x)&0x03)<<6)
#define MCF_CIM_LPCR_LPMD_STOP  (0xC0)
#define MCF_CIM_LPCR_LPMD_WAIT  (0x80)
#define MCF_CIM_LPCR_LPMD_DOZE  (0x40)
#define MCF_CIM_LPCR_LPMD_RUN   (0x00)

/* Bit definitions and macros for MCF_CIM_RCON */
#define MCF_CIM_RCON_RLOAD      (0x0020)

/* Bit definitions and macros for MCF_CIM_CIR */
#define MCF_CIM_CIR_PRN(x)      (((x)&0x003F)<<0)   /* Part revision number */
#define MCF_CIM_CIR_PIN(x)      (((x)&0x03FF)<<6)   /* Part identification number */

/********************************************************************/

#endif /* __MCF5223_CIM_H__ */
