#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


void codeGen(int return_value) {
    uint8_t output[20];
    
    output[0] = 0x55;        // push ebp
    output[1] = 0x89;        // mov ebp, esp
    output[2] = 0xE5;
    output[3] = 0xB8;        // mov eax, return_value
    output[4] = return_value & 0xFF;
    output[5] = (return_value >> 8) & 0xFF;
    output[6] = (return_value >> 16) & 0xFF;
    output[7] = (return_value >> 24) & 0xFF;
    output[8] = 0x5D;        // pop ebp
    output[9] = 0xC3;        // ret
    
    // Write to file
    FILE* f = fopen("output.bin", "wb");
    fwrite(output, 1, 10, f);
    fclose(f);
    
    printf("Generated binary: output.bin\n");
}

