ENTRY(assemble_buffer)

SECTIONS
{
    . = 0x8000; /* load address */

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data ALIGN(4) :
    {
        *(.data)
    }

    .bss ALIGN(4) :
    {
        bss_start = .;      /* start of BSS */
        *(.bss)
        *(COMMON)
        bss_end = .;        /* end of BSS */
    }
}
