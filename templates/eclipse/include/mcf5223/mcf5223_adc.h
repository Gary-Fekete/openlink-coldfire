/*
 * File:    mcf5223_adc.h
 * Purpose: Register and bit definitions for Analog-to-Digital Converter (ADC) Module
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 * GPL v3 License
 *
 * Based on MCF52235 Reference Manual, Chapter 28
 *
 * ADC Features:
 *   - 12-bit resolution
 *   - 8 analog input channels (AN0-AN7)
 *   - Two independent converter modules (A and B)
 *   - Sequential or simultaneous sampling
 *   - Programmable sample list (up to 8 samples per scan)
 *   - High/low limit comparison with interrupt
 *   - Zero crossing detection
 *   - Automatic power-down mode
 *
 * Typical Usage:
 *   1. Configure clock divider (CTRL2.DIV)
 *   2. Power up converters (POWER register)
 *   3. Configure sample list (ADLST1/2)
 *   4. Set scan mode (CTRL1.SMODE)
 *   5. Start conversion (CTRL1.START0 or CTRL2.START1)
 *   6. Poll ADSTAT.RDYx or wait for interrupt
 *   7. Read results from ADRSLTx registers
 */

#ifndef __MCF5223_ADC_H__
#define __MCF5223_ADC_H__

#include <stdint.h>

/*********************************************************************
 *
 * Analog-to-Digital Converter (ADC)
 *
 * The ADC module provides 12-bit analog-to-digital conversion with
 * 8 multiplexed input channels. It supports both sequential and
 * simultaneous conversion modes.
 *
 *********************************************************************/

/* ADC Module Base Address */
#define MCF_ADC_BASE            (IPSBAR + 0x190000)


/*********************************************************************
 * ADC Register Definitions
 *********************************************************************/

/*--------------------------------------------------------------------
 * Control Registers
 *--------------------------------------------------------------------*/

/*
 * MCF_ADC_CTRL1 - Control Register 1 (16-bit)
 * Offset: 0x00
 *
 * Controls converter A operation, scan mode, and interrupts.
 */
#define MCF_ADC_CTRL1            (*(volatile uint16_t *)(MCF_ADC_BASE + 0x00))

/*
 * MCF_ADC_CTRL2 - Control Register 2 (16-bit)
 * Offset: 0x02
 *
 * Controls converter B operation and clock divider.
 * The clock divider applies to both converters.
 */
#define MCF_ADC_CTRL2            (*(volatile uint16_t *)(MCF_ADC_BASE + 0x02))

/*
 * MCF_ADC_ADZCC - Zero Crossing Control Register (16-bit)
 * Offset: 0x04
 *
 * Configures zero crossing detection for each sample slot.
 * Zero crossing can detect when input crosses a reference level.
 */
#define MCF_ADC_ADZCC            (*(volatile uint16_t *)(MCF_ADC_BASE + 0x04))

/*--------------------------------------------------------------------
 * Sample List Registers
 *
 * Define which analog channels are sampled in each slot.
 * Up to 8 samples can be configured per scan sequence.
 *--------------------------------------------------------------------*/

/*
 * MCF_ADC_ADLST1 - Sample List Register 1 (16-bit)
 * Offset: 0x06
 *
 * Configures sample slots 0-3. Each 4-bit field selects AN0-AN7.
 */
#define MCF_ADC_ADLST1           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x06))

/*
 * MCF_ADC_ADLST2 - Sample List Register 2 (16-bit)
 * Offset: 0x08
 *
 * Configures sample slots 4-7. Each 4-bit field selects AN0-AN7.
 */
#define MCF_ADC_ADLST2           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x08))

/*
 * MCF_ADC_ADSDIS - Sample Disable Register (16-bit)
 * Offset: 0x0A
 *
 * Disables individual sample slots. Set bit to skip that slot.
 * Useful for scanning fewer than 8 channels.
 */
#define MCF_ADC_ADSDIS           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x0A))

/*--------------------------------------------------------------------
 * Status Registers
 *--------------------------------------------------------------------*/

/*
 * MCF_ADC_ADSTAT - Status Register (16-bit)
 * Offset: 0x0C
 *
 * Indicates conversion status, ready flags, and interrupt conditions.
 * RDYx bits are set when conversion result is available.
 * Write 1 to clear interrupt flags.
 */
