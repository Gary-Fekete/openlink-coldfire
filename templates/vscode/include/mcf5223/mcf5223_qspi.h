/*
 * File:    mcf5223_qspi.h
 * Purpose: Queued SPI (QSPI) register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_QSPI_H
#define MCF5223_QSPI_H

#include <stdint.h>

/*********************************************************************
 * Queued SPI (QSPI)
 * Base Address: IPSBAR + 0x000340
 *********************************************************************/

#define MCF_QSPI_BASE           (IPSBAR + 0x000340)

/*********************************************************************
 * Register definitions
 *********************************************************************/

#define MCF_QSPI_QMR            (*(volatile uint16_t *)(MCF_QSPI_BASE + 0x00))
#define MCF_QSPI_QDLYR          (*(volatile uint16_t *)(MCF_QSPI_BASE + 0x04))
#define MCF_QSPI_QWR            (*(volatile uint16_t *)(MCF_QSPI_BASE + 0x08))
#define MCF_QSPI_QIR            (*(volatile uint16_t *)(MCF_QSPI_BASE + 0x0C))
#define MCF_QSPI_QAR            (*(volatile uint16_t *)(MCF_QSPI_BASE + 0x10))
#define MCF_QSPI_QDR            (*(volatile uint16_t *)(MCF_QSPI_BASE + 0x14))

/*********************************************************************
 * QMR bit definitions - QSPI Mode Register
 *********************************************************************/

#define MCF_QSPI_QMR_BAUD(x)    ((x) & 0x00FF)           /* Baud rate */
#define MCF_QSPI_QMR_CPHA       (0x0100)                 /* Clock phase */
#define MCF_QSPI_QMR_CPOL       (0x0200)                 /* Clock polarity */
#define MCF_QSPI_QMR_BITS(x)    (((x) & 0x0F) << 10)     /* Bits per transfer */
#define MCF_QSPI_QMR_DOHIE      (0x4000)                 /* Data output high impedance enable */
#define MCF_QSPI_QMR_MSTR       (0x8000)                 /* Master mode */

/* Bits per transfer values */
#define MCF_QSPI_QMR_BITS_8     (0x0000)    /* 8 bits (when BITS=0) */
#define MCF_QSPI_QMR_BITS_16    (0x0000)    /* 16 bits (when BITS=0) - default */

/*********************************************************************
 * QDLYR bit definitions - QSPI Delay Register
 *********************************************************************/

#define MCF_QSPI_QDLYR_DTL(x)   ((x) & 0x00FF)           /* Delay after transfer */
#define MCF_QSPI_QDLYR_QCD(x)   (((x) & 0x7F) << 8)      /* QSPI clock delay */
#define MCF_QSPI_QDLYR_SPE      (0x8000)                 /* QSPI enable */

/*********************************************************************
 * QWR bit definitions - QSPI Wrap Register
 *********************************************************************/

#define MCF_QSPI_QWR_NEWQP(x)   ((x) & 0x000F)           /* New queue pointer */
#define MCF_QSPI_QWR_CPTQP(x)   (((x) & 0x0F) << 4)      /* Completed queue pointer */
#define MCF_QSPI_QWR_ENDQP(x)   (((x) & 0x0F) << 8)      /* End queue pointer */
#define MCF_QSPI_QWR_CSIV       (0x1000)                 /* Chip select inactive */
#define MCF_QSPI_QWR_WRTO       (0x2000)                 /* Wrap to */
#define MCF_QSPI_QWR_WREN       (0x4000)                 /* Wrap enable */
#define MCF_QSPI_QWR_HALT       (0x8000)                 /* Halt */

/*********************************************************************
 * QIR bit definitions - QSPI Interrupt Register
 *********************************************************************/

#define MCF_QSPI_QIR_SPIF       (0x0001)    /* QSPI finished flag */
#define MCF_QSPI_QIR_ABRT       (0x0004)    /* Abort flag */
#define MCF_QSPI_QIR_WCEF       (0x0008)    /* Write collision error flag */
#define MCF_QSPI_QIR_SPIFE      (0x0100)    /* QSPI finished interrupt enable */
#define MCF_QSPI_QIR_ABRTE      (0x0400)    /* Abort interrupt enable */
#define MCF_QSPI_QIR_WCEFE      (0x0800)    /* Write collision interrupt enable */
#define MCF_QSPI_QIR_ABRTL      (0x1000)    /* Abort lock */
#define MCF_QSPI_QIR_ABRTB      (0x4000)    /* Abort begin */
#define MCF_QSPI_QIR_WCEFB      (0x8000)    /* Write collision error begin */

/*********************************************************************
 * QAR bit definitions - QSPI Address Register
 *********************************************************************/

#define MCF_QSPI_QAR_ADDR(x)    ((x) & 0x003F)

/* Queue RAM addresses */
#define MCF_QSPI_QAR_TX_BASE    (0x00)      /* Transmit RAM base */
#define MCF_QSPI_QAR_RX_BASE    (0x10)      /* Receive RAM base */
#define MCF_QSPI_QAR_CMD_BASE   (0x20)      /* Command RAM base */

/*********************************************************************
 * Command RAM bit definitions (written via QDR when QAR points to CMD)
 *********************************************************************/

#define MCF_QSPI_CMD_CONT       (0x8000)    /* Continuous CS */
#define MCF_QSPI_CMD_BITSE      (0x4000)    /* Bits enable */
#define MCF_QSPI_CMD_DT         (0x2000)    /* Delay after transfer */
#define MCF_QSPI_CMD_DSCK       (0x1000)    /* Delay start of clock */
#define MCF_QSPI_CMD_CS(x)      (((x) & 0x0F) << 8)  /* Chip select */
#define MCF_QSPI_CMD_CS0        (0x0100)
#define MCF_QSPI_CMD_CS1        (0x0200)
#define MCF_QSPI_CMD_CS2        (0x0400)
#define MCF_QSPI_CMD_CS3        (0x0800)

/*********************************************************************
 * Baud rate calculation
 *
 * QSPI Clock = System Clock / (2 * BAUD)
 *
 * For 60MHz system clock and 1MHz SPI:
 *   BAUD = 60MHz / (2 * 1MHz) = 30
 *********************************************************************/

#define MCF_QSPI_CALC_BAUD(sysclk, spiclk) ((sysclk) / (2 * (spiclk)))

#endif /* MCF5223_QSPI_H */
