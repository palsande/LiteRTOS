ENTRY(_start)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

SECTIONS
{
    .text : {
        *(.vectors)  /* Vector table */
        *(.text*)    /* Code */
        *(.rodata*)  /* Read-only data */
        . = ALIGN(4);
    } > FLASH

    .data : {
        *(.data*)
        . = ALIGN(4);
    } > SRAM AT > FLASH

    .bss : {
        *(.bss*)
        . = ALIGN(4);
    } > SRAM
}