#define MCF_ADC_ADSTAT           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x0C))

/*
 * MCF_ADC_ADLSTAT - Limit Status Register (16-bit)
 * Offset: 0x0E
 *
 * Indicates which samples exceeded high or low limits.
 * LLSx = Low limit exceeded, HLSx = High limit exceeded.
 * Write 1 to clear flags.
 */
#define MCF_ADC_ADLSTAT          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x0E))

/*
 * MCF_ADC_ADZCSTAT - Zero Crossing Status Register (16-bit)
 * Offset: 0x10
 *
 * Indicates which samples detected a zero crossing event.
 * Write 1 to clear flags.
 */
#define MCF_ADC_ADZCSTAT         (*(volatile uint16_t *)(MCF_ADC_BASE + 0x10))

/*--------------------------------------------------------------------
 * Result Registers
 *
 * 12-bit conversion results are stored in bits [14:3].
 * Bits [2:0] are always zero. Bit 15 is sign extension (2's complement).
 *
 * To extract the 12-bit unsigned result:
 *   uint16_t result = (MCF_ADC_ADRSLTx >> 3) & 0x0FFF;
 *
 * Result range: 0x000 (0V) to 0xFFF (VREFH)
 *--------------------------------------------------------------------*/

#define MCF_ADC_ADRSLT0          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x12))
#define MCF_ADC_ADRSLT1          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x14))
#define MCF_ADC_ADRSLT2          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x16))
#define MCF_ADC_ADRSLT3          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x18))
#define MCF_ADC_ADRSLT4          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x1A))
#define MCF_ADC_ADRSLT5          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x1C))
#define MCF_ADC_ADRSLT6          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x1E))
#define MCF_ADC_ADRSLT7          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x20))

/* Indexed access: MCF_ADC_ADRSLT(0) through MCF_ADC_ADRSLT(7) */
#define MCF_ADC_ADRSLT(x)        (*(volatile uint16_t *)(MCF_ADC_BASE + 0x12 + ((x) * 0x02)))

/*--------------------------------------------------------------------
 * Limit Registers
 *
 * Low and high limit registers for automatic comparison.
 * When a result exceeds these limits, an interrupt can be generated.
 * Limit values use the same format as result registers (12-bit in [14:3]).
 *--------------------------------------------------------------------*/

/* Low Limit Registers */
#define MCF_ADC_ADLLMT0          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x22))
#define MCF_ADC_ADLLMT1          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x24))
#define MCF_ADC_ADLLMT2          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x26))
#define MCF_ADC_ADLLMT3          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x28))
#define MCF_ADC_ADLLMT4          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x2A))
#define MCF_ADC_ADLLMT5          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x2C))
#define MCF_ADC_ADLLMT6          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x2E))
#define MCF_ADC_ADLLMT7          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x30))

/* Indexed access: MCF_ADC_ADLLMT(0) through MCF_ADC_ADLLMT(7) */
#define MCF_ADC_ADLLMT(x)        (*(volatile uint16_t *)(MCF_ADC_BASE + 0x22 + ((x) * 0x02)))

/* High Limit Registers */
#define MCF_ADC_ADHLMT0          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x32))
#define MCF_ADC_ADHLMT1          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x34))
#define MCF_ADC_ADHLMT2          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x36))
#define MCF_ADC_ADHLMT3          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x38))
#define MCF_ADC_ADHLMT4          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x3A))
#define MCF_ADC_ADHLMT5          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x3C))
#define MCF_ADC_ADHLMT6          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x3E))
#define MCF_ADC_ADHLMT7          (*(volatile uint16_t *)(MCF_ADC_BASE + 0x40))

/* Indexed access: MCF_ADC_ADHLMT(0) through MCF_ADC_ADHLMT(7) */
#define MCF_ADC_ADHLMT(x)        (*(volatile uint16_t *)(MCF_ADC_BASE + 0x32 + ((x) * 0x02)))

/*--------------------------------------------------------------------
 * Offset Registers
 *
 * Calibration offset for each sample slot.
 * The offset is subtracted from the raw conversion result.
 * Use for correcting DC offset errors in the analog input path.
 *--------------------------------------------------------------------*/

