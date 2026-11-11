# QuantumOS

QuantumOS is a small 32-bit x86 (i386) educational kernel written in C and GNU
assembler. It boots in QEMU and demonstrates core operating-system concepts
through a VGA text console and command-line shell.

## Features

- 32-bit protected-mode startup with a flat GDT and IDT
- Programmable Interval Timer (PIT) at 100 Hz
- Preemptive Round-Robin thread scheduler
- Mutexes, semaphores, and wait-for-cycle deadlock detection
- 512 KiB first-fit kernel heap with block splitting and coalescing
- RAM-based file system with basic root/guest access permissions
- VGA text output, serial console, PS/2 keyboard input, and command history
- Number conversion utility and kernel demonstrations

## Requirements

On Ubuntu or WSL2 Ubuntu, install the toolchain:

\`\`\`bash
sudo apt update
sudo apt install -y build-essential gcc-multilib qemu-system-x86 grub-pc-bin grub-common xorriso mtools gdb
\`\`\`

## Build and run

Run these commands from the directory containing this README and the Makefile:

| Command | Description |
|---|---|
| \`make\` | Build the kernel at \`build/quantum.elf\` |
| \`make run\` | Run the kernel in a QEMU window with serial output in the terminal |
| \`make run-serial\` | Run QEMU in text-only mode |
| \`make iso\` | Create a bootable GRUB ISO |
| \`make run-iso\` | Boot the ISO in QEMU |
| \`make debug\` | Start QEMU paused for a GDB connection |
| \`make clean\` | Remove generated build files |

To rebuild from a clean state:

\`\`\`bash
make clean
make
make run
\`\`\`

## First commands

At the \`guest@quantumos\` prompt, enter:

\`\`\`text
help
help 1
about
uptime
\`\`\`

The numbered help sections are:

1. Number conversion
2. System information
3. Processes and scheduling
4. Memory management
5. File system
6. Security and permissions
7. Kernel demonstrations

## Demonstration checklist

\`\`\`text
help
threads
sched
ps
race
deadlock
deadlock fix
meminfo
alloc 1000
free 0
memtest
touch notes.txt
write notes.txt Hello QuantumOS
cat notes.txt
append notes.txt  - updated
cat notes.txt
ls
\`\`\`

The file system starts with \`readme.txt\` and \`secret.txt\`. The default account
is \`guest\`. The demo root password is \`toor\`; this is a teaching example only,
not real authentication or production-grade security.

## Source layout

| File | Responsibility |
|---|---|
| \`src/boot.S\` | Multiboot header, entry point, bootstrap stack |
| \`src/main.c\` | Kernel initialization sequence |
| \`src/console.c\` | VGA, serial, keyboard, mouse polling, line editor |
| \`src/shell.c\` | Startup screen, help menu, command interpreter |
| \`src/interrupts.c\`, \`src/isr.S\` | GDT, IDT, PIC, PIT, interrupt stubs |
| \`src/thread.c\` | Thread lifecycle and Round-Robin scheduler |
| \`src/sync.c\` | Mutexes, semaphores, deadlock detection |
| \`src/mem.c\` | Kernel heap allocator |
| \`src/fs.c\` | RAM file system and access permissions |
| \`src/demo.c\` | Thread, race-condition, deadlock, and memory demos |
| \`src/string.c\` | Minimal freestanding string and memory routines |
| \`src/kernel.h\` | Shared types, constants, and subsystem interfaces |

## Notes

QuantumOS is a teaching kernel. Its root password, permission model, memory
management, and device handling are intentionally small demonstrations and
must not be treated as security or reliability guarantees for real systems.
