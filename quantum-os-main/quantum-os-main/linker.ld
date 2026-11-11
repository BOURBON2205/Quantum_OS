/* QuantumOS kernel linker script */
ENTRY(_start)

SECTIONS
{
    /* Load the kernel at the conventional 1 MiB physical address. */
    . = 1M;

    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data : ALIGN(4K)
    {
        *(.data*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss*)
    }
}
