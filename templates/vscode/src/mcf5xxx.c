/*
 * File:    mcf5xxx.c
 * Purpose: Common ColdFire CPU support functions
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 * Copyright (C) 2025 Gary Fekete
 *
 * Functions for exception handling, interrupt management,
 * and CPU identification for all ColdFire V2 processors.
 */

#include "mcf5xxx.h"
#include "mcf52235.h"
#include <stdint.h>

/*********************************************************************
 * Exception Vector Table
 *
 * The vector table can be relocated by writing to VBR.
 * This RAM-based table allows dynamic handler installation.
 *********************************************************************/

/* Default vector table in RAM (256 vectors * 4 bytes = 1KB) */
static ADDRESS vector_table[256] __attribute__((aligned(4)));

/* Flag to track if vector table has been initialized */
static int vectors_initialized = 0;

/*********************************************************************
 * Exception Names (for debugging)
 *********************************************************************/

__attribute__((unused))
static const char * const exception_names[] = {
    "Initial SP",           /* 0 */
    "Initial PC",           /* 1 */
    "Access Error",         /* 2 */
    "Address Error",        /* 3 */
    "Illegal Instruction",  /* 4 */
    "Divide by Zero",       /* 5 */
    "Reserved",             /* 6 */
    "Reserved",             /* 7 */
    "Privilege Violation",  /* 8 */
    "Trace",                /* 9 */
    "Line A",               /* 10 */
    "Line F",               /* 11 */
    "Debug",                /* 12 */
    "Reserved",             /* 13 */
    "Format Error",         /* 14 */
    "Uninitialized Int",    /* 15 */
};

/*********************************************************************
 * mcf5xxx_exception_handler
 *
 * This is the C-level exception handler called from the assembly
 * wrapper asm_exception_handler in mcf5xxx.S
 *
 * The assembly code saves D0-D1/A0-A1 and passes a pointer to the
 * exception stack frame.
 *********************************************************************/

/* Alias for assembly code that expects leading underscore */
void mcf5xxx_exception_handler(void *framep) __attribute__((alias("_mcf5xxx_exception_handler")));

void _mcf5xxx_exception_handler(void *framep)
{
    uint16_t *frame = (uint16_t *)framep;
    uint16_t sr;
    uint32_t pc;
    int vector;
    int format;

    /* Extract information from exception frame */
    sr = MCF5XXX_SF_SR(frame);
    pc = MCF5XXX_SF_PC(frame);
    vector = MCF5XXX_RD_SF_VECTOR(frame);
    format = MCF5XXX_RD_SF_FORMAT(frame);

    (void)sr;      /* Suppress unused warning if not used */
    (void)pc;
    (void)format;

    /* Check if there's a custom handler installed */
    if (vectors_initialized && vector < 256 && vector_table[vector] != 0) {
        /* Call the installed handler */
        void (*handler)(void) = (void (*)(void))vector_table[vector];
        handler();
        return;
    }

    /* Default behavior: halt on unhandled exception */
    /* In a real system, you might log this or reset */

    /* For vectors 64-255, these are typically peripheral interrupts */
    if (vector >= 64) {
        /* Call cpu_handle_interrupt for peripheral interrupts */
        cpu_handle_interrupt(vector - 64);
        return;
    }

    /* For CPU exceptions (vectors 0-63), halt */
    /* You could add debug output here if UART is initialized */

    /* Infinite loop - system halted */
    while (1) {
        __asm__ __volatile__("halt");
    }
}

/*********************************************************************
 * mcf5xxx_interpret_d0d1
 *
 * Decode and print the CPU identification from D0/D1 reset values.
 * These registers contain processor feature information at reset.
 *********************************************************************/

void mcf5xxx_interpret_d0d1(int d0, int d1)
{
    /* D0 contains:
     * Bits 31-24: Processor Family
     * Bits 23-20: Version
     * Bits 19-16: Revision
     * Bit 15: MAC present
     * Bit 14: DIV present
     * Bit 13: EMAC present
     * Bit 12: FPU present
     * Bit 11: MMU present
     * Bits 7-4: ISA revision
     * Bits 3-0: Debug revision
     */

    /* D1 contains:
     * Bits 31-30: Cache line size
     * Bits 29-28: I-cache associativity
     * Bits 27-24: I-cache size
     * Bits 23-20: RAM0 size
     * Bits 19-16: ROM0 size
     * Bits 15-14: Bus width
     * Bits 13-12: D-cache associativity
     * Bits 11-8: D-cache size
     * Bits 7-4: RAM1 size
     * Bits 3-0: ROM1 size
     */

    (void)d0;
    (void)d1;

    /* Implementation would typically print to UART */
    /* For now, just a stub */
}

/*********************************************************************
 * mcf5xxx_irq_enable / mcf5xxx_irq_disable
 *
 * Global interrupt enable/disable using the Status Register.
 * These modify the IPL (Interrupt Priority Level) bits in SR.
 *********************************************************************/

void mcf5xxx_irq_enable(void)
{
    /* Set IPL to 0, enabling all interrupts */
    /* SR bits 10-8 contain the IPL */
    asm_set_ipl(0);
}

void mcf5xxx_irq_disable(void)
{
    /* Set IPL to 7, disabling all maskable interrupts */
    asm_set_ipl(7);
}

/*********************************************************************
 * mcf5xxx_set_handler
 *
 * Install a custom exception/interrupt handler.
 * Returns the previous handler address.
 *
 * vector: Exception vector number (0-255)
 * handler: Address of new handler function
 *********************************************************************/

ADDRESS mcf5xxx_set_handler(int vector, ADDRESS handler)
{
    ADDRESS old_handler;

    if (vector < 0 || vector >= 256) {
        return 0;
    }

    /* Initialize vector table on first use */
    if (!vectors_initialized) {
        int i;
        for (i = 0; i < 256; i++) {
            vector_table[i] = 0;
        }

        /* Point VBR to our RAM vector table */
        mcf5xxx_wr_vbr((uint32)vector_table);

        vectors_initialized = 1;
    }

    /* Save old handler and install new one */
    old_handler = vector_table[vector];
    vector_table[vector] = handler;

    return old_handler;
}

/*********************************************************************
 * cpu_handle_interrupt (weak)
 *
 * Default interrupt handler - does nothing.
 * Override this in your application to handle peripheral interrupts.
 *********************************************************************/

__attribute__((weak))
void cpu_handle_interrupt(int irq)
{
    /* Default: do nothing */
    (void)irq;
}

/*********************************************************************
 * Additional utility functions
 *********************************************************************/

/*
 * Get current interrupt priority level
 */
int mcf5xxx_get_ipl(void)
{
    return asm_set_ipl(0) & 0x07;  /* Read and restore */
}

/*
 * Set interrupt priority level (convenience wrapper)
 * Returns previous IPL
 */
int mcf5xxx_set_ipl(int ipl)
{
    return asm_set_ipl(ipl & 0x07);
}

/*
 * Enter supervisor mode (if not already)
 * Note: This requires special handling on ColdFire
 */
void mcf5xxx_enter_supervisor(void)
{
    /* On ColdFire, use the SR to ensure supervisor mode */
    /* Set S bit (bit 13) in SR */
    uint32_t sr;
    __asm__ __volatile__(
        "move.w %%sr, %0"
        : "=d" (sr)
    );
    sr |= MCF5XXX_SR_S;
    mcf5xxx_wr_sr(sr);
}
