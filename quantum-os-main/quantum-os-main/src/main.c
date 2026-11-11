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
    kputs("[ READY ] kernel online | shell ready | scheduler armed\n\n");
    console_color(15);
    __asm__ volatile("sti");
    shell_run();
}
