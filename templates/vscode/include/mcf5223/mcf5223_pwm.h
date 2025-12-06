/*
 * File:    mcf5223_pwm.h
 * Purpose: Register and bit definitions for Pulse-Width Modulation (PWM) Module
 *
 * Based on MCF52235 Reference Manual, Chapter 29
 * Base Address: IPSBAR + 0x1B0000
 */

#ifndef __MCF5223_PWM_H__
#define __MCF5223_PWM_H__

#include <stdint.h>

/*********************************************************************
*
* PULSE-WIDTH Module (PWM)
*
*********************************************************************/

/* Base address of PWM */
#define MCF_PWM_BASE            (IPSBAR + 0x1B0000)

/* Register definitions */
#define MCF_PWM_PWME            (*(volatile uint8_t *)(MCF_PWM_BASE + 0x00))
#define MCF_PWM_PWMPOL          (*(volatile uint8_t *)(MCF_PWM_BASE + 0x01))
#define MCF_PWM_PWMCLK          (*(volatile uint8_t *)(MCF_PWM_BASE + 0x02))
#define MCF_PWM_PWMPRCLK        (*(volatile uint8_t *)(MCF_PWM_BASE + 0x03))
#define MCF_PWM_PWMCAE          (*(volatile uint8_t *)(MCF_PWM_BASE + 0x04))
#define MCF_PWM_PWMCTL          (*(volatile uint8_t *)(MCF_PWM_BASE + 0x05))
#define MCF_PWM_PWMSCLA         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x08))
#define MCF_PWM_PWMSCLB         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x09))
#define MCF_PWM_PWMCNT0         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x0C))
#define MCF_PWM_PWMCNT1         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x0D))
#define MCF_PWM_PWMCNT2         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x0E))
#define MCF_PWM_PWMCNT3         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x0F))
#define MCF_PWM_PWMCNT4         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x10))
#define MCF_PWM_PWMCNT5         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x11))
#define MCF_PWM_PWMCNT6         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x12))
#define MCF_PWM_PWMCNT7         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x13))
#define MCF_PWM_PWMCNT(x)       (*(volatile uint8_t *)(MCF_PWM_BASE + 0x0C + (x)))
#define MCF_PWM_PWMPER0         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x14))
#define MCF_PWM_PWMPER1         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x15))
#define MCF_PWM_PWMPER2         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x16))
#define MCF_PWM_PWMPER3         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x17))
#define MCF_PWM_PWMPER4         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x18))
#define MCF_PWM_PWMPER5         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x19))
#define MCF_PWM_PWMPER6         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1A))
#define MCF_PWM_PWMPER7         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1B))
#define MCF_PWM_PWMPER(x)       (*(volatile uint8_t *)(MCF_PWM_BASE + 0x14 + (x)))
#define MCF_PWM_PWMDTY0         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1C))
#define MCF_PWM_PWMDTY1         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1D))
#define MCF_PWM_PWMDTY2         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1E))
#define MCF_PWM_PWMDTY3         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1F))
#define MCF_PWM_PWMDTY4         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x20))
#define MCF_PWM_PWMDTY5         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x21))
#define MCF_PWM_PWMDTY6         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x22))
#define MCF_PWM_PWMDTY7         (*(volatile uint8_t *)(MCF_PWM_BASE + 0x23))
#define MCF_PWM_PWMDTY(x)       (*(volatile uint8_t *)(MCF_PWM_BASE + 0x1C + (x)))


/* Bit definitions and macros for MCF_PWM_PWME */
#define MCF_PWM_PWME_PWME0         (0x01)
#define MCF_PWM_PWME_PWME1         (0x02)
#define MCF_PWM_PWME_PWME2         (0x04)
#define MCF_PWM_PWME_PWME3         (0x08)

/* Bit definitions and macros for MCF_PWM_PWMPOL */
#define MCF_PWM_PWMPOL_PPOL0       (0x01)
#define MCF_PWM_PWMPOL_PPOL1       (0x02)
#define MCF_PWM_PWMPOL_PPOL2       (0x04)
#define MCF_PWM_PWMPOL_PPOL3       (0x08)

/* Bit definitions and macros for MCF_PWM_PWMCLK */
#define MCF_PWM_PWMCLK_PCLK0       (0x01)
#define MCF_PWM_PWMCLK_PCLK1       (0x02)
#define MCF_PWM_PWMCLK_PCLK2       (0x04)
#define MCF_PWM_PWMCLK_PCLK3       (0x08)

/* Bit definitions and macros for MCF_PWM_PWMPRCLK */
#define MCF_PWM_PWMPRCLK_PCKA(x)   (( (x) & 0x07 ) << 0 )
#define MCF_PWM_PWMPRCLK_PCKB(x)   (( (x) & 0x07 ) << 4 ) 

/* Bit definitions and macros for MCF_PWM_PWMCAE */
#define MCF_PWM_PWMCAE_CAE0        (0x01)
#define MCF_PWM_PWMCAE_CAE1        (0x02)
#define MCF_PWM_PWMCAE_CAE2        (0x04)
#define MCF_PWM_PWMCAE_CAE3        (0x08)

/* Bit definitions and macros for MCF_PWM_PWMCTL */
#define MCF_PWM_PWMCTL_PFRZ        (0x04)
#define MCF_PWM_PWMCTL_PSWAI       (0x08)
#define MCF_PWM_PWMCTL_CON01       (0x10)
#define MCF_PWM_PWMCTL_CON23       (0x20)

/* Bit definitions and macros for MCF_PWM_PWMSCLA */
#define MCF_PWM_PWMSCLA_SCALEA(x)  (( (x) & 0xFF ) << 0 )

/* Bit definitions and macros for MCF_PWM_PWMSCLB */
#define MCF_PWM_PWMSCLB_SCALEB(x)  (( (x) & 0xFF ) << 0 )

/* Bit definitions and macros for MCF_PWM_PWMCNT */
#define MCF_PWM_PWMCNT_COUNT(x)    (( (x) & 0xFF ) << 0 )

/* Bit definitions and macros for MCF_PWM_PWMPER */
#define MCF_PWM_PWMPER_PERIOD(x)   (( (x) & 0xFF ) << 0 )

/* Bit definitions and macros for MCF_PWM_PWMDTY */
#define MCF_PWM_PWMDTY_DUTY(x)     (( (x) & 0xFF ) << 0 )

/********************************************************************/

#endif /* __MCF5223_PWM_H__ */
