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
    console_color(11);
    kputs("\n  QUANTUM OS  v1.0\n");
    console_color(8);
    kputs("  --------------------------------------------------------------\n");
    console_color(7);
    kputs("  32-bit educational operating system\n");
    console_color(10);
    kputs("  CPU  OK   TIMER  OK   MEMORY  OK   THREADS  OK   RAMFS  OK\n");
    console_color(11);
    kputs("\n  MAIN MENU\n");
    console_color(14);
    kputs("  1  Convert numbers       2  System information\n");
    kputs("  3  Processes & CPU       4  Memory management\n");
    kputs("  5  Files                 6  Users & permissions\n");
    kputs("  7  Demonstrations\n");
    console_color(7);
    kputs("\n  Type a number (1-7), 'help', or a command.\n");
    console_color(8);
    kputs("  --------------------------------------------------------------\n");
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
        ui_header("01  NUMBER CONVERTER", "Change a value from one number base to another");
        ui_row("convert", "start the guided converter");
        ui_usage("convert 2A hex dec");
        ui_info("Bases: bin = 2, oct = 8, dec = 10, hex = 16");
        ui_info("Example: 2A in hex is 42 in decimal.");
        return;
    }
    if (t && !strcmp(t, "system")) {
        ui_header("02  SYSTEM", "Basic system commands");
        ui_row("about", "show OS and hardware configuration");
        ui_row("uptime", "show time since boot");
        ui_row("clear", "clear the screen");
        ui_row("echo <text>", "print text");
        ui_row("reboot / halt", "restart / stop the virtual machine");
        return;
    }
    if (t && !strcmp(t, "process")) {
        ui_header("03  PROCESSES & CPU", "View and control kernel threads");
        ui_row("ps", "list threads and their states");
        ui_row("threads", "run a thread scheduling example");
        ui_row("sched", "show CPU scheduling settings");
        ui_row("trace on|off", "show or hide thread trace messages");
        ui_row("kill <id>", "stop a thread using its ID");
        ui_info("A thread is a unit of work scheduled by the CPU.");
        return;
    }
    if (t && !strcmp(t, "memory")) {
        ui_header("04  MEMORY", "Manage temporary kernel memory");
        ui_row("meminfo", "show heap capacity and usage");
        ui_row("alloc <bytes>", "reserve memory; returns a handle");
        ui_row("free <handle>", "release memory using its handle");
        ui_row("memtest", "run the memory stress test");
        ui_info("Example: alloc 128, then free 0 (if handle 0 was returned).");
        return;
    }
    if (t && !strcmp(t, "filesystem")) {
        ui_header("05  FILES", "Create, read, update and delete RAM files");
        ui_row("ls", "list stored files");
        ui_row("touch <name>", "create an empty file");
        ui_row("write <name> <text>", "replace file contents");
        ui_row("append <name> <text>", "add text at the end");
        ui_row("cat <name>", "display file contents");
        ui_row("rm <name>", "delete a file");
        ui_info("Files are stored in RAM and reset when the OS restarts.");
        return;
    }
    if (t && !strcmp(t, "security")) {
        ui_header("06  USERS & PERMISSIONS", "Basic account and file access controls");
        ui_row("whoami", "show the current account");
        ui_row("su guest", "switch to the guest account");
        ui_row("su root <password>", "switch to the root account");
        ui_row("chmod <OW> <file>", "set owner and other access levels");
        ui_info("Permission digits: 4 read, 2 write, 1 execute; add them together.");
        ui_info("Example: 6 means read + write. This is a teaching model.");
        return;
    }
    if (t && !strcmp(t, "demo")) {
        ui_header("07  DEMONSTRATIONS", "See operating-system concepts in action");
        ui_row("race", "compare unsafe and protected shared data");
        ui_row("deadlock", "create and detect a circular wait");
        ui_row("deadlock fix", "show a deadlock prevention strategy");
        return;
    }
    if (choice) {
        ui_error("Unknown section. Enter a number from 1 to 7.");
        return;
    }
    ui_header("HELP / COMMAND MENU", "Enter 1-7 for details, or run a command");
    ui_row("1", "number conversion");
    ui_row("2", "system information");
    ui_row("3", "processes and CPU scheduling");
    ui_row("4", "memory management");
    ui_row("5", "file storage");
    ui_row("6", "users and permissions");
    ui_row("7", "operating-system demonstrations");
    ui_rule();
    ui_usage("convert 2A hex dec");
    ui_usage("touch notes.txt");
    ui_usage("write notes.txt Hello");
    ui_info("Tip: type 'help 5' to see file commands and examples.");
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

