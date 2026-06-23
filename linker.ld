ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS
{
    .text   : { *(.text*)   } > RAM
    .rodata : { *(.rodata*) } > RAM
    .data   : { *(.data*)   } > RAM
    .bss    : { *(.bss*) *(COMMON) } > RAM
    .stack  : { *(.stack)   } > RAM   /* stack at the very end, safely away */
}