#define MCF_ADC_ADOFS0           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x42))
#define MCF_ADC_ADOFS1           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x44))
#define MCF_ADC_ADOFS2           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x46))
#define MCF_ADC_ADOFS3           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x48))
#define MCF_ADC_ADOFS4           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x4A))
#define MCF_ADC_ADOFS5           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x4C))
#define MCF_ADC_ADOFS6           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x4E))
#define MCF_ADC_ADOFS7           (*(volatile uint16_t *)(MCF_ADC_BASE + 0x50))

/* Indexed access: MCF_ADC_ADOFS(0) through MCF_ADC_ADOFS(7) */
#define MCF_ADC_ADOFS(x)         (*(volatile uint16_t *)(MCF_ADC_BASE + 0x42 + ((x) * 0x02)))

/*--------------------------------------------------------------------
 * Power and Calibration Registers
 *--------------------------------------------------------------------*/

/*
 * MCF_ADC_POWER - Power Control Register (16-bit)
 * Offset: 0x52
 *
 * Controls power-down modes and power-up delay.
 * Each converter (A, B, and reference) can be powered down independently.
 */
#define MCF_ADC_POWER            (*(volatile uint16_t *)(MCF_ADC_BASE + 0x52))

/*
 * MCF_ADC_CAL - Calibration Register (16-bit)
 * Offset: 0x54
 *
 * Controls self-calibration for each converter.
 * Calibration compensates for internal offset and gain errors.
 */
#define MCF_ADC_CAL              (*(volatile uint16_t *)(MCF_ADC_BASE + 0x54))


/*********************************************************************
 * CTRL1 - Control Register 1 Bit Definitions
 *********************************************************************/

/*
 * SMODE - Scan Mode Select (bits 2:0)
 *
 * Defines how conversions are triggered and sequenced.
 *
 * Values:
 *   0 = Once sequential      - Single scan, one sample at a time
 *   1 = Once parallel        - Single scan, simultaneous A and B
 *   2 = Loop sequential      - Continuous scan, one sample at a time
 *   3 = Loop parallel        - Continuous scan, simultaneous A and B
 *   4 = Triggered sequential - External trigger, one sample at a time
 *   5 = Triggered parallel   - External trigger, simultaneous A and B
 *   6 = Reserved
 *   7 = Reserved
 */
#define MCF_ADC_CTRL1_SMODE(x)     (((x) & 0x0007) << 0)

/* Scan mode convenience definitions */
#define MCF_ADC_CTRL1_SMODE_ONCE_SEQ       MCF_ADC_CTRL1_SMODE(0)
#define MCF_ADC_CTRL1_SMODE_ONCE_PAR       MCF_ADC_CTRL1_SMODE(1)
#define MCF_ADC_CTRL1_SMODE_LOOP_SEQ       MCF_ADC_CTRL1_SMODE(2)
#define MCF_ADC_CTRL1_SMODE_LOOP_PAR       MCF_ADC_CTRL1_SMODE(3)
#define MCF_ADC_CTRL1_SMODE_TRIG_SEQ       MCF_ADC_CTRL1_SMODE(4)
#define MCF_ADC_CTRL1_SMODE_TRIG_PAR       MCF_ADC_CTRL1_SMODE(5)

/*
 * CHNCFG - Channel Configuration (bits 7:4)
 *
 * Configures how the 8 analog inputs are used.
 * Can be configured for single-ended or differential inputs.
 *
 * Bit 7: AN6/AN7 pair (1 = differential, 0 = single-ended)
 * Bit 6: AN4/AN5 pair
 * Bit 5: AN2/AN3 pair
 * Bit 4: AN0/AN1 pair
 */
#define MCF_ADC_CTRL1_CHNCFG(x)    (((x) & 0x000F) << 4)

/* Common channel configurations */
#define MCF_ADC_CTRL1_CHNCFG_ALL_SE   MCF_ADC_CTRL1_CHNCFG(0x0)  /* All single-ended */
#define MCF_ADC_CTRL1_CHNCFG_ALL_DIFF MCF_ADC_CTRL1_CHNCFG(0xF)  /* All differential */

/*
 * Interrupt Enable bits
 */
