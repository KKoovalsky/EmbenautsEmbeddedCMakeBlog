/*
 * am243x_external.ld - AM243x linker script (with external RAM)
 */

--retain="*(.vectors)"

MEMORY
{
    /* Internal RAM - 512KB */
    RAM     : ORIGIN = 0x70000000, LENGTH = 0x00080000

    /* On-chip flash - 2MB */
    FLASH   : ORIGIN = 0x60000000, LENGTH = 0x00200000

    /* External DDR - 256MB */
    DDR     : ORIGIN = 0x80000000, LENGTH = 0x10000000
}

SECTIONS
{
    .vectors : {} > FLASH
    .rodata  : {} > FLASH
    .data    : {} > RAM
    .bss     : {} > RAM

    /* Code goes to the external DDR */
    .text    : {} > DDR
    .ddr_data : {} > DDR
}
