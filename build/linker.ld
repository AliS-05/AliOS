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
		bss_start = .;      /* <-- ADD THIS */
		*(.bss)
		*(COMMON)
		bss_end = .;        /* <-- ADD THIS */
	}
}

