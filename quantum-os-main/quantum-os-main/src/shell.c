/* ============================================================================
 * shell.c
 * QuantumOS command shell and Command-line interface.
 * ========================================================================== */
#include "kernel.h"

static void *h_ptr[16];
static u32 h_sz[16];

/* ---------- visual helpers ---------- */

static void ui_rule(void) {
    console_color(8);
    kputs("-------------------------------------------------------------------------------\n");
    console_color(15);
}

static void ui_header(const char *name, const char *subtitle) {
    console_color(11);
    kputs("\n  ");
    kputs(name);
    kputs("\n");
    console_color(7);
    kputs("  ");
    kputs(subtitle);
    kputs("\n");
    ui_rule();
}

static void ui_section(const char *id, const char *name) {
    console_color(11);
    kprintf("\n  [%s] %s\n", id, name);
    console_color(8);
    kputs("  -----------------------------------------------------------------------\n");
    console_color(15);
}

static void ui_row(const char *key, const char *value) {
    int len = (int)strlen(key);
    console_color(14);
    kprintf("  %s", key);
    for (int i = len; i < 20; i++) kputc(' ');
    console_color(15);
    kputs(value);
    kputc('\n');
}

static void ui_ok(const char *text) {
    console_color(10);
    kputs("  [ OK ] ");
    console_color(15);
    kputs(text);
    kputc('\n');
}

static void ui_info(const char *text) {
    console_color(7);
    kputs("  [ .. ] ");
    console_color(15);
    kputs(text);
    kputc('\n');
}

static void ui_error(const char *text) {
    console_color(12);
    kputs("  [ERR ] ");
    console_color(15);
    kputs(text);
    kputc('\n');
}

static void ui_usage(const char *text) {
    console_color(14);
    kputs("  usage: ");
    console_color(15);
    kputs(text);
    kputc('\n');
}

void banner(void) {
    /* Compact welcome header leaves room for command output on 80x25 VGA. */
    console_color(11);
    kputs("\n  +----------------------------------------------------------------------------+\n");
    console_color(15);
    kputs("  |  QUANTUM OS                                                  VERSION 1.0   |\n");
    console_color(11);
    kputs("  +----------------------------------------------------------------------------+\n");

    console_color(7);
    kputs("  A lightweight 32-bit operating system for learning core OS concepts.\n");
    console_color(10);
    kputs("  [OK] CPU  [OK] TIMER  [OK] MEMORY  [OK] THREADS  [OK] RAMFS  [OK] SECURITY\n");

    console_color(11);
    kputs("  MAIN MENU\n");
    console_color(8);
    kputs("  ------------------------------------------------------------------------------\n");
    console_color(14);
    kputs("   01  Number conversion       02  System information\n");
    kputs("   03  Processes & scheduling  04  Memory management\n");
    kputs("   05  File system             06  Security & permissions\n");
    kputs("   07  OS demonstrations\n");

    console_color(7);
    kputs("  Type ");
    console_color(14);
    kputs("help");
    console_color(7);
    kputs(" for commands, or ");
    console_color(14);
    kputs("help 1");
    console_color(7);
    kputs(" to ");
    console_color(14);
    kputs("help 7");
    console_color(7);
    kputs(" for details.\n");
    console_color(11);
    kputs("  ------------------------------------------------------------------------------\n");
    console_color(15);
    console_set_header_end();
}

static const char *help_name(const char *choice) {
    if (!choice) return 0;
    if (!strcmp(choice, "1")) return "calculator";
    if (!strcmp(choice, "2")) return "system";
    if (!strcmp(choice, "3")) return "process";
    if (!strcmp(choice, "4")) return "memory";
    if (!strcmp(choice, "5")) return "filesystem";
    if (!strcmp(choice, "6")) return "security";
    if (!strcmp(choice, "7")) return "demo";
    return choice;
}

