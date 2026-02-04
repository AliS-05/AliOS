AS = nasm
CXX = g++
LD = ld

SRC_DIR = ../src
BUILD_DIR = .


BINARY = $(BUILD_DIR)/os.bin
BOOT = $(SRC_DIR)/superboot.s
KERNEL_ASM = $(SRC_DIR)/kernel_asm.s

BOOT_BIN = $(BUILD_DIR)/superboot.bin
KERNEL_ASM_OBJ = $(BUILD_DIR)/kernel_asm.o
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin

CPP_SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
CPP_OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SOURCES))

CXXFLAGS = -m32 -ffreestanding -fno-exceptions -fno-rtti -fno-builtin \
           -nostdlib -fno-builtin -fno-pic -fno-stack-protector \
           -Wall -Wextra -O0

LDFLAGS = -m elf_i386 -T linker.ld

all: $(BINARY)

# Build bootloader
$(BOOT_BIN): $(BOOT)
	$(AS) -f bin $(BOOT) -o $(BOOT_BIN)

# Assemble kernel assembly
$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	$(AS) -f elf32 $(KERNEL_ASM) -o $(KERNEL_ASM_OBJ)

# Link kernel
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
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
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

debug: $(BINARY)
	qemu-system-i386 -s -S -drive format=raw,file=$(BINARY)

clean:
	rm -f *.bin *.o *.elf

.PHONY: all run debug clean
