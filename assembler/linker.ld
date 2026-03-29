ENTRY(_start)
SECTIONS
{
    . = 0x200000;
    .text :
    {
        *(.text.startup)  /* ← _start goes here first */
        *(.text)          /* ← Everything else after */
    }
    .rodata : { *(.rodata) }
    .data ALIGN(4) : { *(.data) }
    .bss ALIGN(4) :
    {
        bss_start = .;
        *(.bss)
        *(COMMON)
        bss_end = .;
    }
}
