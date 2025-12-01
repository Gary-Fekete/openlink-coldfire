/*
 * MCF52235 Register Definitions
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This is the main header for MCF52233/MCF52235 development.
 * It provides both simple direct register access (backward compatible)
 * and comprehensive peripheral headers via the mcf5223/ directory.
 *
 * Usage Options:
 *   1. Simple: Just include this file for basic register access
 *      #include "mcf52235.h"
 *
 *   2. Full:   Define MCF5223_FULL_HEADERS before including
 *      #define MCF5223_FULL_HEADERS
 *      #include "mcf52235.h"
 *
 *   3. Direct: Include specific peripheral headers
 *      #include "mcf5223/mcf5223_uart.h"
 *
 * Full definitions available in Freescale/NXP MCF52235RM.pdf
 */

#ifndef MCF52235_H
#define MCF52235_H

#include <stdint.h>

/* Include common ColdFire definitions */
#include "mcf5xxx.h"

/*********************************************************************
 * Base addresses
 *********************************************************************/

#define IPSBAR              0x40000000

/*********************************************************************
 * Full peripheral headers (optional)
 * Define MCF5223_FULL_HEADERS to include all comprehensive headers
 *********************************************************************/

#ifdef MCF5223_FULL_HEADERS
#include "mcf5223/mcf5223.h"
#else

/*********************************************************************
 * Simple register definitions for quick development
 * These provide direct access to commonly used registers
 *********************************************************************/

/* System Control Module (SCM) */
#define SCM_BASE            (IPSBAR + 0x000000)
#define SCM_IPSBAR          (*(volatile uint32_t *)(SCM_BASE + 0x00))
#define SCM_RAMBAR          (*(volatile uint32_t *)(SCM_BASE + 0x08))

/* Clock Module */
#define CLOCK_BASE          (IPSBAR + 0x120000)
#define SYNCR               (*(volatile uint16_t *)(CLOCK_BASE + 0x00))
#define SYNSR               (*(volatile uint8_t  *)(CLOCK_BASE + 0x02))
#define ROCR                (*(volatile uint16_t *)(CLOCK_BASE + 0x04))
#define CCHR                (*(volatile uint8_t  *)(CLOCK_BASE + 0x08))

/* GPIO Base */
#define GPIO_BASE           (IPSBAR + 0x100000)

/* GPIO - Port TC (M52233DEMO LEDs) */
#define PORTTC              (*(volatile uint8_t *)(GPIO_BASE + 0x0F))
#define DDRTC               (*(volatile uint8_t *)(GPIO_BASE + 0x23))
#define SETTC               (*(volatile uint8_t *)(GPIO_BASE + 0x37))
#define CLRTC               (*(volatile uint8_t *)(GPIO_BASE + 0x4B))

/* GPIO - Port TD */
#define PORTTD              (*(volatile uint8_t *)(GPIO_BASE + 0x10))
#define DDRTD               (*(volatile uint8_t *)(GPIO_BASE + 0x24))
#define SETTD               (*(volatile uint8_t *)(GPIO_BASE + 0x38))
#define CLRTD               (*(volatile uint8_t *)(GPIO_BASE + 0x4C))

/* GPIO Pin Assignment Registers */
#define PTCPAR              (*(volatile uint8_t *)(GPIO_BASE + 0x5F))
#define PTDPAR              (*(volatile uint8_t *)(GPIO_BASE + 0x60))

/* Programmable Interrupt Timer (PIT0) */
#define PIT0_BASE           (IPSBAR + 0x150000)
#define PIT0_PCSR           (*(volatile uint16_t *)(PIT0_BASE + 0x00))
#define PIT0_PMR            (*(volatile uint16_t *)(PIT0_BASE + 0x02))
#define PIT0_PCNTR          (*(volatile uint16_t *)(PIT0_BASE + 0x04))

