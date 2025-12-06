/*
 * File:    mcf5223_phy.h
 * Purpose: Register and bit definitions for Ethernet Physical Layer (EPHY)
 *
 * Based on MCF52235 Reference Manual, Chapter 19 - Ethernet Physical Layer (EPHY)
 * Base Address: IPSBAR + 0x1E0000
 *
 * Note: These are the EPHY control registers, not the MII PHY registers
 * which are accessed through the FEC MMFR/MSCR registers via MDIO.
 */

#ifndef __MCF5223_PHY_H__
#define __MCF5223_PHY_H__

#include <stdint.h>

/*********************************************************************
*
* Ethernet Physical Layer (EPHY)
*
*********************************************************************/

/* Base address */
#define MCF_PHY_BASE            (IPSBAR + 0x1E0000)

/* Register definitions */
#define MCF_PHY_EPHYCTL0        (*(volatile uint8_t *)(MCF_PHY_BASE + 0x00))
#define MCF_PHY_EPHYCTL1        (*(volatile uint8_t *)(MCF_PHY_BASE + 0x01))
#define MCF_PHY_EPHYSR          (*(volatile uint8_t *)(MCF_PHY_BASE + 0x02))

/* Bit definitions and macros for MCF_PHY_EPHYCTL0 */
#define MCF_PHY_EPHYCTL0_EPHYIEN     (0x01)  /* EPHY Interrupt Enable */
#define MCF_PHY_EPHYCTL0_EPHYWAI     (0x04)  /* EPHY Wait Mode */
#define MCF_PHY_EPHYCTL0_LEDEN       (0x08)  /* LED Enable */
#define MCF_PHY_EPHYCTL0_DIS10       (0x10)  /* Disable 10Base-T */
#define MCF_PHY_EPHYCTL0_DIS100      (0x20)  /* Disable 100Base-TX */
#define MCF_PHY_EPHYCTL0_ANDIS       (0x40)  /* Auto-negotiation Disable */
#define MCF_PHY_EPHYCTL0_EPHYEN      (0x80)  /* EPHY Enable */

/* Bit definitions and macros for MCF_PHY_EPHYCTL1 */
#define MCF_PHY_EPHYCTL1_PHYADDR(x)  (((x)&0x1F)<<0)  /* PHY Address */

/* Bit definitions and macros for MCF_PHY_EPHYSR */
#define MCF_PHY_EPHYSR_EPHYIF        (0x01)  /* EPHY Interrupt Flag */
#define MCF_PHY_EPHYSR_10DIS         (0x10)  /* 10Base-T Disabled Status */
#define MCF_PHY_EPHYSR_100DIS        (0x20)  /* 100Base-TX Disabled Status */

/********************************************************************/

#endif /* __MCF5223_PHY_H__ */
