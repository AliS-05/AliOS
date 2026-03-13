AS = nasm
CXX = gcc
LD = ld

SRC_DIR = src
HEADER_DIR = include
BUILD_DIR = build

# 1. FIND ALL SOURCES RECURSIVELY
# This finds every .cpp file in src/ and any sub-directory (like fs/)
C_SOURCES := $(shell find $(SRC_DIR) -name "*.c")

# 2. MAP SOURCES TO OBJECTS IN BUILD DIR
# This converts src/fs/ata.cpp -> build/fs/ata.o
C_OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))

BINARY = $(BUILD_DIR)/os.bin
DRIVE = $(SRC_DIR)/fs/disk.img
BOOT = $(SRC_DIR)/superboot.s
KERNEL_ASM = $(SRC_DIR)/kernel_asm.s

BOOT_BIN = $(BUILD_DIR)/superboot.bin
KERNEL_ASM_OBJ = $(BUILD_DIR)/kernel_asm.o
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

CXXFLAGS = -m32 -ffreestanding \
           -nostdlib -fno-builtin -fno-pic -fno-stack-protector \
           -Wall -Wextra -O0 -I$(HEADER_DIR)

LDFLAGS = -m elf_i386 -T linker.ld

all: $(BINARY)

# Combine bootloader and kernel
$(BINARY): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@

# Link kernel (Uses all discovered CPP objects + asm object)
$(KERNEL_ELF): $(KERNEL_ASM_OBJ) $(C_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Extract kernel binary
$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

# Build bootloader
$(BOOT_BIN): $(BOOT)
	@mkdir -p $(BUILD_DIR)
	$(AS) -f bin $< -o $@

# Assemble kernel assembly
$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	@mkdir -p $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

# THE MAGIC RULE: Compiles any .cpp in any subfolder
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) --std=c99 -c $< -o $@

run: $(BINARY)
	qemu-system-x86_64 -drive format=raw,file=$(BINARY) -drive format=raw,file=$(DRIVE)

clean:
	rm $(BUILD_DIR)/*.o $(BUILD_DIR)/*.bin 
	rm -rf $(BUILD_DIR)/fs
 
.PHONY: all run clean
