/* shell.c - CLI QuantumOS */
#include "kernel.h"

static void *h_ptr[16]; static u32 h_sz[16];

static void draw_quantumos(void) {
    static const char font[9][7][8] = {
        { ".#####.", "##...##", "##...##", "##...##", "##.#.##", "##..###", ".######" }, /* Q */
        { "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", ".#####." }, /* U */
        { "..###..", ".##.##.", "##...##", "##...##", "#######", "##...##", "##...##" }, /* A */
        { "##...##", "###..##", "####.##", "##.####", "##..###", "##...##", "##...##" }, /* N */
        { "#######", "...#...", "...#...", "...#...", "...#...", "...#...", "...#..." }, /* T */
        { "##...##", "###.###", "#######", "##.#.##", "##...##", "##...##", "##...##" }, /* M */
        { ".#####.", "##...##", "##...##", "##...##", "##...##", "##...##", ".#####." }, /* O */
        { ".######", "##.....", "##.....", ".#####.", ".....##", ".....##", "######." }, /* S */
    };
    static const u8 letters[9] = { 0, 1, 2, 3, 4, 1, 5, 6, 7 };
    for (int row = 0; row < 7; row++) {
        kputs("    ");
        for (int letter = 0; letter < 9; letter++) {
            for (int col = 0; col < 7; col++)
                kputc(font[letters[letter]][row][col] == '#' ? '#' : ' ');
            if (letter < 8) kputc(' ');
        }
        kputc('\n');
    }
}

void banner(void) {                                /* identitas dan status awal QuantumOS */
    console_color(11);
    draw_quantumos();

    console_color(9);
    kputs(
"  .--------.        .------( |0> )------.\n"
"--| Q-CORE |--o----(       Q       )----o---[ i386 ]\n"
"  '--------'        '------( |1> )------'\n");

    console_color(11); kputs("SPECIALIZED EDUCATIONAL OPERATING SYSTEM\n");
    console_color(7);  kputs("Number Systems Explorer | BIN <-> OCT <-> DEC <-> HEX\n");
    console_color(15); kprintf("%s v%s | i386 protected mode\n", OS_NAME, OS_VER);

    console_color(10);
    kputs("[ OK ] GDT / IDT       [ OK ] TIMER / PIC\n");
    kputs("[ OK ] MEMORY          [ OK ] SCHEDULER\n");
    kputs("[ OK ] RAM FILESYSTEM  [ OK ] SECURITY\n");

    console_color(14);
    kputs("\nType 'help' to explore the system.\n");
    console_color(8);
    kputs("-------------------------------------------------------------------------------\n");
    console_color(15);
    console_set_header_end();
}

static void row(const char *k, const char *v) {      /* perintah (kuning) + penjelasan (putih) */
    console_color(14); kprintf("%s", k);
    for (int i = (int)strlen(k); i < 20; i++) kputc(' ');
    console_color(15); kprintf("%s\n", v);
}
static void title(const char *t) { console_color(11); kprintf("%s\n", t); console_color(15); }
static void note(const char *t)  { console_color(7);  kprintf("%s\n", t); console_color(15); }

