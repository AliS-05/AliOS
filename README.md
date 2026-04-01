# *AliOS* 
A minimal x86 operating system built from scratch in C and assembly, featuring an x86 assembler, FAT16 filesystem, and text editor for writing and running programs entirely within the OS.
## Features
### Core System

- Custom Bootloader - x86 bootloader written in assembly
- Protected Mode Kernel - 32-bit kernel in C
- Keyboard Driver - PS/2 keyboard scancodes to ascii 
- Memory Management - Custom malloc, realloc, and free implementation
- ATA Disk Driver - Reads and writes sectors

### Filesystem

*FAT16 Implementation* - Read, write, create, and delete files
Disk Operations - Multi-sector read/write
File Execution - Load and run binary programs from disk

### Dev Tools

*Text Editor* - Vim based editor (normal/insert/command modes)

<img width="330" height="80" alt="image" src="https://github.com/user-attachments/assets/08592556-5f4b-44bb-8410-adc4d0cbf695" />

Write and edit files inside of the OS
Navigation with hjkl keys
Save with :wq, quit with :q

*x86 Assembler* - Two-pass assembler built into the shell

Write assembly code in the editor
Assemble into machine code
Run generated binaries

*E1000 Driver* - Only able to transmit packets at the moment, 
receive packets planned in next version

### Game

*Snake* - It's Snake but written in x86 assembly

<img width="731" height="468" alt="image" src="https://github.com/user-attachments/assets/8c59f2ab-c269-4c80-bbca-590dc928c638" />




### Shell Commands
*Filesystem*:

- `ls` - Lists files
- `write <file.ext> <data>` - Create file with data
- `read <file.ext> [bytes]` - Read file contents
- `del <file.ext>` - Delete file
- `edit <file.ext>` - Open text editor

*Execution*:

- `run <file.ext>` - Execute binary program
- `assemble <file.asm>` - Assemble x86 code to asoutput.exe


*Utilities*:


- `echo <text>` - Print text
- `calc <expression>` - Simple calculator
- `hexdump <addr> <size>` - Memory dump (default: 0x100000, 256 bytes)
- `color <num>` - Change screen color (0-255)
- `clear` - Clear screen
- `reboot` - Restart system
- `help` - Show all commands

## *Prerequisites*

- nasm - Assembler
- gcc - C compiler
- make - Build system
- qemu-system-x86_64 - Emulator

### *Build & Run*
```
./mkfat           # Create disk image
make run          # Boot in QEMU
```

## Architecture

*Disk Layout (FAT16):*

Sector 0      : Boot sector
Sector 1-20   : FAT table 1
Sector 21-40  : FAT table 2
Sector 41-72  : Root directory (512 entries)
Sector 73+    : Data area (clusters)

*Technical Details*

- Language: C (kernel), x86 Assembly (bootloader, drivers, games)
- Architecture: x86 32-bit protected mode
- Filesystem: FAT16
- Execution Model: Single tasking (DOS-Like)
- Lines of Code: ~4000

## Known Limitations

- Single cluster files only (2048 bytes max per file)
- No subdirectories
- No virtual memory

## Planned
- Currently the Networking Stack only transmits packets, so receiving packets is an obvious next step.
- Switching from flat memory to virtual memory, along with adding processes and a user space.
- Improving the assembler to be able to assemble more complex programs.
- A C compiler robust enough to compile the source code of the operating system

## What I Learned
What started as a simple project to learn x86 assembly 
turned into an amazing project over the course of 2 months.
I genuinely learned so much in such a short period of time.
Of course I have a much more intimate understanding of how
the computer actually functions and in turn am now a better 
programmer, but more specifically many things that once seemed like pure magic no longer seem so unfathomable.
For example, software interacting with hardware
and driver code. It might sound obvious but hardware is 
made to be interacted with software. Crazy right? 
But that's not really something you can truly grasp until
you interact with the manuals yourself I guess. At least I
didn't. If you're interested in doing something similar to
this project, the best advice I could give is to just get 
started. I literally didn't know a single bit of assembly
syntax other than mov and rax. And I ended up writing my own
bootloader, keyboard driver, and snake game all in assembly
