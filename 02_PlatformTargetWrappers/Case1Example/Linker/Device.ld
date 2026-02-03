/*
 * Device.ld - TI Linker command file for AM243x (example)
 * This is a simplified example - real scripts are more complex
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
    /* Vector table at start of flash */
    .vectors : {} > FLASH

    /* Code */
    .text : {} > FLASH

    /* Read-only data */
    .rodata : {} > FLASH

    /* Initialized data */
    .data : {} > RAM

    /* Uninitialized data */
    .bss : {} > RAM
}
