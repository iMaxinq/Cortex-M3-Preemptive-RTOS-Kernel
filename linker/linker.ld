/* Memory layout for QEMU mps2-an385. The boundary symbols (_estack, _sdata, ...)
 * are used by startup.c, which does the actual copying and zeroing. */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

/* Initial MSP: top of RAM, since the stack grows down */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Code and constants */
    .text :
    {
        . = ALIGN(4);

        /* Vector table must be at 0x00000000 */
        KEEP(*(.isr_vector))
        
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* FLASH address of the .data initial values */
    _sidata = LOADADDR(.data);

    /* Initialized data: lives in RAM (VMA), stored in FLASH (LMA) */
    .data :
    {
        . = ALIGN(4);

        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);

        _edata = .;
    } > RAM AT> FLASH

    /* Zero-initialized data */
    .bss :
    {
        . = ALIGN(4);

        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > RAM
}