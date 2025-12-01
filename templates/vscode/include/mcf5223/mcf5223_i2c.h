/*
 * File:    mcf5223_i2c.h
 * Purpose: I2C Module register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_I2C_H
#define MCF5223_I2C_H

#include <stdint.h>

/*********************************************************************
 * I2C Module
 * Base Address: IPSBAR + 0x000300
 *********************************************************************/

#define MCF_I2C_BASE            (IPSBAR + 0x000300)

/*********************************************************************
 * Register definitions
 *********************************************************************/

#define MCF_I2C_I2ADR           (*(volatile uint8_t *)(MCF_I2C_BASE + 0x00))
#define MCF_I2C_I2FDR           (*(volatile uint8_t *)(MCF_I2C_BASE + 0x04))
#define MCF_I2C_I2CR            (*(volatile uint8_t *)(MCF_I2C_BASE + 0x08))
#define MCF_I2C_I2SR            (*(volatile uint8_t *)(MCF_I2C_BASE + 0x0C))
#define MCF_I2C_I2DR            (*(volatile uint8_t *)(MCF_I2C_BASE + 0x10))

/*********************************************************************
 * I2ADR bit definitions - I2C Address Register
 *********************************************************************/

#define MCF_I2C_I2ADR_ADR(x)    (((x) & 0x7F) << 1)

/*********************************************************************
 * I2FDR bit definitions - I2C Frequency Divider Register
 *********************************************************************/

#define MCF_I2C_I2FDR_IC(x)     ((x) & 0x3F)

/*********************************************************************
 * I2CR bit definitions - I2C Control Register
 *********************************************************************/

#define MCF_I2C_I2CR_RSTA       (0x04)      /* Repeat start */
#define MCF_I2C_I2CR_TXAK       (0x08)      /* Transmit acknowledge enable */
#define MCF_I2C_I2CR_MTX        (0x10)      /* Master transmit mode */
#define MCF_I2C_I2CR_MSTA       (0x20)      /* Master mode select */
#define MCF_I2C_I2CR_IIEN       (0x40)      /* I2C interrupt enable */
#define MCF_I2C_I2CR_IEN        (0x80)      /* I2C enable */

/*********************************************************************
 * I2SR bit definitions - I2C Status Register
 *********************************************************************/

#define MCF_I2C_I2SR_RXAK       (0x01)      /* Received acknowledge */
#define MCF_I2C_I2SR_IIF        (0x02)      /* I2C interrupt flag */
#define MCF_I2C_I2SR_SRW        (0x04)      /* Slave read/write */
#define MCF_I2C_I2SR_IAL        (0x10)      /* Arbitration lost */
#define MCF_I2C_I2SR_IBB        (0x20)      /* I2C bus busy */
#define MCF_I2C_I2SR_IAAS       (0x40)      /* Addressed as a slave */
#define MCF_I2C_I2SR_ICF        (0x80)      /* Data transfer complete */

/*********************************************************************
 * I2C Frequency Divider values
 *
 * I2C Clock = System Clock / Divider
 *
 * For 60MHz system clock and 100kHz I2C:
 *   Divider = 60MHz / 100kHz = 600
 *   IC = 0x37 (divider = 640, closest match)
 *
 * For 60MHz system clock and 400kHz I2C:
 *   Divider = 60MHz / 400kHz = 150
 *   IC = 0x27 (divider = 160, closest match)
 *********************************************************************/

/* Common I2FDR values for 60MHz system clock */
#define MCF_I2C_I2FDR_100KHZ    (0x37)      /* ~94kHz (divider 640) */
#define MCF_I2C_I2FDR_400KHZ    (0x27)      /* ~375kHz (divider 160) */

#endif /* MCF5223_I2C_H */