#define MCF_ADC_CTRL1_HLMTIE       (0x0100)  /* High Limit Interrupt Enable */
#define MCF_ADC_CTRL1_LLMTIE       (0x0200)  /* Low Limit Interrupt Enable */
#define MCF_ADC_CTRL1_ZCIE         (0x0400)  /* Zero Crossing Interrupt Enable */
#define MCF_ADC_CTRL1_EOSIE0       (0x0800)  /* End of Scan Interrupt Enable (Conv A) */

/*
 * Converter A Control bits
 */
#define MCF_ADC_CTRL1_SYNC0        (0x1000)  /* Sync input enabled for Conv A */
#define MCF_ADC_CTRL1_START0       (0x2000)  /* Start conversion on Conv A */
#define MCF_ADC_CTRL1_STOP0        (0x4000)  /* Stop conversion on Conv A */


/*********************************************************************
 * CTRL2 - Control Register 2 Bit Definitions
 *********************************************************************/

/*
 * DIV - Clock Divider (bits 4:0)
 *
 * ADC clock = System clock / (DIV + 1)
 *
 * The ADC requires a clock between 100 kHz and 5 MHz for proper operation.
 * For a 60 MHz system clock:
 *   DIV = 11 gives 60/(11+1) = 5 MHz (fastest)
 *   DIV = 31 gives 60/(31+1) = 1.875 MHz
 *
 * Conversion time = 8.5 ADC clocks per sample + 6 clocks setup
 */
#define MCF_ADC_CTRL2_DIV(x)       (((x) & 0x001F) << 0)

/*
 * SIMULT - Simultaneous Mode (bit 5)
 *
 * When set, converters A and B sample simultaneously.
 * Sample slots 0-3 use converter A, slots 4-7 use converter B.
 * Useful for sampling related signals at exactly the same time.
 */
#define MCF_ADC_CTRL2_SIMULT       (0x0020)

/*
 * Converter B Control bits (same function as CTRL1 for converter A)
 */
#define MCF_ADC_CTRL2_EOSIE1       (0x0800)  /* End of Scan Interrupt Enable (Conv B) */
#define MCF_ADC_CTRL2_SYNC1        (0x1000)  /* Sync input enabled for Conv B */
#define MCF_ADC_CTRL2_START1       (0x2000)  /* Start conversion on Conv B */
#define MCF_ADC_CTRL2_STOP1        (0x4000)  /* Stop conversion on Conv B */


/*********************************************************************
 * ADZCC - Zero Crossing Control Bit Definitions
 *
 * Each sample slot has a 2-bit field to configure zero crossing mode.
 * Zero crossing detects when the input crosses a mid-scale reference.
 *
 * Values for each ZCE field:
 *   0 = Zero crossing disabled
 *   1 = Detect positive-to-negative crossing
 *   2 = Detect negative-to-positive crossing
 *   3 = Detect any crossing
 *********************************************************************/

#define MCF_ADC_ADZCC_ZCE0(x)      (((x) & 0x0003) << 0)   /* Sample 0 */
#define MCF_ADC_ADZCC_ZCE1(x)      (((x) & 0x0003) << 2)   /* Sample 1 */
#define MCF_ADC_ADZCC_ZCE2(x)      (((x) & 0x0003) << 4)   /* Sample 2 */
#define MCF_ADC_ADZCC_ZCE3(x)      (((x) & 0x0003) << 6)   /* Sample 3 */
#define MCF_ADC_ADZCC_ZCE4(x)      (((x) & 0x0003) << 8)   /* Sample 4 */
#define MCF_ADC_ADZCC_ZCE5(x)      (((x) & 0x0003) << 10)  /* Sample 5 */
#define MCF_ADC_ADZCC_ZCE6(x)      (((x) & 0x0003) << 12)  /* Sample 6 */
#define MCF_ADC_ADZCC_ZCE7(x)      (((x) & 0x0003) << 14)  /* Sample 7 */

/* Zero crossing mode values */
#define MCF_ADC_ZCE_DISABLED       0  /* Zero crossing detection disabled */
#define MCF_ADC_ZCE_POS_TO_NEG     1  /* Positive to negative crossing */
#define MCF_ADC_ZCE_NEG_TO_POS     2  /* Negative to positive crossing */
#define MCF_ADC_ZCE_ANY            3  /* Any crossing */


