AS = nasm
CXX = g++
LD = ld

SRC_DIR = ../src
BUILD_DIR = .
VPATH := $(SRC_DIR) $(SRC_DIR)/fs

BINARY = $(BUILD_DIR)/os.bin
DRIVE = $(SRC_DIR)/fs/disk.img
BOOT = $(SRC_DIR)/superboot.s
KERNEL_ASM = $(SRC_DIR)/kernel_asm.s

BOOT_BIN = $(BUILD_DIR)/superboot.bin
KERNEL_ASM_OBJ = $(BUILD_DIR)/kernel_asm.o
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

CPP_SOURCES := $(wildcard $(SRC_DIR)/*.cpp) \
	       $(wildcard $(SRC_DIR)/fs/*.cpp)

CPP_OBJECTS := $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

CXXFLAGS = -m32 -ffreestanding -fno-exceptions -fno-rtti \
           -nostdlib -fno-builtin -fno-pic -fno-stack-protector \
           -Wall -Wextra -O0 -I$(SRC_DIR)

LDFLAGS = -m elf_i386 -T linker.ld

all: $(BINARY)

# Build bootloader
$(BOOT_BIN): $(BOOT)
	$(AS) -f bin $(BOOT) -o $(BOOT_BIN)

# Assemble kernel assembly
$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	$(AS) -f elf32 $(KERNEL_ASM) -o $(KERNEL_ASM_OBJ)

# Link kernel
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(KERNEL_ELF): $(KERNEL_ASM_OBJ) $(CPP_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Extract kernel binary
$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $< $@

# Combine bootloader and kernel
$(BINARY): $(BOOT_BIN) $(KERNEL_BIN)
	cat $^ > $@

run: $(BINARY)
	qemu-system-x86_64 \
		-drive format=raw,file=$(BINARY) \
		-drive format=raw,file=$(DRIVE)

debug: $(BINARY)
	qemu-system-x86_64 -s -S -drive format=raw,file=$(BINARY)

clean:
	rm -rf $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.elf $(BUILD_DIR)/*.o $(BUILD_DIR)/fs
.PHONY: all run debug clean