static void help_category(const char *choice) {
    const char *t = help_name(choice);
    if (t && !strcmp(t, "calculator")) {
        ui_header("01  NUMBER CONVERSION", "Convert values between common number systems");
        ui_row("convert", "interactive conversion or direct command");
        ui_usage("convert 2A hex dec");
        ui_info("Bases: 2 binary, 8 octal, 10 decimal, 16 hexadecimal");
        return;
    }
    if (t && !strcmp(t, "system")) {
        ui_header("02  SYSTEM", "General operating system information");
        ui_row("about", "show system and kernel information");
        ui_row("clear", "clear the screen and redraw the header");
        ui_row("echo <text>", "print a line of text");
        ui_row("uptime", "show time since startup");
        ui_row("reboot", "restart the virtual machine");
        ui_row("halt", "stop the virtual machine");
        return;
    }
    if (t && !strcmp(t, "process")) {
        ui_header("03  PROCESSES", "Threads and CPU scheduling");
        ui_row("ps", "list threads and their current states");
        ui_row("threads", "run the thread lifecycle example");
        ui_row("sched", "show scheduler settings and context switches");
        ui_row("trace on|off", "enable or disable thread state messages");
        ui_row("kill <tid>", "terminate a thread by its ID");
        return;
    }
    if (t && !strcmp(t, "memory")) {
        ui_header("04  MEMORY", "Kernel heap management");
        ui_row("meminfo", "show heap usage and allocation statistics");
        ui_row("alloc <bytes>", "allocate memory and receive a handle");
        ui_row("free <handle>", "release a previously allocated handle");
        ui_row("memtest", "run the out-of-memory safety test");
        return;
    }
    if (t && !strcmp(t, "filesystem")) {
        ui_header("05  FILE SYSTEM", "RAM-based file storage");
        ui_row("ls", "list files and their permissions");
        ui_row("touch <file>", "create an empty file");
        ui_row("write <file> <text>", "replace file contents");
        ui_row("append <file> <text>", "add text to a file");
        ui_row("cat <file>", "display file contents");
        ui_row("rm <file>", "delete a file");
        return;
    }
    if (t && !strcmp(t, "security")) {
        ui_header("06  SECURITY", "Users and file permissions");
        ui_row("whoami", "show the current user");
        ui_row("su <user> [pass]", "switch between guest and root");
        ui_row("chmod <OW> <file>", "set owner and other permissions");
        ui_info("Permission values: 4 = read, 2 = write, 6 = read/write, 0 = none");
        return;
    }
    if (t && !strcmp(t, "demo")) {
        ui_header("07  DEMONSTRATIONS", "Concurrency and synchronization examples");
        ui_row("race", "compare an unsafe counter with mutex protection");
        ui_row("deadlock", "create and detect a circular wait");
        ui_row("deadlock fix", "run the deadlock prevention example");
        return;
    }
    if (choice) {
        ui_error("Unknown section. Choose a number from 1 to 7.");
        return;
    }
    ui_header("QUANTUM OS  /  MAIN MENU", "Enter 'help <number>' to open a section");
    ui_section("01", "NUMBER CONVERSION");
    ui_row("convert", "convert binary, octal, decimal and hexadecimal");
    ui_section("02", "SYSTEM");
    ui_row("about / uptime / clear", "system information and screen controls");
    ui_row("reboot / halt / echo", "basic system commands");
    ui_section("03", "PROCESSES");
    ui_row("ps / threads / sched", "thread status and CPU scheduling");
    ui_row("trace / kill", "scheduler tracing and thread control");
    ui_section("04", "MEMORY");
    ui_row("meminfo / alloc / free", "inspect and manage kernel heap");
    ui_row("memtest", "memory allocation stress test");
    ui_section("05", "FILE SYSTEM");
    ui_row("ls / touch / write / append", "create and update files");
    ui_row("cat / rm", "read and remove files");
    ui_section("06", "SECURITY");
    ui_row("whoami / su / chmod", "user accounts and permissions");
    ui_section("07", "DEMONSTRATIONS");
    ui_row("race / deadlock", "race condition, mutex and deadlock");
    ui_rule();
    console_color(7);
    kputs("  Examples: help 3   |   convert 2A hex dec   |   about\n");
    console_color(15);
}

