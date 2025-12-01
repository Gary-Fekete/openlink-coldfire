/*
 * MCF52235 Linker Script for M52233DEMO Board
 *
 * Memory Map:
 *   Flash:  0x00000000 - 0x0003FFFF (256KB)
 *   SRAM:   0x20000000 - 0x20007FFF (32KB)
 *   IPSBAR: 0x40000000 - 0x401FFFFF (Peripheral registers)
 */

MEMORY
{
    flash (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
    sram  (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

/* Entry point */
ENTRY(_start)

/* Stack at top of SRAM */
_stack_top = ORIGIN(sram) + LENGTH(sram);

SECTIONS
{
    /* Vector table at start of flash */
    .vectors :
    {
        . = ALIGN(4);
        _vectors_start = .;
        KEEP(*(.vectors))
        . = ALIGN(4);
        _vectors_end = .;
    } > flash

    /* Code and read-only data */
    .text :
    {
        . = ALIGN(4);
        _text_start = .;
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        _text_end = .;
    } > flash

    /* Exception handling (C++) */
    .eh_frame :
    {
        *(.eh_frame)
    } > flash

    /* Initialized data - stored in flash, copied to SRAM at startup */
    .data :
    {
        . = ALIGN(4);
        _data_start = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _data_end = .;
    } > sram AT > flash

    _data_load = LOADADDR(.data);

    /* Uninitialized data - in SRAM, zeroed at startup */
    .bss :
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > sram

    /* Heap starts after BSS */
    .heap :
    {
        . = ALIGN(4);
        _heap_start = .;
        . = . + 0x1000;  /* 4KB heap minimum */
        _heap_end = .;
    } > sram

    /* Stack at end of SRAM (4KB reserved) */
    .stack (NOLOAD) :
    {
        . = ALIGN(4);
        _stack_end = .;
        . = . + 0x1000;  /* 4KB stack */
    } > sram

    /* Debug info */
    .debug_info     0 : { *(.debug_info) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_ranges   0 : { *(.debug_ranges) }
}