/* UART0 */
#define UART0_BASE          (IPSBAR + 0x000200)
#define UART0_UMR           (*(volatile uint8_t *)(UART0_BASE + 0x00))
#define UART0_USR           (*(volatile uint8_t *)(UART0_BASE + 0x04))
#define UART0_UCSR          (*(volatile uint8_t *)(UART0_BASE + 0x04))
#define UART0_UCR           (*(volatile uint8_t *)(UART0_BASE + 0x08))
#define UART0_URB           (*(volatile uint8_t *)(UART0_BASE + 0x0C))
#define UART0_UTB           (*(volatile uint8_t *)(UART0_BASE + 0x0C))
#define UART0_UIPCR         (*(volatile uint8_t *)(UART0_BASE + 0x10))
#define UART0_UACR          (*(volatile uint8_t *)(UART0_BASE + 0x10))
#define UART0_UISR          (*(volatile uint8_t *)(UART0_BASE + 0x14))
#define UART0_UIMR          (*(volatile uint8_t *)(UART0_BASE + 0x14))
#define UART0_UBG1          (*(volatile uint8_t *)(UART0_BASE + 0x18))
#define UART0_UBG2          (*(volatile uint8_t *)(UART0_BASE + 0x1C))
#define UART0_UIP           (*(volatile uint8_t *)(UART0_BASE + 0x34))
#define UART0_UOP1          (*(volatile uint8_t *)(UART0_BASE + 0x38))
#define UART0_UOP0          (*(volatile uint8_t *)(UART0_BASE + 0x3C))

/* ColdFire Flash Module (CFM) */
#define CFM_BASE            (IPSBAR + 0x1D0000)
#define CFMMCR              (*(volatile uint16_t *)(CFM_BASE + 0x00))
#define CFMCLKD             (*(volatile uint8_t  *)(CFM_BASE + 0x02))
#define CFMSEC              (*(volatile uint32_t *)(CFM_BASE + 0x08))
#define CFMPROT             (*(volatile uint32_t *)(CFM_BASE + 0x10))
#define CFMSACC             (*(volatile uint32_t *)(CFM_BASE + 0x14))
#define CFMDACC             (*(volatile uint32_t *)(CFM_BASE + 0x18))
#define CFMUSTAT            (*(volatile uint8_t  *)(CFM_BASE + 0x20))
#define CFMCMD              (*(volatile uint8_t  *)(CFM_BASE + 0x24))

/* CFMUSTAT bits */
#define CFMUSTAT_CBEIF      0x80
#define CFMUSTAT_CCIF       0x40
#define CFMUSTAT_PVIOL      0x20
#define CFMUSTAT_ACCERR     0x10
#define CFMUSTAT_BLANK      0x04

/* CFM Commands */
#define CFM_CMD_BLANK_CHECK     0x05
#define CFM_CMD_PAGE_ERASE      0x40
#define CFM_CMD_MASS_ERASE      0x41
#define CFM_CMD_PROGRAM         0x20

#endif /* MCF5223_FULL_HEADERS */

/*********************************************************************
 * M52233DEMO Board Support
 * LED definitions (always available)
 *********************************************************************/

/* LEDs are active-low on Port TC */
#define LED1_BIT            0
#define LED2_BIT            1
#define LED3_BIT            2
#define LED4_BIT            3

#define LED1_ON()           (CLRTC = (1 << LED1_BIT))
#define LED1_OFF()          (SETTC = (1 << LED1_BIT))
#define LED1_TOGGLE()       (PORTTC ^= (1 << LED1_BIT))

#define LED2_ON()           (CLRTC = (1 << LED2_BIT))
#define LED2_OFF()          (SETTC = (1 << LED2_BIT))
#define LED2_TOGGLE()       (PORTTC ^= (1 << LED2_BIT))

#define LED3_ON()           (CLRTC = (1 << LED3_BIT))
#define LED3_OFF()          (SETTC = (1 << LED3_BIT))
#define LED3_TOGGLE()       (PORTTC ^= (1 << LED3_BIT))

#define LED4_ON()           (CLRTC = (1 << LED4_BIT))
#define LED4_OFF()          (SETTC = (1 << LED4_BIT))
#define LED4_TOGGLE()       (PORTTC ^= (1 << LED4_BIT))

/* Initialize LEDs */
static inline void leds_init(void) {
    PTCPAR = 0x00;              /* GPIO function */
    DDRTC |= 0x0F;              /* Output */
    SETTC = 0x0F;               /* All LEDs off (active-low) */
}

/* Simple delay loop */
static inline void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ __volatile__ ("nop");
    }
}

#endif /* MCF52235_H */
