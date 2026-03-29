CC = gcc
CFLAGS = -m32 -ffreestanding -nostdlib -Wall -Wextra -Iinclude -I../include -O2
LDFLAGS = -nostdlib

# All assembler source files
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

# Precompiled objects in ../build
BUILD_OBJ = $(wildcard ../build/*.o) $(wildcard ../build/fs/*.o) $(wildcard ../build/tools/*.o) $(wildcard ../build/networking/*.o)

TARGET = assemblr.bin

all: $(TARGET)

$(TARGET): $(OBJ) $(BUILD_OBJ)
	$(CC) $(LDFLAGS) -m32 -o  $@ $(OBJ) $(BUILD_OBJ) -T linker.ld

# Compile .c -> .o
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
