#pragma once
#include <structures.h>

static inline void outb(uint16_t port, uint8_t val){
	__asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void insw(uint16_t port, void* addr, int count) {
	__asm__ __volatile__ ("rep insw"
                  : "+D"(addr), "+c"(count)
                  : "d"(port)
                  : "memory");
}

static inline void outsw(uint16_t port, void* addr, int count){
	__asm__ __volatile__ ("rep outsw"
			: "+S"(addr), "+c"(count)
			: "d"(port)
			: "memory");
}

static void ata_wait_busy() {
    while (inb(0x1F7) & 0x80); // BSY bit
}

static void ata_wait_drq() {
    while (!(inb(0x1F7) & 0x08)); // Data Request Bit
}
