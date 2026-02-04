/*
 * am243x_internal.ld - AM243x linker script (internal RAM only)
 */

--retain="*(.vectors)"

MEMORY
{
    /* Internal RAM - 512KB */
    RAM     : ORIGIN = 0x70000000, LENGTH = 0x00080000

    /* On-chip flash - 2MB */
    FLASH   : ORIGIN = 0x60000000, LENGTH = 0x00200000
}

SECTIONS
{
    .vectors : {} > FLASH
    .text    : {} > FLASH
    .rodata  : {} > FLASH
    .data    : {} > RAM
    .bss     : {} > RAM
}
