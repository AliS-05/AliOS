CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -Iinclude -I../include -O2 -fno-pie -fno-pic
LDFLAGS = -nostdlib -m32 -no-pie

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

KERNEL_OBJ = ../build/memory.o \
             ../build/string.o \
             ../build/utilities.o \
             ../build/fs/ata.o \
             ../build/fs/fat16.o \
             ../build/fs/fs.o

TARGET = assemblr.bin

all: $(TARGET)

$(TARGET): $(OBJ) $(KERNEL_OBJ)
	$(CC) $(LDFLAGS) -o assemblr.elf $(OBJ) $(KERNEL_OBJ) -T linker.ld
	objcopy -O binary assemblr.elf $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o assemblr.elf $(TARGET)

.PHONY: all clean
