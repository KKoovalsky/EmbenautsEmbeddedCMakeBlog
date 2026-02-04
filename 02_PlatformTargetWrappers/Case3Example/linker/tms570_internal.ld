/*
 * tms570_internal.ld - TMS570 linker script (internal RAM only)
 */

--retain="*(.vectors)"

MEMORY
{
    /* Internal RAM - 256KB */
    RAM     : ORIGIN = 0x08000000, LENGTH = 0x00040000

    /* On-chip flash - 3MB */
    FLASH   : ORIGIN = 0x00000000, LENGTH = 0x00300000
}

SECTIONS
{
    .vectors : {} > FLASH
    .text    : {} > FLASH
    .rodata  : {} > FLASH
    .data    : {} > RAM
    .bss     : {} > RAM

}
