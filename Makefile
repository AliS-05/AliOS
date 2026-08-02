AS = nasm
CXX = gcc
LD = ld

SRC_DIR = src
HEADER_DIR = include
BUILD_DIR = build

# This finds every .c file in src/ and any sub-directory (like fs/ or assembler/)
C_SOURCES := $(shell find $(SRC_DIR) -name "*.c")

C_OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))

BINARY = $(BUILD_DIR)/os.bin
DRIVE = $(SRC_DIR)/fs/disk.img
BOOT = $(SRC_DIR)/superboot.s
KERNEL_ASM = $(SRC_DIR)/kernel_asm.s

BOOT_BIN = $(BUILD_DIR)/superboot.bin
KERNEL_ASM_OBJ = $(BUILD_DIR)/kernel_asm.o
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

# must match KERNEL_SECTORS in superboot.s
KERNEL_SECTORS = 384
IMAGE_BYTES = 197120

# CXXFLAGS = -m32 -ffreestanding \
#      -nostdlib -fno-builtin -fno-pic -fno-stack-protector \
#      -Wall -Wextra -O0 -I$(HEADER_DIR)
# 
CXXFLAGS = -m32 -ffreestanding -ggdb3 \
	   -nostdlib -fno-builtin -fno-pic -fno-stack-protector \
	   -Wall -Wextra -O0 -I$(HEADER_DIR)

LDFLAGS = -m elf_i386 -T linker.ld

all: $(BINARY)

  # cat binaries to single .bin file (needed because different origin points)
$(BINARY): $(BOOT_BIN) $(KERNEL_BIN)
	@if [ $$(stat -c%s $(KERNEL_BIN)) -gt $$(( $(KERNEL_SECTORS) * 512 )) ]; then \
                echo "kernel.bin is $$(stat -c%s $(KERNEL_BIN)) bytes, bootloader only reads $$(( $(KERNEL_SECTORS) * 512 ))"; \
                exit 1; \
        fi
	cat $^ > $@
	truncate -s $(IMAGE_BYTES) $@
	@echo "os.bin padded to $$(stat -c%s $@) bytes"

$(KERNEL_ELF): $(KERNEL_ASM_OBJ) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

$(BOOT_BIN): $(BOOT)
	@mkdir -p $(BUILD_DIR)
	$(AS) -f bin $< -o $@

$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	@mkdir -p $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) --std=c99 -c $< -o $@

run: $(BINARY)
	qemu-system-x86_64 -drive format=raw,file=$(BINARY) -drive format=raw,file=$(DRIVE) \
		-netdev tap,id=net0,ifname=tap0,script=no,downscript=no -device e1000,netdev=net0 \
		-object filter-dump,id=dump0,netdev=net0,file=packets.pcap

# gdb debug session: qemu halts at reset with the gdbstub on :1234, gdb attaches with symbols
# override the initial breakpoint:  make debug BRK=handle_udp
BRK ?= parse_packet

debug: $(BINARY)
	qemu-system-i386 -s -S -drive format=raw,file=$(BINARY) -drive format=raw,file=$(DRIVE) \
		-netdev tap,id=net0,ifname=tap0,script=no,downscript=no -device e1000,netdev=net0 \
		-object filter-dump,id=dump0,netdev=net0,file=packets.pcap & \
		QEMU_PID=$$!; \
		gdb -q \
		-ex "set confirm off" \
		-ex "set pagination off" \
		-ex "set disassembly-flavor intel" \
		-ex "set architecture i386" \
		-ex "symbol-file $(KERNEL_ELF)" \
		-ex "target remote :1234" \
		-ex "break $(BRK)" \
		-ex "continue"; \
		kill $$QEMU_PID 2>/dev/null

clean:
	rm -rf $(BUILD_DIR)
