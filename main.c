#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    struct chip8 *emulator = malloc(sizeof(struct chip8));
    initialize(emulator);
    read_to_memory(argv[1], emulator);
    emulator->pc = 0x200;
    while (emulator->pc < 613) {
        emulator->opcode = emulator->memory[emulator->pc] << 8 | emulator->memory[emulator->pc + 1];
        execute(emulator->opcode, emulator);
    }
    return 0;
}