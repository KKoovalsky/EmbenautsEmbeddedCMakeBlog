/*
 * tms570_external.ld - TMS570 linker script (with external RAM)
 */

--retain="*(.vectors)"

MEMORY
{
    /* Internal RAM - 256KB */
    RAM     : ORIGIN = 0x08000000, LENGTH = 0x00040000

    /* On-chip flash - 3MB */
    FLASH   : ORIGIN = 0x00000000, LENGTH = 0x00300000

    /* External SRAM - 2MB */
    EXTRAM  : ORIGIN = 0x60000000, LENGTH = 0x00200000
}

SECTIONS
{
    .vectors : {} > FLASH
    .rodata  : {} > FLASH
    .data    : {} > RAM
    .bss     : {} > RAM

    /* Large buffers and code goes to external SRAM */
    .ext_data : {} > EXTRAM
    .text    : {} > EXTRAM
}
