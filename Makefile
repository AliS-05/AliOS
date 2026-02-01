AS = nasm
BINARY = os.bin
KERNEL = kernel.s
BOOT = superboot.s

all: $(BINARY)

$(BINARY): $(BOOT) $(KERNEL)
	nasm -f bin $(BOOT) -o superboot.bin
	nasm -f bin $(KERNEL) -o kernel.bin
	cat superboot.bin kernel.bin > os.bin

run: $(BINARY)
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

debug: $(BINARY)
	qemu-system-x86_64 -s -S -drive format=raw,file=$(BINARY)

clean:
	rm *.bin
