# QuantumOS build configuration
# Target: freestanding 32-bit i386 kernel

CC      = gcc
AS      = gcc
LD      = ld

CFLAGS  = -m32 -std=gnu11 -ffreestanding -fno-pie -fno-stack-protector \
          -fno-builtin -mno-sse -mno-mmx -mno-80387 -O1 -Wall -Wextra -Isrc
ASFLAGS = -m32
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib -z noexecstack

SRC_C   = $(wildcard src/*.c)
SRC_S   = $(wildcard src/*.S)
OBJ     = $(patsubst src/%.c,build/%.o,$(SRC_C)) \
          $(patsubst src/%.S,build/%.o,$(SRC_S))
KERNEL  = build/quantum.elf

.PHONY: all run run-serial iso run-iso debug clean info

all: $(KERNEL)

info:
	@echo "Target: 32-bit i386 freestanding kernel"
	@echo "Output: $(KERNEL)"
	@echo "Run:    make run"

build/%.o: src/%.c src/kernel.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.S | build
	$(AS) $(ASFLAGS) -c $< -o $@

$(KERNEL): $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

build:
	mkdir -p build

# Run with a QEMU window and serial output.
run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 64M -serial stdio

# Run without a graphical window.
run-serial: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 64M -nographic

# Build a bootable GRUB ISO.
iso: $(KERNEL)
	mkdir -p build/iso/boot/grub
	cp $(KERNEL) build/iso/boot/quantum.elf
	printf 'set timeout=0\nset default=0\nmenuentry "QuantumOS" {\n  multiboot /boot/quantum.elf\n  boot\n}\n' > build/iso/boot/grub/grub.cfg
	grub-mkrescue -o build/quantum.iso build/iso

run-iso: iso
	qemu-system-i386 -cdrom build/quantum.iso -m 64M -serial stdio

# Start QEMU paused and wait for GDB on port 1234.
debug: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 64M -serial stdio -s -S

clean:
	rm -rf build
