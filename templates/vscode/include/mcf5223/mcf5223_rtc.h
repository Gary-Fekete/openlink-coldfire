/*
 * File:    mcf5223_rtc.h
 * Purpose: Register and bit definitions for Real-time Clock
 *
 * Based on MCF52235 Reference Manual, Chapter 8 - Real-Time Clock (RTC)
 * Base Address: IPSBAR + 0x0003C0
 */

#ifndef __MCF5223_RTC_H__
#define __MCF5223_RTC_H__

#include <stdint.h>

/*********************************************************************
*
* Real-time Clock (RTC)
*
*********************************************************************/

/* Base address */
#define MCF_RTC_BASE            (IPSBAR + 0x0003C0)

/* Register definitions */
#define MCF_RTC_HOURMIN         (*(volatile uint32_t *)(MCF_RTC_BASE + 0x00))
#define MCF_RTC_SECONDS         (*(volatile uint32_t *)(MCF_RTC_BASE + 0x04))
#define MCF_RTC_ALRM_HM         (*(volatile uint32_t *)(MCF_RTC_BASE + 0x08))
#define MCF_RTC_ALRM_SEC        (*(volatile uint32_t *)(MCF_RTC_BASE + 0x0C))
#define MCF_RTC_CR              (*(volatile uint32_t *)(MCF_RTC_BASE + 0x10))
#define MCF_RTC_ISR             (*(volatile uint32_t *)(MCF_RTC_BASE + 0x14))
#define MCF_RTC_IER             (*(volatile uint32_t *)(MCF_RTC_BASE + 0x18))
#define MCF_RTC_STPWCH          (*(volatile uint32_t *)(MCF_RTC_BASE + 0x1C))
#define MCF_RTC_DAYS            (*(volatile uint32_t *)(MCF_RTC_BASE + 0x20))
#define MCF_RTC_ALRM_DAY        (*(volatile uint32_t *)(MCF_RTC_BASE + 0x24))

/* Bit definitions and macros for MCF_RTC_HOURMIN */
#define MCF_RTC_HOURMIN_MINUTES(x)   (((x)&0x0000003F)<<0)
#define MCF_RTC_HOURMIN_HOURS(x)     (((x)&0x0000001F)<<8)

/* Bit definitions and macros for MCF_RTC_SECONDS */
#define MCF_RTC_SECONDS_SECONDS(x)   (((x)&0x0000003F)<<0)

/* Bit definitions and macros for MCF_RTC_ALRM_HM */
#define MCF_RTC_ALRM_HM_MINUTES(x)   (((x)&0x0000003F)<<0)
#define MCF_RTC_ALRM_HM_HOURS(x)     (((x)&0x0000001F)<<8)

/* Bit definitions and macros for MCF_RTC_ALRM_SEC */
#define MCF_RTC_ALRM_SEC_SECONDS(x)  (((x)&0x0000003F)<<0)

/* Bit definitions and macros for MCF_RTC_CR */
#define MCF_RTC_CR_SWR               (0x00000001)  /* Software Reset */
#define MCF_RTC_CR_XTL(x)            (((x)&0x00000003)<<5)
#define MCF_RTC_CR_EN                (0x00000080)  /* RTC Enable */
#define MCF_RTC_CR_32768             (0x0)
#define MCF_RTC_CR_32000             (0x1)
#define MCF_RTC_CR_38400             (0x2)

/* Bit definitions and macros for MCF_RTC_ISR */
#define MCF_RTC_ISR_SW               (0x00000001)
#define MCF_RTC_ISR_MIN              (0x00000002)
#define MCF_RTC_ISR_ALM              (0x00000004)
#define MCF_RTC_ISR_DAY              (0x00000008)
#define MCF_RTC_ISR_1HZ              (0x00000010)
#define MCF_RTC_ISR_HR               (0x00000020)
#define MCF_RTC_ISR_2HZ              (0x00000080)
#define MCF_RTC_ISR_SAM0             (0x00000100)
#define MCF_RTC_ISR_SAM1             (0x00000200)
#define MCF_RTC_ISR_SAM2             (0x00000400)
#define MCF_RTC_ISR_SAM3             (0x00000800)
#define MCF_RTC_ISR_SAM4             (0x00001000)
#define MCF_RTC_ISR_SAM5             (0x00002000)
#define MCF_RTC_ISR_SAM6             (0x00004000)
#define MCF_RTC_ISR_SAM7             (0x00008000)

/* Bit definitions and macros for MCF_RTC_IER */
#define MCF_RTC_IER_SW               (0x00000001)
#define MCF_RTC_IER_MIN              (0x00000002)
#define MCF_RTC_IER_ALM              (0x00000004)
#define MCF_RTC_IER_DAY              (0x00000008)
#define MCF_RTC_IER_1HZ              (0x00000010)
#define MCF_RTC_IER_HR               (0x00000020)
#define MCF_RTC_IER_2HZ              (0x00000080)
#define MCF_RTC_IER_SAM0             (0x00000100)
#define MCF_RTC_IER_SAM1             (0x00000200)
#define MCF_RTC_IER_SAM2             (0x00000400)
#define MCF_RTC_IER_SAM3             (0x00000800)
#define MCF_RTC_IER_SAM4             (0x00001000)
#define MCF_RTC_IER_SAM5             (0x00002000)
#define MCF_RTC_IER_SAM6             (0x00004000)
#define MCF_RTC_IER_SAM7             (0x00008000)

/* Bit definitions and macros for MCF_RTC_STPWCH */
#define MCF_RTC_STPWCH_CNT(x)        (((x)&0x0000003F)<<0)

/* Bit definitions and macros for MCF_RTC_DAYS */
#define MCF_RTC_DAYS_DAYS(x)         (((x)&0x0000FFFF)<<0)

/* Bit definitions and macros for MCF_RTC_ALRM_DAY */
#define MCF_RTC_ALRM_DAY_DAYS(x)     (((x)&0x0000FFFF)<<0)

/********************************************************************/

#endif /* __MCF5223_RTC_H__ */