/*********************************************************************
 * ADLST1/ADLST2 - Sample List Register Bit Definitions
 *
 * Each SAMPLE field selects which analog input (AN0-AN7) is
 * converted for that sample slot.
 *********************************************************************/

/* ADLST1 - Samples 0-3 */
#define MCF_ADC_ADLST1_SAMPLE0(x)  (((x) & 0x0007) << 0)   /* AN0-AN7 for slot 0 */
#define MCF_ADC_ADLST1_SAMPLE1(x)  (((x) & 0x0007) << 4)   /* AN0-AN7 for slot 1 */
#define MCF_ADC_ADLST1_SAMPLE2(x)  (((x) & 0x0007) << 8)   /* AN0-AN7 for slot 2 */
#define MCF_ADC_ADLST1_SAMPLE3(x)  (((x) & 0x0007) << 12)  /* AN0-AN7 for slot 3 */

/* ADLST2 - Samples 4-7 */
#define MCF_ADC_ADLST2_SAMPLE4(x)  (((x) & 0x0007) << 0)   /* AN0-AN7 for slot 4 */
#define MCF_ADC_ADLST2_SAMPLE5(x)  (((x) & 0x0007) << 4)   /* AN0-AN7 for slot 5 */
#define MCF_ADC_ADLST2_SAMPLE6(x)  (((x) & 0x0007) << 8)   /* AN0-AN7 for slot 6 */
#define MCF_ADC_ADLST2_SAMPLE7(x)  (((x) & 0x0007) << 12)  /* AN0-AN7 for slot 7 */


/*********************************************************************
 * ADSDIS - Sample Disable Register Bit Definitions
 *
 * Set bit to disable (skip) that sample slot during a scan.
 * Useful when scanning fewer than 8 channels.
 *********************************************************************/

#define MCF_ADC_ADSDIS_DS0         (0x0001)  /* Disable sample slot 0 */
#define MCF_ADC_ADSDIS_DS1         (0x0002)  /* Disable sample slot 1 */
#define MCF_ADC_ADSDIS_DS2         (0x0004)  /* Disable sample slot 2 */
#define MCF_ADC_ADSDIS_DS3         (0x0008)  /* Disable sample slot 3 */
#define MCF_ADC_ADSDIS_DS4         (0x0010)  /* Disable sample slot 4 */
#define MCF_ADC_ADSDIS_DS5         (0x0020)  /* Disable sample slot 5 */
#define MCF_ADC_ADSDIS_DS6         (0x0040)  /* Disable sample slot 6 */
#define MCF_ADC_ADSDIS_DS7         (0x0080)  /* Disable sample slot 7 */


/*********************************************************************
 * ADSTAT - Status Register Bit Definitions
 *********************************************************************/

/* Ready flags - set when conversion result is available */
#define MCF_ADC_ADSTAT_RDY0        (0x0001)  /* Sample 0 ready */
#define MCF_ADC_ADSTAT_RDY1        (0x0002)  /* Sample 1 ready */
#define MCF_ADC_ADSTAT_RDY2        (0x0004)  /* Sample 2 ready */
#define MCF_ADC_ADSTAT_RDY3        (0x0008)  /* Sample 3 ready */
#define MCF_ADC_ADSTAT_RDY4        (0x0010)  /* Sample 4 ready */
#define MCF_ADC_ADSTAT_RDY5        (0x0020)  /* Sample 5 ready */
#define MCF_ADC_ADSTAT_RDY6        (0x0040)  /* Sample 6 ready */
#define MCF_ADC_ADSTAT_RDY7        (0x0080)  /* Sample 7 ready */

/* Interrupt status flags - write 1 to clear */
#define MCF_ADC_ADSTAT_HLMT        (0x0100)  /* High limit exceeded */
#define MCF_ADC_ADSTAT_LLMTI       (0x0200)  /* Low limit exceeded */
#define MCF_ADC_ADSTAT_ZCI         (0x0400)  /* Zero crossing detected */
#define MCF_ADC_ADSTAT_EOSI        (0x0800)  /* End of scan (either converter) */

/* Conversion in progress - read only */
#define MCF_ADC_ADSTAT_CIP         (0x8000)  /* Conversion in progress */


