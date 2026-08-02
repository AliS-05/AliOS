#include <networking/ethernet.h>

uint16_t checksum(const void* data, uint32_t length) {
	uint32_t sum = 0;
	const uint16_t* words = (const uint16_t*)data;

	while (length > 1) {
		sum += btol16(*words++);
		length -= 2;
	}

	if (length == 1) {
		sum += ((const uint8_t*)words)[0] << 8;
	}

	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ltob16(~sum);
}
