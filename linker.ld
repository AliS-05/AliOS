ENTRY(kernel)

SECTIONS
{
	. = 0x8000; /*origin*/

	.text : { *(.text) }
	.rodata : { *(.rodata) }  
	.data ALIGN(4): { *(.data) }
	.bss ALIGN(4) : { *(.bss) }
}
