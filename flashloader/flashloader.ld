/*
 * OpenLink ColdFire Flashloader Linker Script
 *
 * The flashloader is loaded to SRAM at 0x20000500 and executed.
 * Parameter block is at 0x20000000, data buffer at 0x20000100.
 */

MEMORY
{
    /* Parameters and data buffer - reserved, not used by linker */
    params (rw)  : ORIGIN = 0x20000000, LENGTH = 0x100
    buffer (rw)  : ORIGIN = 0x20000100, LENGTH = 0x400

    /* Flashloader code area */
    sram (rwx)   : ORIGIN = 0x20000500, LENGTH = 0x7B00

    /* Stack at top of SRAM (32KB total: 0x20000000-0x20007FFF) */
}

ENTRY(_start)

SECTIONS
{
    /* Vectors/startup at beginning */
    .vectors : {
        *(.vectors)
    } > sram

    /* Code */
    .text : {
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    } > sram

    /* Initialized data */
    .data : {
        *(.data)
        *(.data.*)
    } > sram

    /* Uninitialized data */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } > sram

    /* Discard */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.ARM.*)
    }
}
