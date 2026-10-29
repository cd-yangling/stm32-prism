ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);
PROVIDE(RAM_LEN = 128K);

MEMORY
{
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = RAM_LEN
}

SECTIONS
{
    .isr_vector : {
        . = ALIGN(4);
        _sisr = .;
        KEEP(*(.isr_vector))
    } > RAM
    ASSERT((_sisr & 0x3FF) == 0, "VTOR requires 1024-byte alignment")

    .text : {
        *(.text*)
        *(.rodata*)
        *(.ARM.exidx*)
        . = ALIGN(4);
    } > RAM

    .data : {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM
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