static void help(const char *t) {
    if (t && !strcmp(t, "calculator")) {
        title("[01] CALCULATOR");
        note("Number Systems Explorer");
        row("convert", "konversi BIN, OCT, DEC, dan HEX");
        note("Contoh: convert 2A hex bin");
        note("Interaktif: pilih basis dengan klik atau tombol 1-4.");
    } else if (t && !strcmp(t, "system")) {
        title("[02] SYSTEM");
        row("about", "informasi QuantumOS dan komponen kernel");
        row("clear", "bersihkan layar");
        row("echo <teks>", "tampilkan teks");
        row("uptime", "lihat waktu OS aktif");
        row("reboot", "restart QuantumOS");
        row("halt", "hentikan sistem");
    } else if (t && !strcmp(t, "process")) {
        title("[03] PROCESS");
        note("Threads dan CPU scheduling");
        row("ps", "daftar thread dan status");
        row("threads", "demo thread dan perubahan status");
        row("sched", "informasi Round Robin dan context switch");
        row("trace on|off", "aktifkan atau matikan trace thread");
        row("kill <tid>", "hentikan thread berdasarkan TID");
        note("Urutan demo: threads -> ps -> sched");
    } else if (t && !strcmp(t, "memory")) {
        title("[04] MEMORY");
        note("Kernel heap: 512 KB");
        row("meminfo", "lihat penggunaan dan statistik heap");
        row("alloc <byte>", "alokasikan memori dan dapatkan handle");
        row("free <handle>", "bebaskan memori berdasarkan handle");
        row("memtest", "uji allocator sampai Out of Memory");
    } else if (t && !strcmp(t, "filesystem")) {
        title("[05] FILESYSTEM");
        note("RAMFS: data bersifat sementara");
        row("ls", "daftar file, owner, dan permission");
        row("touch <file>", "buat file kosong");
        row("write <file> <teks>", "tulis atau ganti isi file");
        row("append <file> <teks>", "tambahkan teks ke file");
        row("cat <file>", "baca isi file");
        row("rm <file>", "hapus file");
        note("Contoh: touch a.txt -> write a.txt halo -> cat a.txt");
    } else if (t && !strcmp(t, "security")) {
        title("[06] SECURITY");
        note("Access control: root dan guest");
        row("whoami", "lihat user aktif");
        row("su <user> [pass]", "ganti user");
        row("chmod <OW> <file>", "atur permission owner/other");
        note("Permission: 4=read  2=write  6=read+write  0=none");
        note("Demo: cat secret.txt -> su root toor -> cat secret.txt");
    } else if (t && !strcmp(t, "demo")) {
        title("[07] KERNEL DEMO");
        note("Demonstrasi konsep concurrency dan synchronization");
        row("race", "demo race condition tanpa dan dengan mutex");
        row("deadlock", "demo deadlock dan deteksi");
        row("deadlock fix", "demo pencegahan deadlock dengan lock ordering");
    } else {
        if (t) {
            console_color(12);
            kprintf("Kategori '%s' tidak ditemukan.\n", t);
            console_color(15);
            kputs("\n");
        }

        title("QUANTUMOS COMMAND CENTER");
        note("Specialized Educational Operating System");
        note("Focus: Number Systems Explorer + Low-Level OS Laboratory");
        kputs("\n");

        console_color(11); kputs("[01] CALCULATOR\n"); console_color(15);
        row("convert", "BIN <-> OCT <-> DEC <-> HEX");

        console_color(11); kputs("\n[02] SYSTEM\n"); console_color(15);
        row("about", "OS dan kernel information");
        row("clear", "clear screen");
        row("uptime", "system uptime");
        row("reboot", "restart OS");
        row("halt", "shutdown OS");

        console_color(11); kputs("\n[03] PROCESS\n"); console_color(15);
        row("ps", "list threads");
        row("threads", "thread demonstration");
        row("sched", "scheduler information");

        console_color(11); kputs("\n[04] MEMORY\n"); console_color(15);
        row("meminfo", "memory information");
        row("alloc", "allocate memory");
        row("free", "release memory");
        row("memtest", "Out of Memory demonstration");

        console_color(11); kputs("\n[05] FILESYSTEM\n"); console_color(15);
        row("ls", "list files");
        row("touch", "create file");
        row("write", "write file");
        row("append", "append file");
        row("cat", "read file");
        row("rm", "delete file");

        console_color(11); kputs("\n[06] SECURITY\n"); console_color(15);
        row("whoami", "current user");
        row("su", "switch user");
        row("chmod", "change permissions");

        console_color(11); kputs("\n[07] KERNEL DEMO\n"); console_color(15);
        row("race", "race condition");
        row("deadlock", "deadlock detection");
        row("deadlock fix", "deadlock prevention");

        console_color(8);
        kputs("\n--------------------------------------------------------------------------\n");
        console_color(14);
        kputs("HELP: ");
        console_color(15);
        kputs("help calculator | system | process | memory\n");
        kputs("      help filesystem | security | demo\n");
        console_color(7);
        kputs("Tip: start with 'convert' or 'about'.\n");
        console_color(15);
    }
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
        kputs("angka tidak valid atau melebihi rentang 32-bit\n");
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
        ask_conversion_field("Angka: ", number, sizeof(number));
        if (!number[0]) { kputs("konversi dibatalkan\n"); return; }

        console_color(7);
        kputs("Klik tombol di jendela QEMU; tekan 1-4 sebagai alternatif.\n");
        int source_row, source_x;
        draw_base_buttons("DARI", &source_row, &source_x);
        source = wait_for_base_button(source_row, source_x);
        if (!source) { kputs("konversi dibatalkan\n"); return; }

        int target_row, target_x;
        draw_base_buttons("KE", &target_row, &target_x);
        target = wait_for_base_button(target_row, target_x);
        if (!target) { kputs("konversi dibatalkan\n"); return; }
        input = number;
    } else if (ac >= 2 && ac <= 4) {
        input = av[1];
        if (ac >= 3) source = number_base(av[2]);
        if (ac == 4) target = number_base(av[3]);
        if ((ac >= 3 && !source) || (ac == 4 && !target)) {
            kputs("basis tidak dikenal; gunakan bin, oct, dec, atau hex\n");
            return;
        }
    } else {
        kputs("pakai: convert [angka [asal [tujuan]]]\n");
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
        console_color(cur_uid == 0 ? 12 : 10);
        kprintf("%s@quantumos", uid_name(cur_uid));
        console_color(14);
        kprintf("> ");
        console_color(15);

        readline(line, sizeof(line));
        memcpy(orig, line, sizeof(line));

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
        } else if (!strcmp(c, "help")) {
            help(ac > 1 ? av[1] : 0);

        } else if (!strcmp(c, "about")) {
            kprintf("%s v%s\n", OS_NAME, OS_VER);
            kprintf("Type   : Specialized Educational Operating System\n");
            kprintf("Focus  : Number Systems Explorer\n");
            kprintf("CPU    : i386 protected mode\n");
            kprintf("Sched  : Preemptive Round Robin | quantum %d tick (%d ms)\n",
                    QUANTUM, QUANTUM * 1000 / TICK_HZ);
            kprintf("Memory : 512 KB kernel heap\n");
            kprintf("FS     : RAMFS\n");
            kprintf("Access : root / guest\n");

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
            if (ac > 1)
                trace_on = !strcmp(av[1], "on");
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

            u32 n = (u32)atoi(av[1]);
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
            int id = ac > 1 ? atoi(av[1]) : -1;
            if (id < 0 || id >= 16 || !h_ptr[id]) {
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

