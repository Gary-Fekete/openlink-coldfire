/*
 * File:    mcf5223_gpio.h
 * Purpose: General Purpose I/O (GPIO) register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_GPIO_H
#define MCF5223_GPIO_H

#include <stdint.h>

/*********************************************************************
 * General Purpose I/O (GPIO)
 * Base Address: IPSBAR + 0x100000
 *********************************************************************/

#define MCF_GPIO_BASE           (IPSBAR + 0x100000)

/*********************************************************************
 * Port Data Registers (read current pin state, write output data)
 *********************************************************************/

#define MCF_GPIO_PORTNQ         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x08))
#define MCF_GPIO_PORTAN         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x0A))
#define MCF_GPIO_PORTAS         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x0B))
#define MCF_GPIO_PORTQS         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x0C))
#define MCF_GPIO_PORTTA         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x0E))
#define MCF_GPIO_PORTTC         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x0F))
#define MCF_GPIO_PORTTD         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x10))
#define MCF_GPIO_PORTUA         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x11))
#define MCF_GPIO_PORTUB         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x12))
#define MCF_GPIO_PORTUC         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x13))
#define MCF_GPIO_PORTDD         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x14))
#define MCF_GPIO_PORTLD         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x15))
#define MCF_GPIO_PORTGP         (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x16))

/*********************************************************************
 * Data Direction Registers (0=input, 1=output)
 *********************************************************************/

#define MCF_GPIO_DDRNQ          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x1C))
#define MCF_GPIO_DDRAN          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x1E))
#define MCF_GPIO_DDRAS          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x1F))
#define MCF_GPIO_DDRQS          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x20))
#define MCF_GPIO_DDRTA          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x22))
#define MCF_GPIO_DDRTC          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x23))
#define MCF_GPIO_DDRTD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x24))
#define MCF_GPIO_DDRUA          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x25))
#define MCF_GPIO_DDRUB          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x26))
#define MCF_GPIO_DDRUC          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x27))
#define MCF_GPIO_DDRDD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x28))
#define MCF_GPIO_DDRLD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x29))
#define MCF_GPIO_DDRGP          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x2A))

/*********************************************************************
 * Port Set Registers (write 1 to set corresponding bit)
 *********************************************************************/

#define MCF_GPIO_SETNQ          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x30))
#define MCF_GPIO_SETAN          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x32))
#define MCF_GPIO_SETAS          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x33))
#define MCF_GPIO_SETQS          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x34))
#define MCF_GPIO_SETTA          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x36))
#define MCF_GPIO_SETTC          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x37))
#define MCF_GPIO_SETTD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x38))
#define MCF_GPIO_SETUA          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x39))
#define MCF_GPIO_SETUB          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x3A))
#define MCF_GPIO_SETUC          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x3B))
#define MCF_GPIO_SETDD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x3C))
#define MCF_GPIO_SETLD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x3D))
#define MCF_GPIO_SETGP          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x3E))

/*********************************************************************
 * Port Clear Registers (write 1 to clear corresponding bit)
 *********************************************************************/

#define MCF_GPIO_CLRNQ          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x44))
#define MCF_GPIO_CLRAN          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x46))
#define MCF_GPIO_CLRAS          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x47))
#define MCF_GPIO_CLRQS          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x48))
#define MCF_GPIO_CLRTA          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x4A))
#define MCF_GPIO_CLRTC          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x4B))
#define MCF_GPIO_CLRTD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x4C))
#define MCF_GPIO_CLRUA          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x4D))
#define MCF_GPIO_CLRUB          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x4E))
#define MCF_GPIO_CLRUC          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x4F))
#define MCF_GPIO_CLRDD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x50))
#define MCF_GPIO_CLRLD          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x51))
#define MCF_GPIO_CLRGP          (*(volatile uint8_t *)(MCF_GPIO_BASE + 0x52))

/*********************************************************************
 * Pin Assignment Registers (select GPIO or peripheral function)
 *********************************************************************/

