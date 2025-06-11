#include <stdint.h>

#define MEMORY_SIZE 4096
#define DISPLAY_SIZE (64 * 32)
#define STACK_SIZE 12
#define REGISTER_SIZE 16

struct chip8 {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[REGISTER_SIZE];
    uint8_t display[DISPLAY_SIZE];
    uint16_t opcode;
    uint16_t I;
    uint16_t pc;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[STACK_SIZE];
    uint16_t sp;
    uint8_t key[REGISTER_SIZE];
};

void initialize(struct chip8* emulator);
void read_to_memory(char* filename, struct chip8* emulator);
uint16_t parse(uint16_t code);
void execute(uint16_t code, struct chip8* emulator);