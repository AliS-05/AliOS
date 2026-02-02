AS = nasm
CXX = g++
LD = ld

BINARY = os.bin
BOOT = superboot.s
KERNEL_ASM = kernel.s
KERNEL_CPP = extra.cpp

BOOT_BIN = superboot.bin
KERNEL_ASM_OBJ = kernel.o
KERNEL_CPP_OBJ = extra.o
KERNEL_ELF = kernel.elf
KERNEL_BIN = kernel.bin

CXXFLAGS = -m32 -ffreestanding -fno-exceptions -fno-rtti \
           -nostdlib -fno-builtin -fno-stack-protector \
           -Wall -Wextra -O2

LDFLAGS = -m elf_i386 -T linker.ld

all: $(BINARY)

# Build bootloader
$(BOOT_BIN): $(BOOT)
	$(AS) -f bin $(BOOT) -o $(BOOT_BIN)

# Assemble kernel assembly
$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
	$(AS) -f elf32 $(KERNEL_ASM) -o $(KERNEL_ASM_OBJ)

# Compile C++ kernel
$(KERNEL_CPP_OBJ): $(KERNEL_CPP)
	$(CXX) $(CXXFLAGS) -c $(KERNEL_CPP) -o $(KERNEL_CPP_OBJ)

# Link kernel
$(KERNEL_ELF): $(KERNEL_ASM_OBJ) $(KERNEL_CPP_OBJ)
	$(LD) $(LDFLAGS) $(KERNEL_ASM_OBJ) $(KERNEL_CPP_OBJ) -o $(KERNEL_ELF)

# Extract kernel binary
$(KERNEL_BIN): $(KERNEL_ELF)
	objcopy -O binary $(KERNEL_ELF) $(KERNEL_BIN)

# Combine bootloader and kernel
$(BINARY): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(BINARY)

run: $(BINARY)
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

debug: $(BINARY)
	qemu-system-i386 -s -S -drive format=raw,file=$(BINARY)

clean:
	rm -f *.bin *.o *.elf

.PHONY: all run debug clean
