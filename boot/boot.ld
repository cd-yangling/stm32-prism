PROVIDE(BOOT_LEN = 128K);
PROVIDE(RAM_LEN  = 64K);

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = BOOT_LEN
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = RAM_LEN
}

ENTRY(reset_handler)

SECTIONS
{
    .isr_vector : {
        KEEP(*(.isr_vector))
    } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data : { *(.data*) } > FLASH
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM

    ASSERT(SIZEOF(.data) == 0 && SIZEOF(.bss) == 0,
           "boot: .data/.bss non-empty; trampoline has no runtime init, refusing to link")

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