static int split(char *line, char **argv, int max) {
    int n = 0;
    while (*line && n < max) {
        while (*line == ' ') *line++ = 0;
        if (!*line) break;
        argv[n++] = line;
        while (*line && *line != ' ') line++;
    }
    return n;
}

static const char *rest(const char *line_orig, int skip) {      /* ambil sisa baris setelah 'skip' kata */
    const char *p = line_orig;
    for (int i = 0; i < skip; i++) { while (*p == ' ') p++; while (*p && *p != ' ') p++; }
    while (*p == ' ') p++;
    return p;
}

static int parse_u32_arg(const char *s, u32 *value) {
    u32 n = 0;
    if (!s || !*s) return 0;
    for (; *s; s++) {
        if (*s < '0' || *s > '9') return 0;
        u32 d = (u32)(*s - '0');
        if (n > (0xFFFFFFFFU - d) / 10U) return 0;
        n = n * 10U + d;
    }
    *value = n;
    return 1;
}

static void reboot(void) { outb(0x64, 0xFE); for (;;) __asm__ volatile("hlt"); }

static void report(int r) { if (r < 0) kprintf("Error: %s\n", fs_err(r)); }

static int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int number_base(const char *s) {
    if (!strcmp(s, "bin") || !strcmp(s, "binary") || !strcmp(s, "biner") || !strcmp(s, "2")) return 2;
    if (!strcmp(s, "oct") || !strcmp(s, "octal") || !strcmp(s, "oktal") || !strcmp(s, "8")) return 8;
    if (!strcmp(s, "dec") || !strcmp(s, "decimal") || !strcmp(s, "desimal") || !strcmp(s, "10")) return 10;
    if (!strcmp(s, "hex") || !strcmp(s, "hexadecimal") || !strcmp(s, "heksadesimal") || !strcmp(s, "16")) return 16;
    return 0;
}

static int parse_number(const char *s, int requested_base, u32 *value, int *negative) {
    int sign = 0;
    if (*s == '-' || *s == '+') { sign = (*s == '-'); s++; }
    if (!*s) return 0;

    int prefix_base = 0;
    if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) prefix_base = 2;
    else if (s[0] == '0' && (s[1] == 'o' || s[1] == 'O')) prefix_base = 8;
    else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) prefix_base = 16;
    if (prefix_base) {
        if (requested_base && requested_base != prefix_base) return 0;
        requested_base = prefix_base;
        s += 2;
    }
    int base = requested_base ? requested_base : 10;
    if (!*s) return 0;

    u32 n = 0;
    for (; *s; s++) {
        int d = digit_value(*s);
        if (d < 0 || d >= base || n > (0xFFFFFFFFU - (u32)d) / (u32)base) return 0;
        n = n * (u32)base + (u32)d;
    }
    if (sign && n > 0x80000000U) return 0;
    *value = n;
    *negative = sign;
    return 1;
}

static const char *base_label(int base) {
    switch (base) {
    case 2: return "BIN";
    case 8: return "OCT";
    case 10: return "DEC";
    default: return "HEX";
    }
}

static void format_number(u32 value, int negative, int base, char *out) {
    static const char digits[] = "0123456789ABCDEF";
    char reverse[32];
    int n = 0, pos = 0;
    do { reverse[n++] = digits[value % (u32)base]; value /= (u32)base; } while (value);
    if (negative) out[pos++] = '-';
    while (n) out[pos++] = reverse[--n];
    out[pos] = 0;
}

static void show_all_bases(u32 value, int negative) {
    char bin[35], oct[35], dec[35], hex[35];
    format_number(value, negative, 2, bin);
    format_number(value, negative, 8, oct);
    format_number(value, negative, 10, dec);
    format_number(value, negative, 16, hex);
    kprintf("BIN: %s\nOCT: %s\nDEC: %s\nHEX: %s\n", bin, oct, dec, hex);
}