#define MCF_GPIO_PNQPAR         (*(volatile uint16_t *)(MCF_GPIO_BASE + 0x56))
#define MCF_GPIO_PANPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x58))
#define MCF_GPIO_PASPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x59))
#define MCF_GPIO_PQSPAR         (*(volatile uint16_t *)(MCF_GPIO_BASE + 0x5A))
#define MCF_GPIO_PTAPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x5E))
#define MCF_GPIO_PTCPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x5F))
#define MCF_GPIO_PTDPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x60))
#define MCF_GPIO_PUAPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x61))
#define MCF_GPIO_PUBPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x62))
#define MCF_GPIO_PUCPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x63))
#define MCF_GPIO_PDDPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x64))
#define MCF_GPIO_PLDPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x65))
#define MCF_GPIO_PGPPAR         (*(volatile uint8_t  *)(MCF_GPIO_BASE + 0x66))

/*********************************************************************
 * Wired-OR and Drive Strength Registers
 *********************************************************************/

#define MCF_GPIO_PWOR           (*(volatile uint16_t *)(MCF_GPIO_BASE + 0x68))
#define MCF_GPIO_PDSRH          (*(volatile uint16_t *)(MCF_GPIO_BASE + 0x6A))
#define MCF_GPIO_PDSRL          (*(volatile uint32_t *)(MCF_GPIO_BASE + 0x6C))

/*********************************************************************
 * Bit definitions for ports (generic)
 *********************************************************************/

#define MCF_GPIO_PIN0           (0x01)
#define MCF_GPIO_PIN1           (0x02)
#define MCF_GPIO_PIN2           (0x04)
#define MCF_GPIO_PIN3           (0x08)
#define MCF_GPIO_PIN4           (0x10)
#define MCF_GPIO_PIN5           (0x20)
#define MCF_GPIO_PIN6           (0x40)
#define MCF_GPIO_PIN7           (0x80)

/*********************************************************************
 * PNQPAR - Port NQ Pin Assignment (IRQ1-IRQ7)
 *********************************************************************/

#define MCF_GPIO_PNQPAR_IRQ1_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ1_IRQ1       (0x0004)
#define MCF_GPIO_PNQPAR_IRQ1_SYNCA      (0x0008)
#define MCF_GPIO_PNQPAR_IRQ1_PWM1       (0x000C)

#define MCF_GPIO_PNQPAR_IRQ2_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ2_IRQ2       (0x0010)

#define MCF_GPIO_PNQPAR_IRQ3_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ3_IRQ3       (0x0040)

#define MCF_GPIO_PNQPAR_IRQ4_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ4_IRQ4       (0x0100)

#define MCF_GPIO_PNQPAR_IRQ5_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ5_IRQ5       (0x0400)

#define MCF_GPIO_PNQPAR_IRQ6_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ6_IRQ6       (0x1000)

#define MCF_GPIO_PNQPAR_IRQ7_GPIO       (0x0000)
#define MCF_GPIO_PNQPAR_IRQ7_IRQ7       (0x4000)

/*********************************************************************
 * PUAPAR - Port UA Pin Assignment (UART0)
 *********************************************************************/

#define MCF_GPIO_PUAPAR_TXD0_GPIO       (0x00)
#define MCF_GPIO_PUAPAR_TXD0_TXD0       (0x01)
#define MCF_GPIO_PUAPAR_RXD0_GPIO       (0x00)
#define MCF_GPIO_PUAPAR_RXD0_RXD0       (0x04)
#define MCF_GPIO_PUAPAR_RTS0_GPIO       (0x00)
#define MCF_GPIO_PUAPAR_RTS0_RTS0       (0x10)
#define MCF_GPIO_PUAPAR_RTS0_CANTX      (0x20)
#define MCF_GPIO_PUAPAR_CTS0_GPIO       (0x00)
#define MCF_GPIO_PUAPAR_CTS0_CTS0       (0x40)
#define MCF_GPIO_PUAPAR_CTS0_CANRX      (0x80)

/*********************************************************************
 * PUBPAR - Port UB Pin Assignment (UART1)
 *********************************************************************/