/*********************************************************************
 * ADLSTAT - Limit Status Register Bit Definitions
 *
 * Indicates which samples exceeded their limit thresholds.
 * Write 1 to clear flags.
 *********************************************************************/

/* Low Limit Status - result < ADLLMT */
#define MCF_ADC_ADLSTAT_LLS0       (0x0001)
#define MCF_ADC_ADLSTAT_LLS1       (0x0002)
#define MCF_ADC_ADLSTAT_LLS2       (0x0004)
#define MCF_ADC_ADLSTAT_LLS3       (0x0008)
#define MCF_ADC_ADLSTAT_LLS4       (0x0010)
#define MCF_ADC_ADLSTAT_LLS5       (0x0020)
#define MCF_ADC_ADLSTAT_LLS6       (0x0040)
#define MCF_ADC_ADLSTAT_LLS7       (0x0080)

/* High Limit Status - result > ADHLMT */
#define MCF_ADC_ADLSTAT_HLS0       (0x0100)
#define MCF_ADC_ADLSTAT_HLS1       (0x0200)
#define MCF_ADC_ADLSTAT_HLS2       (0x0400)
#define MCF_ADC_ADLSTAT_HLS3       (0x0800)
#define MCF_ADC_ADLSTAT_HLS4       (0x1000)
#define MCF_ADC_ADLSTAT_HLS5       (0x2000)
#define MCF_ADC_ADLSTAT_HLS6       (0x4000)
#define MCF_ADC_ADLSTAT_HLS7       (0x8000)


/*********************************************************************
 * ADZCSTAT - Zero Crossing Status Register Bit Definitions
 *
 * Indicates which samples detected a zero crossing.
 * Write 1 to clear flags.
 *********************************************************************/

#define MCF_ADC_ADZCSTAT_ZCS0      (0x0001)
#define MCF_ADC_ADZCSTAT_ZCS1      (0x0002)
#define MCF_ADC_ADZCSTAT_ZCS2      (0x0004)
#define MCF_ADC_ADZCSTAT_ZCS3      (0x0008)
#define MCF_ADC_ADZCSTAT_ZCS4      (0x0010)
#define MCF_ADC_ADZCSTAT_ZCS5      (0x0020)
#define MCF_ADC_ADZCSTAT_ZCS6      (0x0040)
#define MCF_ADC_ADZCSTAT_ZCS7      (0x0080)


/*********************************************************************
 * ADRSLT - Result Register Bit Definitions
 *
 * The 12-bit result is stored in bits [14:3].
 * Bit 15 (SEXT) provides sign extension for differential mode.
 *********************************************************************/

/*
 * RSLT - Result value macro (for writing limit/offset registers)
 *
 * Converts a 12-bit value to register format (shifted left by 3).
 */
#define MCF_ADC_ADRSLT_RSLT(x)     (((x) & 0x0FFF) << 3)

/*
 * SEXT - Sign Extension (bit 15)
 *
 * In differential mode, this bit extends the sign for 2's complement.
 */
#define MCF_ADC_ADRSLT_SEXT        (0x8000)

/*
 * Helper macro to extract 12-bit unsigned result from register
 *
 * Usage: uint16_t value = MCF_ADC_GET_RESULT(MCF_ADC_ADRSLT0);
 */
#define MCF_ADC_GET_RESULT(reg)    (((reg) >> 3) & 0x0FFF)


/*********************************************************************
 * ADLLMT/ADHLMT - Limit Register Bit Definitions
 *
 * Format matches result registers (12-bit value in bits [14:3]).
 *********************************************************************/

#define MCF_ADC_ADLLMT_LLMT(x)     (((x) & 0x0FFF) << 3)
#define MCF_ADC_ADHLMT_HLMT(x)     (((x) & 0x0FFF) << 3)


/*********************************************************************
 * ADOFS - Offset Register Bit Definitions
 *
 * Calibration offset subtracted from raw result.
 * Format matches result registers (12-bit value in bits [14:3]).
 *********************************************************************/

#define MCF_ADC_ADOFS_OFFSET(x)    (((x) & 0x0FFF) << 3)


/*********************************************************************
 * POWER - Power Control Register Bit Definitions
 *********************************************************************/

/*
 * Power-down control bits
 * Set bit to power down that component, clear to power up.
 */
