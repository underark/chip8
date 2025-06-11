#include "chip8.h"
#include <stdio.h>

// Can I just have a global variable since I only plan to have one emulator struct in memory at a time?
void initialize(struct chip8* emulator) {
    emulator->pc = 0x200;
    emulator->opcode = 0;
    emulator->I = 0;
    emulator->sp = 0;

    emulator->sound_timer = 0;
    emulator->delay_timer = 0;

    for (int i = 0; i <= MEMORY_SIZE; i++) {
        emulator->memory[i] = 0;
    }

    for (int i = 0; i <= DISPLAY_SIZE; i++) {
        emulator->display[i] = 0;
    }

    for (int i = 0; i <= STACK_SIZE; i++) {
        emulator->stack[i] = 0;
    }

    for (int i = 0; i <= REGISTER_SIZE; i++) {
        emulator->V[i] = 0;
    }
}

void read_to_memory(char* filename, struct chip8* emulator) {
    FILE* rom = fopen(filename, "rb");
    if (!rom) {
        printf("file not found");
        return;
    }

    uint8_t buffer;
    while (fread(&buffer, sizeof(uint8_t), 1, rom) > 0) {
        emulator->memory[emulator->pc] = buffer;
        emulator->pc++;
    }
    fclose(rom);
}

uint16_t parse(uint16_t code) {
    return (code & 0xF000);
}

void execute(uint16_t code, struct chip8* emulator) {
    uint16_t parsed = parse(code);
    switch (parsed) {
        case 0x0000:
            if (code == 0x00E0) {
                for (int i = 0; i < DISPLAY_SIZE; i++) {
                    emulator->display[i] = 0;
                }
            }
            emulator->pc += 2;
            break;
        case 0x1000:
            emulator->pc = code & 0x0FFF;
            break;
        case 0x6000:
        default:
            emulator->pc += 2;
            break;
    }
}

