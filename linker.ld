ENTRY(kernel)

SECTIONS
{
	. = 0x8000; /*origin*/

	.text : {
		*(.text)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}
}
