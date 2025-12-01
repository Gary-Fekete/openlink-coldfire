/*
 * MCF52235 Startup Code
 *
 * This file contains the vector table and startup code for the
 * M52233DEMO board with MCF52235 ColdFire V2 processor.
 */

    .section .vectors, "ax"
    .global _vectors

_vectors:
    /* Initial Stack Pointer */
    .long   _stack_top          /* 0x000: Initial SP */
    .long   _start              /* 0x004: Initial PC (Reset) */

    /* Exception Vectors */
    .long   _fault_handler      /* 0x008: Access Error */
    .long   _fault_handler      /* 0x00C: Address Error */
    .long   _fault_handler      /* 0x010: Illegal Instruction */
    .long   _fault_handler      /* 0x014: Divide by Zero */
    .long   _fault_handler      /* 0x018: Reserved */
    .long   _fault_handler      /* 0x01C: Reserved */
    .long   _fault_handler      /* 0x020: Privilege Violation */
    .long   _fault_handler      /* 0x024: Trace */
    .long   _fault_handler      /* 0x028: Unimplemented Line A */
    .long   _fault_handler      /* 0x02C: Unimplemented Line F */
    .long   _fault_handler      /* 0x030: Debug Interrupt */
    .long   _fault_handler      /* 0x034: Reserved */
    .long   _fault_handler      /* 0x038: Format Error */
    .long   _fault_handler      /* 0x03C: Uninitialized Interrupt */

    /* Reserved vectors 0x040-0x05C */
    .rept 8
    .long   _fault_handler
    .endr

    /* Spurious Interrupt */
    .long   _fault_handler      /* 0x060: Spurious Interrupt */

    /* Autovector interrupts 0x064-0x07C */
    .rept 7
    .long   _fault_handler
    .endr

    /* TRAP vectors 0x080-0x0BC */
    .rept 16
    .long   _fault_handler
    .endr

    /* Reserved vectors 0x0C0-0x0FC */
    .rept 16
    .long   _fault_handler
    .endr

    /* Interrupt vectors 0x100-0x3FC (192 vectors) */
    .rept 192
    .long   _fault_handler
    .endr

    .section .text
    .global _start
    .global _fault_handler

/*
 * _start - Entry point after reset
 */
_start:
    /* Disable interrupts */
    move.w  #0x2700, %sr

    /* Set up VBR (Vector Base Register) to point to flash */
    move.l  #_vectors, %d0
    movec   %d0, %vbr

    /* Initialize RAMBAR - enable SRAM at 0x20000000 */
    move.l  #0x20000221, %d0    /* Base 0x20000000, valid, r/w enabled */
    movec   %d0, %rambar

    /* Set up stack pointer */
    move.l  #_stack_top, %sp

    /* Copy .data section from flash to SRAM */
    lea     _data_load, %a0     /* Source (flash) */
    lea     _data_start, %a1    /* Destination (SRAM) */
    lea     _data_end, %a2      /* End of data */

_copy_data:
    cmp.l   %a1, %a2
    beq     _clear_bss
    move.l  (%a0)+, (%a1)+
    bra     _copy_data

    /* Clear .bss section */
_clear_bss:
    lea     _bss_start, %a0
    lea     _bss_end, %a1
    clr.l   %d0

_clear_loop:
    cmp.l   %a0, %a1
    beq     _call_main
    move.l  %d0, (%a0)+
    bra     _clear_loop

    /* Call main() */
_call_main:
    jsr     main

    /* If main returns, loop forever */
_halt:
    stop    #0x2000
    bra     _halt

/*
 * _fault_handler - Default exception handler
 */
_fault_handler:
    /* Halt on any unhandled exception */
    halt
    bra     _fault_handler

    .end
