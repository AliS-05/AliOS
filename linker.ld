ENTRY(kernel)

SECTIONS
{
	. = 0x8000; /* kernel load address */

	.text : { *(.text) }

	.rodata : { *(.rodata) }

	.data ALIGN(4) :
	{
		*(.data)
	}

	.bss ALIGN(4) :
	{
		bss_start = .;      /* hyper important for the kernel to not triple fault */
		*(.bss)
		*(COMMON)
		bss_end = .;        
	}
}
