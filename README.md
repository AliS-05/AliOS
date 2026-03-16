# AliOS

A minimal x86 operating system built from scratch with the goal of achieving self-hosting capability (compiling itself from within itself).

## Project Status
- Active Development

### Implemented
- **Bootloader** - Custom x86 bootloader in assembly
- **Kernel** - ~1200+ lines of C/C++ kernel code
- **VGA Text Mode** - 80x25 text display with cursor management
- **Keyboard Driver** - PS/2 keyboard input with scancode translation
- **Memory Management** - malloc/free implementation
- **ATA Disk Driver** - Read/write support from hard drive
- **Filesystem** - Custom simple filesystem with:
  - File creation, reading, writing, deletion
  - Multi-sector file support
  - Max 12 files (expansion planned)
- **Shell** - Interactive command-line interface with built-in commands
- **Program Loader** - Load and execute binary programs from disk (DOS-style single-tasking)

### In Progress
- NIC Driver / Network Capability

### Planned
- Self-compilation capability
- Enhanced filesystem (more files, better allocation)
- More utilities and programs

## Features

### Shell Commands
- `ls` - List files on disk
- `read <file>` - Display file contents
- `write <file> <content>` - Create/write file
- `del <file>` - Delete file
- `run <program>` - Execute binary program
- `hexdump [addr] [size]` - Memory hex dump
- `calc <expression>` - Calculator
- `echo <text>` - Print text
- `clear` - Clear screen
- `reboot` - Restart system
- `help` - Show available commands
- `run snake` - Play my x86 Assembly Snake game !

### Filesystem
- Simple flat filesystem design
- 512-byte sectors
- File table at sector 0
- Support for multi-sector files
- Read/write operations from kernel and programs

### Program Loading
- Load raw binary files (.bin) from disk
- Execute at fixed memory location (0x200000)
- Single-tasking (one program at a time)
- Return to shell on program completion

## Building

### Prerequisites
- `nasm` - Netwide Assembler
- `g++` - C++ compiler with freestanding support
- `ld` - GNU linker
- `make` - Build automation
- `qemu-system-x86_64` - Emulator for testing

### Build Commands
```bash
# Build the OS
make

# Create filesystem image
g++ mkfs.cpp -o mkfs && ./mkfs

# Run in QEMU
make run

# Clean build artifacts
make clean
```

## Architecture

### Memory Layout
```
0x00000000 - 0x000FFFFF: Low memory (BIOS, VGA, etc.)
0x00080000 - 0x00090000: Kernel
0x00100000 - 0x00200000: Heap
0x00200000+ : Program loading
```

### Disk Layout 
```
Sector 0: File table (12 file entries)
Sector 1+: File data
```

### File Entry Structure
```c
struct FileObject {
    char name[32];
    uint32_t startSector;
    uint32_t size;
} __attribute__((packed));
```

## Roadmap to Self-Hosting

1. Basic OS infrastructure (bootloader, kernel, drivers)
2. Filesystem implementation
3. Program loading
4. Text editor for writing code within OS
5. C compiler (lexer, parser, code generation)
6. Assembler and linker
7. Self-compilation test (compile kernel.c from within AliOS)

## Goals

- **Primary Goal:** Achieve self-hosting , ability to recompile the kernel from within the OS itself
- **Learning Focus:** Deep understanding of systems programming, OS internals, and low-level x86 architecture
- **Philosophy:** Build everything from scratch to understand fundamentals

## Technical Details

- **Language:** C (kernel, filesystem), x86 Assembly (bootloader, low-level, snake)
- **Architecture:** x86 (32-bit protected mode)
- **Filesystem:** Custom simple flat filesystem
- **Execution Model:** Single-tasking (DOS like program loading)
