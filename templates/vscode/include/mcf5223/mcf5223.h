/*
 * File:    mcf5223.h
 * Purpose: Master include file for MCF5223 (MCF52233/MCF52235) peripherals
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 *
 * This header includes register definitions for all MCF5223 peripherals.
 * Include this file to get access to all on-chip peripheral registers.
 *
 * Memory Map:
 *   0x00000000 - 0x0003FFFF : Flash (256KB)
 *   0x20000000 - 0x20007FFF : SRAM (32KB)
 *   0x40000000 - 0x401FFFFF : Internal Peripheral System (IPS)
 *
 * Usage:
 *   #include "mcf5223/mcf5223.h"
 *
 *   // Or define IPSBAR and include individual headers:
 *   #define IPSBAR 0x40000000
 *   #include "mcf5223/mcf5223_gpio.h"
 */

#ifndef MCF5223_H
#define MCF5223_H

#include <stdint.h>

/*********************************************************************
 * Memory Map Definitions
 *********************************************************************/

/* Internal Peripheral System Base Address */
#ifndef IPSBAR
#define IPSBAR                  0x40000000
#endif

/* Flash memory */
#define MCF_FLASH_BASE_ADDR     0x00000000
#define MCF_FLASH_END_ADDR      0x0003FFFF
#define MCF_FLASH_SIZE_BYTES    0x00040000      /* 256KB */

/* SRAM memory */
#define MCF_SRAM_BASE_ADDR      0x20000000
#define MCF_SRAM_END_ADDR       0x20007FFF
#define MCF_SRAM_SIZE_BYTES     0x00008000      /* 32KB */

/*********************************************************************
 * Peripheral Module Includes
 *********************************************************************/

/* System Control Module */
#include "mcf5223_scm.h"

/* Clock Module */
#include "mcf5223_clock.h"

/* General Purpose I/O */
#include "mcf5223_gpio.h"

/* Interrupt Controller */
#include "mcf5223_intc.h"

/* UARTs */
#include "mcf5223_uart.h"

/* I2C */
#include "mcf5223_i2c.h"

/* Queued SPI */
#include "mcf5223_qspi.h"

/* Programmable Interrupt Timers */
#include "mcf5223_pit.h"

/* DMA Timers */
#include "mcf5223_dtim.h"

/* ColdFire Flash Module */
#include "mcf5223_cfm.h"

/*********************************************************************
 * CPU-specific definitions
 *********************************************************************/

/* Vector Base Register (VBR) - stores exception vector table address */
#define MCF_VBR_ADDRESS         MCF_FLASH_BASE_ADDR

/* Initial stack pointer (top of SRAM) */
#define MCF_INITIAL_SP          (MCF_SRAM_BASE_ADDR + MCF_SRAM_SIZE_BYTES)

/* Default system clock (assuming 25MHz crystal with PLL) */
#define MCF_SYSTEM_CLOCK_HZ     60000000        /* 60MHz typical */

/*********************************************************************
 * Board-specific definitions (M52233DEMO)
 *********************************************************************/

#ifdef M52233DEMO

/* Crystal frequency on M52233DEMO board */
#define MCF_CRYSTAL_FREQ_HZ     25000000        /* 25MHz */

/* LED port (Port TC on M52233DEMO) */
#define MCF_LED_PORT            MCF_GPIO_PORTTC
#define MCF_LED_DDR             MCF_GPIO_DDRTC
#define MCF_LED_SET             MCF_GPIO_SETTC
#define MCF_LED_CLR             MCF_GPIO_CLRTC
#define MCF_LED_PAR             MCF_GPIO_PTCPAR

/* LED pins (active-low on M52233DEMO) */
#define MCF_LED1_BIT            (1 << 0)
#define MCF_LED2_BIT            (1 << 1)
#define MCF_LED3_BIT            (1 << 2)
#define MCF_LED4_BIT            (1 << 3)
#define MCF_LED_ALL             (0x0F)

/* LED macros (active-low) */
#define MCF_LED1_ON()           (MCF_LED_CLR = MCF_LED1_BIT)
#define MCF_LED1_OFF()          (MCF_LED_SET = MCF_LED1_BIT)
#define MCF_LED1_TOGGLE()       (MCF_LED_PORT ^= MCF_LED1_BIT)

#define MCF_LED2_ON()           (MCF_LED_CLR = MCF_LED2_BIT)
#define MCF_LED2_OFF()          (MCF_LED_SET = MCF_LED2_BIT)
#define MCF_LED2_TOGGLE()       (MCF_LED_PORT ^= MCF_LED2_BIT)

#define MCF_LED3_ON()           (MCF_LED_CLR = MCF_LED3_BIT)
#define MCF_LED3_OFF()          (MCF_LED_SET = MCF_LED3_BIT)
#define MCF_LED3_TOGGLE()       (MCF_LED_PORT ^= MCF_LED3_BIT)

#define MCF_LED4_ON()           (MCF_LED_CLR = MCF_LED4_BIT)
#define MCF_LED4_OFF()          (MCF_LED_SET = MCF_LED4_BIT)
#define MCF_LED4_TOGGLE()       (MCF_LED_PORT ^= MCF_LED4_BIT)

/* Initialize LEDs */
static inline void mcf_leds_init(void)
{
    MCF_LED_PAR = 0x00;             /* GPIO function */
    MCF_LED_DDR |= MCF_LED_ALL;     /* Output */
    MCF_LED_SET = MCF_LED_ALL;      /* All LEDs off (active-low) */
}

#endif /* M52233DEMO */

#endif /* MCF5223_H */