static void show_conversion(const char *number, int source, int target) {
    u32 value; int negative;
    if (!parse_number(number, source, &value, &negative)) {
        kputs("Invalid number or value outside the 32-bit range\n");
        return;
    }
    if (target) {
        char result[35];
        format_number(value, negative, target, result);
        console_color(11); kprintf("%s: ", base_label(target));
        console_color(15); kprintf("%s\n", result);
    } else show_all_bases(value, negative);
}

static void ask_conversion_field(const char *prompt, char *value, int max) {
    console_color(11); kputs(prompt);
    console_color(15); readline(value, max);
}

static void draw_base_buttons(const char *label, int *row, int *start) {
    console_color(11); kprintf("%s: ", label);
    console_get_cursor(start, row);
    console_color(14); kputs("[ BIN ]  [ OCT ]  [ DEC ]  [ HEX ]\n");
    console_color(15);
}

static int base_from_button(int x, int y, int row, int start) {
    static const int values[4] = { 2, 8, 10, 16 };
    if (y != row) return 0;
    for (int i = 0; i < 4; i++)
        if (x >= start + i * 9 && x < start + i * 9 + 7) return values[i];
    return 0;
}

static int wait_for_base_button(int row, int start) {
    static const int values[4] = { 2, 8, 10, 16 };
    for (;;) {
        int x, y;
        if (console_get_mouse_click(&x, &y)) {
            int base = base_from_button(x, y, row, start);
            if (base) return base;
        }
        int key = console_getc_nb();
        if (key >= '1' && key <= '4') {
            kprintf("%c\n", key);
            return values[key - '1'];
        }
        if (key == 27) return 0;
        thread_yield();
        if (!others_ready()) __asm__ volatile("sti; hlt");
    }
}

static void convert_number(int ac, char **av) {
    char number[64];
    int source = 0, target = 0;
    const char *input;

    if (ac == 1) {
        ask_conversion_field("Number: ", number, sizeof(number));
        if (!number[0]) { kputs("Conversion cancelled\n"); return; }

        console_color(7);
        kputs("Select a base with 1-4 or click an option in the QEMU window.\n");
        int source_row, source_x;
        draw_base_buttons("FROM", &source_row, &source_x);
        source = wait_for_base_button(source_row, source_x);
        if (!source) { kputs("konversi dibatalkan\n"); return; }

        int target_row, target_x;
        draw_base_buttons("TO", &target_row, &target_x);
        target = wait_for_base_button(target_row, target_x);
        if (!target) { kputs("konversi dibatalkan\n"); return; }
        input = number;
    } else if (ac >= 2 && ac <= 4) {
        input = av[1];
        if (ac >= 3) source = number_base(av[2]);
        if (ac == 4) target = number_base(av[3]);
        if ((ac >= 3 && !source) || (ac == 4 && !target)) {
            kputs("Unknown base. Use bin, oct, dec or hex\n");
            return;
        }
    } else {
        kputs("Usage: convert [number [source [target]]]\n");
        return;
    }

    show_conversion(input, source, target);
}

