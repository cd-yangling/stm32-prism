ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);
PROVIDE(APP_OFFSET = 0);
PROVIDE(APP_LENGTH = 2M);
PROVIDE(RAM_LEN   = 128K);

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000 + APP_OFFSET, LENGTH = APP_LENGTH
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = RAM_LEN
}

SECTIONS
{
    .isr_vector : {
        . = ALIGN(4);
        _sisr = .;
        KEEP(*(.isr_vector))
    } > FLASH
    ASSERT((_sisr & 0x3FF) == 0, "VTOR requires 1024-byte alignment")

    .text : {
        *(.text*)
        *(.rodata*)
        *(.ARM.exidx*)
        . = ALIGN(4);
    } > FLASH

    .data : {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    _sidata = LOADADDR(.data);

    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*) *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    /DISCARD/ : { *(.comment) *(.note*) }
}
