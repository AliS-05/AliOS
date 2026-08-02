# *AliOS* 
A minimal x86 operating system built from scratch in C and assembly, featuring an x86 assembler, FAT16 filesystem, text editor, and network stack for writing, running programs, and interacting with other machines on your network entirely within the OS.
## Features
### Core System

- Custom Bootloader - x86 bootloader written in assembly
- Protected Mode Kernel - 32-bit kernel in C
- Keyboard Driver - PS/2 keyboard scancodes to ascii 
- Memory Management - Custom malloc, realloc, and free implementation
- ATA Disk Driver - Reads and writes sectors
- Fat16 Filesystem
- 32-bit x86 Assembler
- E1000 Network Adapter Driver + Network Stack

### Filesystem

*FAT16 Implementation* 
- Read, write, create, and delete files
- Disk Operations: Sector read/write
- File Execution:  Load and run binary programs from disk

### Dev Tools

*Text Editor* - Vim based editor (normal/insert/command modes)

<img width="330" height="80" alt="image" src="https://github.com/user-attachments/assets/08592556-5f4b-44bb-8410-adc4d0cbf695" />

- Write and edit files inside of the OS
- Navigation with hjkl keys
- Save with :wq, quit with :q

### x86 Assembler:
- Two-pass assembler built into the shell 
- Write assembly code in the editor 
- Support eax,ebx,ecx, and edx registers
- Supports memory dereferencing but **NOT** offsets (\[eax\] is fine but not \[eax+4\])
- Assemble into machine code
- Run generated binaries

### E1000 Driver + Network Stack: 
**Requires Setup Script**
- Sends ARP requests and handles ARP replies, storing IP + MAC address combinations in a vector
- ICMP Protocol, ping machines on the subnet and outside of the subnetkj. Also replies to ping requests
- UDP Protocol, send and receive UDP packets. Demultiplexes based on incoming port.
- DNS Protocol, allows you to ping domains such as google.com, storing resolved IP addresses in a resolution table.
### Game

*Snake* - It's Snake but written in x86 assembly

<img width="731" height="468" alt="image" src="https://github.com/user-attachments/assets/8c59f2ab-c269-4c80-bbca-590dc928c638" />




### Shell Commands
*Filesystem*:

- `ls` - Lists files
- `write <file.ext> <data>` - Create file with data
- `read <file.ext> <bytes>` - Read file contents
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

*Networking*:
- `ping <ip|domain name>` - Ping an IP or Domain Name

## *Prerequisites*

- nasm - Assembler
- gcc - C compiler
- make - Build system
- qemu-system-x86_64 - Emulator

### *Build & Run*
```bash
gcc mkFAT16.c -o mkfat
./mkfat           # Create disk image 
sudo bash slirp_tap_setup.sh # Set up tap0 bridge for networking (uses nft and adds a network rule. Make sure this doesnt mess up anything you need)
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

- Architecture: x86 32-bit protected mode
- Filesystem: FAT16
- Execution Model: Single tasking (DOS-Like)
- Lines of Code: ~5500

## Known Limitations

- No directories still (on the way soon.)
- Assembler only supports 4 registers and no offsets in dereferencing
- No virtual memory
- No multithreading / concurrency

## Planned
- RPC protocol
- DHCP
- Includes for the assembler
- Improving the assembler to be able to support more registers and offsetting.

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
bootloader, keyboard driver, and snake game all in assembly. 

After taking a few months break, I've come back and added many new features.
Including upgrading the filesystem, assembler, keyboard driver, allowing for scrolling in the terminal,
command history, improving the text editor, and even introducing a featureful network stack.