void shell_run(void) {
    char line[128], orig[128];
    char *av[8];

    for (;;) {
        /* ================================================================
           PROMPT
           ================================================================ */
        console_color(cur_uid == UID_ROOT ? 12 : 10);
        kprintf("%s@quantumos:~%c ", uid_name(cur_uid),
                cur_uid == UID_ROOT ? "#" : "$");
        console_color(15);

        readline(line, sizeof(line));
        strncpy(orig, line, sizeof(orig) - 1);
        orig[sizeof(orig) - 1] = 0;

        int ac = split(line, av, 8);
        if (!ac) continue;

        const char *c = av[0];

        /* ================================================================
           01. CALCULATOR
           ================================================================ */
        if (!strcmp(c, "convert")) {
            convert_number(ac, av);

        /* ================================================================
           02. SYSTEM
           ================================================================ */
        } else if (!strcmp(c, "help") || !strcmp(c, "menu")) {
            help_category(ac > 1 ? av[1] : 0);

        } else if (!strcmp(c, "about")) {
            ui_header("SYSTEM INFORMATION", "QuantumOS kernel profile");
            ui_row("OS", "QuantumOS v1.0");
            ui_row("Type", "Specialized Educational Operating System");
            ui_row("Interface", "Command-line shell");
            ui_row("CPU", "i386 protected mode");
            ui_row("Scheduler", "Preemptive Round Robin");
            ui_row("Quantum", "3 ticks / 30 ms");
            ui_row("Timer", "PIT 100 Hz");
            ui_row("Heap", "512 KB first-fit allocator");
            ui_row("Filesystem", "RAMFS");
            ui_row("Access", "root / guest");
            ui_ok("Kernel initialization completed.");

        } else if (!strcmp(c, "clear")) {
            console_clear();

        } else if (!strcmp(c, "echo")) {
            kprintf("%s\n", rest(orig, 1));

        } else if (!strcmp(c, "uptime")) {
            kprintf("Uptime: %u.%02u seconds (%u ticks)\n",
                    ticks / TICK_HZ, ticks % TICK_HZ, ticks);

        } else if (!strcmp(c, "reboot")) {
            reboot();

        } else if (!strcmp(c, "halt")) {
            kprintf("System halted.\n");
            for (;;) __asm__ volatile("cli; hlt");

        /* ================================================================
           03. PROCESS / SCHEDULER
           ================================================================ */
        } else if (!strcmp(c, "ps")) {
            threads_dump();

        } else if (!strcmp(c, "threads")) {
            demo_threads();

        } else if (!strcmp(c, "sched")) {
            kprintf("Scheduler : Preemptive Round Robin\n");
            kprintf("Quantum   : %d tick (%d ms)\n",
                    QUANTUM, QUANTUM * 1000 / TICK_HZ);
            kprintf("Timer     : PIT %d Hz\n", TICK_HZ);
            kprintf("Switches  : %u\n", ctx_switches);

        } else if (!strcmp(c, "trace")) {
            if (ac > 1) {
                if (!strcmp(av[1], "on")) trace_on = 1;
                else if (!strcmp(av[1], "off")) trace_on = 0;
                else { kputs("Usage: trace on|off\n"); continue; }
            }
            kprintf("Thread trace: %s\n", trace_on ? "ON" : "OFF");

        } else if (!strcmp(c, "kill")) {
            if (ac < 2) {
                kprintf("Usage: kill <tid>\n");
            } else {
                kprintf(thread_kill(atoi(av[1]))
                    ? "Invalid TID.\n"
                    : "Thread terminated.\n");
            }

        /* ================================================================
           04. MEMORY
           ================================================================ */
        } else if (!strcmp(c, "meminfo")) {
            struct meminfo m;
            mem_info(&m);
            kprintf("Heap total : %u B\n", m.total);
            kprintf("Used       : %u B\n", m.used);
            kprintf("Available  : %u B\n", m.free);
            kprintf("Blocks     : %u\n", m.nblocks);
            kprintf("Allocations: %u\n", m.nalloc);
            kprintf("Frees      : %u\n", m.nfree);
            kprintf("OOM fails  : %u\n", m.nfail);

        } else if (!strcmp(c, "alloc")) {
            if (ac < 2) {
                kprintf("Usage: alloc <byte>\n");
                continue;
            }

            u32 n;
            if (!parse_u32_arg(av[1], &n) || n == 0) {
                kputs("Invalid size. Enter a positive decimal byte count.\n");
                continue;
            }
            int id = -1;
            for (int i = 0; i < 16; i++)
                if (!h_ptr[i]) {
                    id = i;
                    break;
                }

            void *p = id >= 0 ? kmalloc(n) : 0;
            if (!p) {
                kprintf("Allocation failed: %s\n",
                        id < 0 ? "handle table full"
                               : "Out of Memory (kernel safe)");
            } else {
                h_ptr[id] = p;
                h_sz[id] = n;
                kprintf("Handle %d: %u byte @ %p\n", id, n, p);
            }

        } else if (!strcmp(c, "free")) {
            u32 parsed_id;
            int id = (ac > 1 && parse_u32_arg(av[1], &parsed_id) && parsed_id < 16)
                     ? (int)parsed_id : -1;
            if (id < 0 || !h_ptr[id]) {
                kprintf("Invalid handle.\n");
            } else {
                kfree(h_ptr[id]);
                h_ptr[id] = 0;
                kprintf("Handle %d released (%u byte)\n", id, h_sz[id]);
            }

        } else if (!strcmp(c, "memtest")) {
            demo_memtest();

        /* ================================================================
           05. FILESYSTEM
           ================================================================ */
        } else if (!strcmp(c, "ls")) {
            fs_list();

        } else if (!strcmp(c, "touch")) {
            if (ac < 2)
                kprintf("Usage: touch <file>\n");
            else
                report(fs_create(av[1]));

        } else if (!strcmp(c, "write") || !strcmp(c, "append")) {
            if (ac < 3) {
                kprintf("Usage: %s <file> <text>\n", c);
                continue;
            }

            int r = fs_write(av[1], rest(orig, 2), c[0] == 'a');
            report(r);
            if (!r) kprintf("OK\n");

        } else if (!strcmp(c, "cat")) {
            if (ac < 2) {
                kprintf("Usage: cat <file>\n");
                continue;
            }

            const char *d;
            u32 sz;
            int r = fs_read(av[1], &d, &sz);

            if (r < 0) {
                report(r);
            } else {
                kputs(d);
                if (!sz || d[sz - 1] != '\n')
                    kputc('\n');
            }

        } else if (!strcmp(c, "rm")) {
            if (ac < 2) {
                kprintf("Usage: rm <file>\n");
            } else {
                int r = fs_delete(av[1]);
                report(r);
                if (!r) kprintf("OK\n");
            }

        /* ================================================================
           06. SECURITY
           ================================================================ */
        } else if (!strcmp(c, "whoami")) {
            kprintf("%s (uid %d)\n", uid_name(cur_uid), cur_uid);

        } else if (!strcmp(c, "su")) {
            if (ac < 2) {
                kprintf("Usage: su <root|guest> [password]\n");
                continue;
            }

            int r = user_switch(av[1], ac > 2 ? av[2] : 0);
            if (r < 0)
                kprintf("su: failed (%s)\n",
                        r == E_PERM ? "wrong password" : "unknown user");
            else
                kprintf("Current user: %s\n", uid_name(cur_uid));

        } else if (!strcmp(c, "chmod")) {
            if (ac < 3 || strlen(av[1]) != 2) {
                kprintf("Usage: chmod <OW> <file>  (example: chmod 64 a.txt)\n");
                continue;
            }

            if (av[1][0] < '0' || av[1][0] > '7' ||
                av[1][1] < '0' || av[1][1] > '7') {
                kputs("Permission values must be between 0 and 7.\n");
                continue;
            }
            int r = fs_chmod(av[2], av[1][0] - '0', av[1][1] - '0');
            report(r);
            if (!r) kprintf("OK\n");

        /* ================================================================
           07. KERNEL DEMONSTRATIONS
           ================================================================ */
        } else if (!strcmp(c, "race")) {
            demo_race();

        } else if (!strcmp(c, "deadlock")) {
            demo_deadlock(ac > 1 && !strcmp(av[1], "fix"));

        /* ================================================================
           UNKNOWN COMMAND
           ================================================================ */
        } else {
            console_color(12);
            kprintf("Unknown command: '%s'\n", c);
            console_color(15);
            kputs("Type 'help' to see available commands.\n");
        }
    }
}

