/* ============================================================================
 * main.c
 * Kernel entry point and subsystem initialization order.
 * ========================================================================== */
#include "kernel.h"

void kmain(void) {
    console_init();
    interrupts_init();
    mem_init();
    threads_init();
    fs_init();
    banner();
    console_color(10);
    kprintf("[BOOT] All kernel subsystems initialized successfully.\n");
    console_color(15);
    kputs("Type 'help' to open the QuantumOS Command Center.\n\n");
    __asm__ volatile("sti");
    shell_run();
}
