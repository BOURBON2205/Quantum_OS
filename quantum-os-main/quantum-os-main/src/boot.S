/* ============================================================================
 * boot.S
 * QuantumOS boot entry, Multiboot header, and bootstrap stack.
 * ========================================================================== */
.set MAGIC,    0x1BADB002
.set FLAGS,    0
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text
.global _start
_start:
    cli                         /* interrupts stay disabled during bootstrap */
    mov $stack_top, %esp       /* switch to the kernel bootstrap stack */
    xor %ebp, %ebp              /* establish a clean frame base */
    call kmain

1:
    hlt
    jmp 1b

.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:
