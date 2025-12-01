/*
 * File:    mcf5xxx.h
 * Purpose: Common definitions for all ColdFire processors
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5XXX_H
#define MCF5XXX_H

#include <stdint.h>

/*********************************************************************
 * Boolean and common definitions
 *********************************************************************/

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef ON
#define ON      (1)
#endif

#ifndef OFF
#define OFF     (0)
#endif

/*********************************************************************
 * Data type definitions (for compatibility with Freescale headers)
 *********************************************************************/

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t     int32;

typedef volatile int8_t     vint8;
typedef volatile int16_t    vint16;
typedef volatile int32_t    vint32;

typedef volatile uint8_t    vuint8;
typedef volatile uint16_t   vuint16;
typedef volatile uint32_t   vuint32;

/*********************************************************************
 * Common M68K & ColdFire definitions
 *********************************************************************/

#define ADDRESS         uint32_t
#define INSTRUCTION     uint16_t
#define ILLEGAL         0x4AFC
#define CPU_WORD_SIZE   16

/*********************************************************************
 * Status Register bit definitions
 *********************************************************************/

#define MCF5XXX_SR_T        (0x8000)    /* Trace mode */
#define MCF5XXX_SR_S        (0x2000)    /* Supervisor mode */
#define MCF5XXX_SR_M        (0x1000)    /* Master/Interrupt state */
#define MCF5XXX_SR_IPL      (0x0700)    /* Interrupt priority level mask */
#define MCF5XXX_SR_IPL_0    (0x0000)
#define MCF5XXX_SR_IPL_1    (0x0100)
#define MCF5XXX_SR_IPL_2    (0x0200)
#define MCF5XXX_SR_IPL_3    (0x0300)
#define MCF5XXX_SR_IPL_4    (0x0400)
#define MCF5XXX_SR_IPL_5    (0x0500)
#define MCF5XXX_SR_IPL_6    (0x0600)
#define MCF5XXX_SR_IPL_7    (0x0700)
#define MCF5XXX_SR_X        (0x0010)    /* Extend flag */
#define MCF5XXX_SR_N        (0x0008)    /* Negative flag */
#define MCF5XXX_SR_Z        (0x0004)    /* Zero flag */
#define MCF5XXX_SR_V        (0x0002)    /* Overflow flag */
#define MCF5XXX_SR_C        (0x0001)    /* Carry flag */

/*********************************************************************
 * RAM Base Address Register (RAMBAR) definitions
 *********************************************************************/

#define MCF5XXX_RAMBAR_BA(a)    ((a) & 0xFFFFC000)
#define MCF5XXX_RAMBAR_PRI_00   (0x00000000)
#define MCF5XXX_RAMBAR_PRI_01   (0x00004000)
#define MCF5XXX_RAMBAR_PRI_10   (0x00008000)
#define MCF5XXX_RAMBAR_PRI_11   (0x0000C000)
#define MCF5XXX_RAMBAR_WP       (0x00000100)    /* Write protect */
#define MCF5XXX_RAMBAR_CI       (0x00000020)    /* Cache inhibit */
#define MCF5XXX_RAMBAR_SC       (0x00000010)    /* Supervisor code */
#define MCF5XXX_RAMBAR_SD       (0x00000008)    /* Supervisor data */
#define MCF5XXX_RAMBAR_UC       (0x00000004)    /* User code */
#define MCF5XXX_RAMBAR_UD       (0x00000002)    /* User data */
#define MCF5XXX_RAMBAR_V        (0x00000001)    /* Valid */

/*********************************************************************
 * Exception stack frame macros
 *
 * The ColdFire family has a simplified exception stack frame:
 *
 *              3322222222221111 111111
 *              1098765432109876 5432109876543210
 *           8 +----------------+----------------+
 *             |         Program Counter         |
 *           4 +----------------+----------------+
 *             |FS/Fmt/Vector/FS|      SR        |
 *   SP -->  0 +----------------+----------------+
 *********************************************************************/

#define MCF5XXX_RD_SF_FORMAT(PTR)   \
    ((*((uint16_t *)(PTR)) >> 12) & 0x00FF)

#define MCF5XXX_RD_SF_VECTOR(PTR)   \
    ((*((uint16_t *)(PTR)) >> 2) & 0x00FF)

#define MCF5XXX_RD_SF_FS(PTR)       \
    (((*((uint16_t *)(PTR)) & 0x0C00) >> 8) | (*((uint16_t *)(PTR)) & 0x0003))

#define MCF5XXX_SF_SR(PTR)  *((uint16_t *)(PTR) + 1)
#define MCF5XXX_SF_PC(PTR)  *((uint32_t *)(PTR) + 1)

/*********************************************************************
 * Inline assembly helpers
 *********************************************************************/

/* Set interrupt priority level, returns previous level */
static inline uint32_t mcf5xxx_set_ipl(uint32_t new_ipl)
{
    uint32_t old_sr, new_sr;
    __asm__ __volatile__ (
        "move.w %%sr, %0\n\t"
        "move.l %0, %1\n\t"
        "andi.l #0xF8FF, %1\n\t"
        "or.l %2, %1\n\t"
        "move.w %1, %%sr"
        : "=d" (old_sr), "=d" (new_sr)
        : "d" ((new_ipl & 7) << 8)
    );
    return (old_sr >> 8) & 7;
}

/* Disable interrupts */
static inline void mcf5xxx_irq_disable(void)
{
    __asm__ __volatile__ ("move.w #0x2700, %sr");
}

/* Enable interrupts */
static inline void mcf5xxx_irq_enable(void)
{
    __asm__ __volatile__ ("move.w #0x2000, %sr");
}

/* No operation */
static inline void mcf5xxx_nop(void)
{
    __asm__ __volatile__ ("nop");
}

/* Halt processor */
static inline void mcf5xxx_halt(void)
{
    __asm__ __volatile__ ("halt");
}

/* Stop processor with specified SR value */
static inline void mcf5xxx_stop(uint16_t sr_value)
{
    __asm__ __volatile__ ("stop %0" : : "i" (sr_value));
}

#endif /* MCF5XXX_H */
