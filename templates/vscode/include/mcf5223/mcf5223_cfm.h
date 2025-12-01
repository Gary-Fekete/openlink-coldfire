/*
 * File:    mcf5223_cfm.h
 * Purpose: ColdFire Flash Module (CFM) register definitions for MCF5223
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 *
 * CRITICAL: Register access widths MUST be respected:
 *   - CFMCR:    16-bit access
 *   - CFMCLKD:  8-bit access (WRITE-ONCE after reset!)
 *   - CFMUSTAT: 8-bit access
 *   - CFMCMD:   8-bit access
 *   - CFMPROT, CFMSACC, CFMDACC, CFMSEC: 32-bit access
 */

#ifndef MCF5223_CFM_H
#define MCF5223_CFM_H

#include <stdint.h>

/*********************************************************************
 * ColdFire Flash Module (CFM)
 * Base Address: IPSBAR + 0x1D0000
 *
 * Flash Memory: 256KB (0x00000000 - 0x0003FFFF)
 * Organization: 32 sectors x 8KB each
 *********************************************************************/

#define MCF_CFM_BASE            (IPSBAR + 0x1D0000)

/* Flash memory base address and size */
#define MCF_FLASH_BASE          0x00000000
#define MCF_FLASH_SIZE          0x00040000      /* 256KB */
#define MCF_FLASH_SECTOR_SIZE   0x00002000      /* 8KB */
#define MCF_FLASH_SECTOR_COUNT  32

/*********************************************************************
 * Register definitions with CORRECT access widths
 *********************************************************************/

/* 16-bit registers */
#define MCF_CFM_CFMCR           (*(volatile uint16_t *)(MCF_CFM_BASE + 0x00))

/* 8-bit registers - CRITICAL: use byte access only! */
#define MCF_CFM_CFMCLKD         (*(volatile uint8_t  *)(MCF_CFM_BASE + 0x02))
#define MCF_CFM_CFMUSTAT        (*(volatile uint8_t  *)(MCF_CFM_BASE + 0x20))
#define MCF_CFM_CFMCMD          (*(volatile uint8_t  *)(MCF_CFM_BASE + 0x24))

/* 32-bit registers */
#define MCF_CFM_CFMSEC          (*(volatile uint32_t *)(MCF_CFM_BASE + 0x08))
#define MCF_CFM_CFMPROT         (*(volatile uint32_t *)(MCF_CFM_BASE + 0x10))
#define MCF_CFM_CFMSACC         (*(volatile uint32_t *)(MCF_CFM_BASE + 0x14))
#define MCF_CFM_CFMDACC         (*(volatile uint32_t *)(MCF_CFM_BASE + 0x18))

/* Data registers for programming (32-bit) */
#define MCF_CFM_CFMDATA0        (*(volatile uint32_t *)(MCF_CFM_BASE + 0x28))
#define MCF_CFM_CFMDATA1        (*(volatile uint32_t *)(MCF_CFM_BASE + 0x2C))

/*********************************************************************
 * CFMCR bit definitions - Flash Module Control Register (16-bit)
 *********************************************************************/

#define MCF_CFM_CFMCR_KEYACC    (0x0020)    /* Enable access to backdoor key */
#define MCF_CFM_CFMCR_CCIE      (0x0040)    /* Command Complete Interrupt Enable */
#define MCF_CFM_CFMCR_CBEIE     (0x0080)    /* Command Buffer Empty Interrupt Enable */
#define MCF_CFM_CFMCR_AEIE      (0x0100)    /* Access Error Interrupt Enable */
#define MCF_CFM_CFMCR_PVIE      (0x0200)    /* Protection Violation Interrupt Enable */
#define MCF_CFM_CFMCR_LOCK      (0x0400)    /* Lock CFMPROT, CFMSACC, CFMDACC */

/*********************************************************************
 * CFMCLKD bit definitions - Flash Clock Divider Register (8-bit)
 *
 * IMPORTANT: This register is WRITE-ONCE after reset!
 *            The DIVLD bit indicates if already programmed.
 *
 * Flash clock must be between 150kHz and 200kHz.
 * Formula: FCLK = FSYS / (8 * (DIV + 1))  if PRDIV8=1
 *          FCLK = FSYS / (DIV + 1)        if PRDIV8=0
 *
 * For 60MHz system clock with PRDIV8=1:
 *   DIV = (60MHz / 8 / 200kHz) - 1 = 37 - 1 = 36 = 0x24
 *   CFMCLKD = 0x40 | 0x24 = 0x64
 *********************************************************************/

#define MCF_CFM_CFMCLKD_DIV(x)  (((x) & 0x3F) << 0) /* Clock divider */
#define MCF_CFM_CFMCLKD_PRDIV8  (0x40)              /* Prescale by 8 */
#define MCF_CFM_CFMCLKD_DIVLD   (0x80)              /* Divider loaded (read-only) */