static int ask_base(const char *step) {
    char choice[8];
    u32 selected;
    console_color(11);
    kprintf("\n%s\n", step);
    console_color(14);
    kputs("  1  BIN (binary)       2  OCT (octal)\n");
    kputs("  3  DEC (decimal)      4  HEX (hexadecimal)\n");
    console_color(7);
    kputs("  Enter 1-4, or 0 to cancel: ");
    console_color(15);
    readline(choice, sizeof(choice));
    if (!parse_u32_arg(choice, &selected) || selected > 4) return 0;
    if (selected == 0) return -1;
    return selected == 1 ? 2 : selected == 2 ? 8 : selected == 3 ? 10 : 16;
}

static void convert_number(int ac, char **av) {
    char number[64];
    int source = 0, target = 0;
    const char *input;

    if (ac == 1) {
        ui_header("NUMBER CONVERTER", "Follow the three prompts below");
        console_color(7);
        kputs("  Step 1 of 3 - enter the number you want to convert.\n");
        console_color(11);
        kputs("  Number: ");
        console_color(15);
        readline(number, sizeof(number));
        if (!number[0]) { ui_info("Conversion cancelled."); return; }

        source = ask_base("Step 2 of 3 - what base is your number written in?");
        if (source <= 0) { ui_info("Conversion cancelled."); return; }
        target = ask_base("Step 3 of 3 - choose the base you want as the result.");
        if (target <= 0) { ui_info("Conversion cancelled."); return; }
        input = number;
    } else if (ac >= 2 && ac <= 4) {
        input = av[1];
        if (ac >= 3) source = number_base(av[2]);
        if (ac == 4) target = number_base(av[3]);
        if ((ac >= 3 && !source) || (ac == 4 && !target)) {
            ui_error("Unknown base. Use bin, oct, dec or hex.");
            ui_usage("convert 2A hex dec");
            return;
        }
    } else {
        ui_usage("convert [number [source [target]]]");
        return;
    }

    u32 value;
    int negative;
    if (!parse_number(input, source, &value, &negative)) {
        ui_error("Invalid number for the selected base, or value exceeds 32 bits.");
        ui_info("Check the digits: binary uses 0-1, octal 0-7, decimal 0-9, hex 0-9 and A-F.");
        return;
    }

    console_color(11);
    kputs("\n  CONVERSION RESULT\n");
    console_color(8);
    kputs("  --------------------------------------------------------------\n");
    console_color(15);
    if (target) {
        char result[35];
        format_number(value, negative, target, result);
        kprintf("  %s (base %d)  ->  %s (base %d)\n",
                input, source ? source : 10, result, target);
    } else {
        show_all_bases(value, negative);
    }
    console_color(8);
    kputs("  --------------------------------------------------------------\n");
    console_color(7);
    kputs("  Run 'convert' to convert another value.\n");
    console_color(15);
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

        /* Numbered menu shortcuts: typing 1-7 opens the matching help page. */
        if (ac == 1 && av[0][1] == 0 && av[0][0] >= '1' && av[0][0] <= '7') {
            help_category(av[0]);
            continue;
        }

        const char *c = av[0];

        /* ================================================================
           01. CALCULATOR
           ================================================================ */
        if (!strcmp(c, "convert")) {
            convert_number(ac, av);

        /* ================================================================
           02. SYSTEM
           ================================================================ */
        } else if (!strcmp(c, "help") || !strcmp(c, "menu") || !strcmp(c, "?")) {
            help_category(ac > 1 ? av[1] : 0);

        } else if (!strcmp(c, "about")) {
            ui_header("SYSTEM INFORMATION", "QuantumOS kernel profile");
            ui_row("OS", "QuantumOS v1.0");
            ui_row("Type", "Educational 32-bit operating system");
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
                u32 tid;
                if (!parse_u32_arg(av[1], &tid) || tid > 0x7FFFFFFFU) {
                    kputs("Invalid thread ID.\n");
                } else {
                    int result = thread_kill((int)tid);
                    if (result) kputs("Thread not found or not safe to terminate.\n");
                    else kputs("Thread terminated.\n");
                }
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
            kputs("Type 'help' to view the numbered command menu.\n");
        }
    }
}