#define MCF_GPIO_PUBPAR_TXD1_GPIO       (0x00)
#define MCF_GPIO_PUBPAR_TXD1_TXD1       (0x01)
#define MCF_GPIO_PUBPAR_RXD1_GPIO       (0x00)
#define MCF_GPIO_PUBPAR_RXD1_RXD1       (0x04)
#define MCF_GPIO_PUBPAR_RTS1_GPIO       (0x00)
#define MCF_GPIO_PUBPAR_RTS1_RTS1       (0x10)
#define MCF_GPIO_PUBPAR_RTS1_SYNCB      (0x20)
#define MCF_GPIO_PUBPAR_RTS1_TXD2       (0x30)
#define MCF_GPIO_PUBPAR_CTS1_GPIO       (0x00)
#define MCF_GPIO_PUBPAR_CTS1_CTS1       (0x40)
#define MCF_GPIO_PUBPAR_CTS1_SYNCA      (0x80)
#define MCF_GPIO_PUBPAR_CTS1_RXD2       (0xC0)

/*********************************************************************
 * PUCPAR - Port UC Pin Assignment (UART2)
 *********************************************************************/

#define MCF_GPIO_PUCPAR_TXD2_GPIO       (0x00)
#define MCF_GPIO_PUCPAR_TXD2_TXD2       (0x01)
#define MCF_GPIO_PUCPAR_RXD2_GPIO       (0x00)
#define MCF_GPIO_PUCPAR_RXD2_RXD2       (0x02)
#define MCF_GPIO_PUCPAR_RTS2_GPIO       (0x00)
#define MCF_GPIO_PUCPAR_RTS2_RTS2       (0x04)
#define MCF_GPIO_PUCPAR_CTS2_GPIO       (0x00)
#define MCF_GPIO_PUCPAR_CTS2_CTS2       (0x08)

/*********************************************************************
 * PTCPAR - Port TC Pin Assignment (Timers/PWM - M52233DEMO LEDs)
 *********************************************************************/

#define MCF_GPIO_PTCPAR_TIN0_GPIO       (0x00)
#define MCF_GPIO_PTCPAR_TIN0_TIN0       (0x01)
#define MCF_GPIO_PTCPAR_TIN0_TOUT0      (0x02)
#define MCF_GPIO_PTCPAR_TIN0_PWM0       (0x03)

#define MCF_GPIO_PTCPAR_TIN1_GPIO       (0x00)
#define MCF_GPIO_PTCPAR_TIN1_TIN1       (0x04)
#define MCF_GPIO_PTCPAR_TIN1_TOUT1      (0x08)
#define MCF_GPIO_PTCPAR_TIN1_PWM2       (0x0C)

#define MCF_GPIO_PTCPAR_TIN2_GPIO       (0x00)
#define MCF_GPIO_PTCPAR_TIN2_TIN2       (0x10)
#define MCF_GPIO_PTCPAR_TIN2_TOUT2      (0x20)
#define MCF_GPIO_PTCPAR_TIN2_PWM4       (0x30)

#define MCF_GPIO_PTCPAR_TIN3_GPIO       (0x00)
#define MCF_GPIO_PTCPAR_TIN3_TIN3       (0x40)
#define MCF_GPIO_PTCPAR_TIN3_TOUT3      (0x80)
#define MCF_GPIO_PTCPAR_TIN3_PWM6       (0xC0)

/*********************************************************************
 * PLDPAR - Port LD Pin Assignment (FEC LEDs)
 *********************************************************************/

#define MCF_GPIO_PLDPAR_ACTLED_GPIO     (0x00)
#define MCF_GPIO_PLDPAR_ACTLED_ACTLED   (0x01)
#define MCF_GPIO_PLDPAR_LNKLED_GPIO     (0x00)
#define MCF_GPIO_PLDPAR_LNKLED_LNKLED   (0x02)
#define MCF_GPIO_PLDPAR_SPDLED_GPIO     (0x00)
#define MCF_GPIO_PLDPAR_SPDLED_SPDLED   (0x04)
#define MCF_GPIO_PLDPAR_DUPLED_GPIO     (0x00)
#define MCF_GPIO_PLDPAR_DUPLED_DUPLED   (0x08)
#define MCF_GPIO_PLDPAR_COLLED_GPIO     (0x00)
#define MCF_GPIO_PLDPAR_COLLED_COLLED   (0x10)
#define MCF_GPIO_PLDPAR_RXLED_GPIO      (0x00)
#define MCF_GPIO_PLDPAR_RXLED_RXLED     (0x20)
#define MCF_GPIO_PLDPAR_TXLED_GPIO      (0x00)
#define MCF_GPIO_PLDPAR_TXLED_TXLED     (0x40)

#endif /* MCF5223_GPIO_H */
