AS = nasm
BINARY = os.bin

all: $(BINARY)

# This rule tells make how to turn a .s into a .bin

run: $(BINARY)
	qemu-system-x86_64 -drive format=raw,file=$(BINARY)

debug: $(BINARY)
	qemu-system-x86_64 -s -S -drive format=raw,file=$(BINARY)

clean:
	rm -f $(BINARY) *.o