#define MCF_ADC_POWER_PD0          (0x0001)  /* Power down converter A */
#define MCF_ADC_POWER_PD1          (0x0002)  /* Power down converter B */
#define MCF_ADC_POWER_PD2          (0x0004)  /* Power down voltage reference */
#define MCF_ADC_POWER_APD          (0x0008)  /* Auto power-down enable */

/*
 * PUDELAY - Power-up Delay (bits 9:4)
 *
 * Number of ADC clock cycles to wait after power-up before conversion.
 * Allows analog circuits to stabilize.
 * Delay = PUDELAY * ADC clock period
 */
#define MCF_ADC_POWER_PUDELAY(x)   (((x) & 0x003F) << 4)

/*
 * Power status bits (read-only)
 * Indicate current power state of each component.
 */
#define MCF_ADC_POWER_PSTS0        (0x0400)  /* Converter A powered up */
#define MCF_ADC_POWER_PSTS1        (0x0800)  /* Converter B powered up */
#define MCF_ADC_POWER_PSTS2        (0x1000)  /* Voltage reference powered up */

/*
 * ASTBY - Auto Standby (bit 15)
 *
 * When set, ADC enters standby mode between scans to save power.
 * Automatically powers up when a new scan is triggered.
 */
#define MCF_ADC_POWER_ASTBY        (0x8000)


/*********************************************************************
 * CAL - Calibration Register Bit Definitions
 *
 * Controls self-calibration for each converter.
 * Write 1 to CALx to start calibration, reads 1 while in progress.
 * CRSx is set when calibration is required (after power-up or reset).
 *********************************************************************/

#define MCF_ADC_CAL_CAL0           (0x0001)  /* Converter A calibration */
#define MCF_ADC_CAL_CRS0           (0x0002)  /* Converter A calibration required */
#define MCF_ADC_CAL_CAL1           (0x0004)  /* Converter B calibration */
#define MCF_ADC_CAL_CRS1           (0x0008)  /* Converter B calibration required */


/*********************************************************************
 * Usage Examples
 *********************************************************************/

/*
 * Example 1: Simple single-channel conversion
 *
 *   // Power up ADC
 *   MCF_ADC_POWER = 0;  // Clear all power-down bits
 *
 *   // Set clock divider for 5 MHz ADC clock (assuming 60 MHz system)
 *   MCF_ADC_CTRL2 = MCF_ADC_CTRL2_DIV(11);
 *
 *   // Configure sample list: only slot 0, reading AN0
 *   MCF_ADC_ADLST1 = MCF_ADC_ADLST1_SAMPLE0(0);  // AN0 in slot 0
 *   MCF_ADC_ADSDIS = 0xFE;  // Disable slots 1-7
 *
 *   // Single scan, sequential mode
 *   MCF_ADC_CTRL1 = MCF_ADC_CTRL1_SMODE_ONCE_SEQ | MCF_ADC_CTRL1_START0;
 *
 *   // Wait for conversion complete
 *   while (!(MCF_ADC_ADSTAT & MCF_ADC_ADSTAT_RDY0));
 *
 *   // Read result (12-bit value)
 *   uint16_t result = MCF_ADC_GET_RESULT(MCF_ADC_ADRSLT0);
 *
 *
 * Example 2: Continuous multi-channel scan with interrupt
 *
 *   // Configure 4 channels
 *   MCF_ADC_ADLST1 = MCF_ADC_ADLST1_SAMPLE0(0) |  // AN0
 *                    MCF_ADC_ADLST1_SAMPLE1(1) |  // AN1
 *                    MCF_ADC_ADLST1_SAMPLE2(2) |  // AN2
 *                    MCF_ADC_ADLST1_SAMPLE3(3);   // AN3
 *   MCF_ADC_ADSDIS = 0xF0;  // Disable slots 4-7
 *
 *   // Enable end-of-scan interrupt, loop mode
 *   MCF_ADC_CTRL1 = MCF_ADC_CTRL1_SMODE_LOOP_SEQ |
 *                   MCF_ADC_CTRL1_EOSIE0 |
 *                   MCF_ADC_CTRL1_START0;
 */


/********************************************************************/

#endif /* __MCF5223_ADC_H__ */
