ENTRY(_start)
SECTIONS
{
    . = 0x200000;
    .text :
    {
        *(.text.startup)  /*hyper important for assembler to run*/
        *(.text)          
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