/*********************************************************************
 * CFMSEC bit definitions - Flash Security Register (32-bit)
 *********************************************************************/

#define MCF_CFM_CFMSEC_SEC(x)   (((x) & 0x0000FFFF) << 0)
#define MCF_CFM_CFMSEC_SECSTAT  (0x40000000)    /* Security status (read-only) */
#define MCF_CFM_CFMSEC_KEYEN    (0x80000000)    /* Backdoor key enable */

#define MCF_CFM_CFMSEC_SEC_UNSECURED    0xFFFF  /* Device unsecured */

/*********************************************************************
 * CFMPROT bit definitions - Flash Protection Register (32-bit)
 *
 * Each bit protects one 8KB sector (bit 0 = sector 0, etc.)
 * 0 = sector protected, 1 = sector unprotected
 *********************************************************************/

#define MCF_CFM_CFMPROT_PROTECT(x)  (~(1 << (x)))   /* Protect sector x */
#define MCF_CFM_CFMPROT_ALL_UNPROTECTED 0xFFFFFFFF  /* All sectors unprotected */
#define MCF_CFM_CFMPROT_ALL_PROTECTED   0x00000000  /* All sectors protected */

/*********************************************************************
 * CFMSACC bit definitions - Supervisor Access Register (32-bit)
 * CFMDACC bit definitions - Data Access Register (32-bit)
 *
 * Each bit controls access for one sector
 *********************************************************************/

#define MCF_CFM_CFMSACC_ALL     0xFFFFFFFF  /* Supervisor access to all */
#define MCF_CFM_CFMDACC_ALL     0xFFFFFFFF  /* Data access to all */

/*********************************************************************
 * CFMUSTAT bit definitions - Flash User Status Register (8-bit)
 *
 * This register must be read with 8-bit access!
 *********************************************************************/

#define MCF_CFM_CFMUSTAT_BLANK  (0x04)  /* Blank check result (1=blank) */
#define MCF_CFM_CFMUSTAT_ACCERR (0x10)  /* Access error (write 1 to clear) */
#define MCF_CFM_CFMUSTAT_PVIOL  (0x20)  /* Protection violation (write 1 to clear) */
#define MCF_CFM_CFMUSTAT_CCIF   (0x40)  /* Command Complete Interrupt Flag */
#define MCF_CFM_CFMUSTAT_CBEIF  (0x80)  /* Command Buffer Empty Interrupt Flag */

/* Commonly used status masks */
#define MCF_CFM_CFMUSTAT_ERRORS (MCF_CFM_CFMUSTAT_ACCERR | MCF_CFM_CFMUSTAT_PVIOL)
#define MCF_CFM_CFMUSTAT_READY  (MCF_CFM_CFMUSTAT_CBEIF | MCF_CFM_CFMUSTAT_CCIF)

/*********************************************************************
 * CFMCMD bit definitions - Flash Command Register (8-bit)
 *
 * This register must be written with 8-bit access!
 *********************************************************************/

#define MCF_CFM_CMD_BLANK_CHECK     (0x05)  /* Blank check */
#define MCF_CFM_CMD_PAGE_ERASE_VER  (0x06)  /* Page erase verify */
#define MCF_CFM_CMD_PROGRAM         (0x20)  /* Program word (4 bytes) */
#define MCF_CFM_CMD_PAGE_ERASE      (0x40)  /* Erase sector (page) */
#define MCF_CFM_CMD_MASS_ERASE      (0x41)  /* Mass erase (all sectors) */

/*********************************************************************
 * Flash programming sequence
 *
 * 1. Wait for CBEIF=1 (command buffer empty)
 * 2. Write to flash address (dummy write for erase, data for program)
 * 3. Write command to CFMCMD
 * 4. Write 0x80 to CFMUSTAT to clear CBEIF and launch command
 * 5. Wait for CCIF=1 (command complete)
 * 6. Check for errors (ACCERR, PVIOL)
 *
 * Mass erase takes 10-40 seconds!
 *********************************************************************/

/* Launch command by writing to CFMUSTAT */
#define MCF_CFM_LAUNCH_CMD      (MCF_CFM_CFMUSTAT_CBEIF)

/*********************************************************************
 * Helper macros for sector operations
 *********************************************************************/

/* Get sector number from address */
#define MCF_FLASH_ADDR_TO_SECTOR(addr)  (((addr) & 0x0003FFFF) >> 13)

/* Get sector base address */
#define MCF_FLASH_SECTOR_ADDR(n)        ((n) * MCF_FLASH_SECTOR_SIZE)

/* Check if address is in flash */
#define MCF_FLASH_ADDR_VALID(addr)      ((addr) < MCF_FLASH_SIZE)

#endif /* MCF5223_CFM_H */
