#include "chip8.h"
#include "display.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// Can I just have a global variable since I only plan to have one emulator struct in memory at a time?
void initialize(struct Chip8* emulator) {
    emulator->opcode = 0;
    emulator->I = 0;
    emulator->sp = 0;

    emulator->sound_timer = 0;
    emulator->delay_timer = 0;

    // Loading the fonts might not actually work??
    uint8_t b;
    emulator->pc = 0;
    FILE* font = fopen("cosmac.txt", "r");
    if (!font) {
        printf("Font loading failed");
        return;
    }
    while (fread(&b, 1, 1, font) > 0) {
        emulator->memory[emulator->pc] = b;
        emulator->pc++;
    }
    fclose(font);

    for (int i = 0; i < MEMORY_SIZE; i++) {
        emulator->memory[i] = 0;
    }

    for (int i = 0; i < DISPLAY_SIZE; i++) {
        emulator->display[i] = 0;
    }

    for (int i = 0; i < REGISTER_SIZE; i++) {
        emulator->V[i] = 0;
        emulator->key[i] = 0;
        emulator->stack[i] = 0;
    }
    emulator->pc = 0x200;
    srand(time(NULL));
}

bool read_to_memory(char* filename, struct Chip8* emulator) {
    FILE* rom = fopen(filename, "rb");
    if (!rom) {
        printf("file not found");
        return false;
    }

    uint8_t buffer;
    while (fread(&buffer, sizeof(uint8_t), 1, rom) > 0) {
        emulator->memory[emulator->pc] = buffer;
        emulator->pc++;
    }
    fclose(rom);
    return true;
}

void decode_execute(uint16_t code, struct Chip8* emulator, struct SDLPack* SDLPack) {
    uint16_t nnn = code & 0x0FFF;
    uint8_t vx = emulator->V[(code & 0x0F00) >> 8];
    uint8_t vy = emulator->V[(code & 0x00F0) >> 4];
    uint8_t nn = code & 0x00FF;
    switch (code & 0xF000) {
        case 0x0000:
            switch (code & 0x0FFF) {
                case 0x00E0:
                    for (int i = 0; i < DISPLAY_SIZE; i++) {
                        emulator->display[i] = 0;
                    }
                    update_display(emulator, SDLPack);
                    break;
                case 0x00EE:
                    emulator->sp--;
                    emulator->pc = emulator->stack[emulator->sp];
                    break;
            }
            emulator->pc += 2;
            break;
        case 0x1000:
            emulator->pc = nnn;
            break;
        case 0x2000:
            emulator->stack[emulator->sp] = emulator->pc;
            emulator->sp++;
            emulator->pc = nnn;
            break;
        case 0x3000:
            if (vx == nn) {
                emulator->pc += 2;
            }
            emulator->pc += 2;
            break;
        case 0x4000:
            if (vx != nn) {
                emulator->pc += 2;
            }
            emulator->pc +=2;
            break;
        case 0x5000:
            if (vx == vy) {
                emulator->pc += 2;
            }
            emulator->pc += 2;
            break;
        case 0x6000:
            emulator->V[(code & 0x0F00) >> 8] = nn;
            emulator->pc += 2;
            break;
        case 0x7000:
            emulator->V[(code & 0x0F00) >> 8] += nn;
            emulator->pc += 2;
            break;
        case 0x8000:
            switch (code & 0x000F) {
                case 0:
                    emulator->V[(code & 0x0F00) >> 8] = emulator->V[(code & 0x00F0) >> 4];
                    break;
                case 1:
                    emulator->V[(code & 0x0F00) >> 8] |= emulator->V[(code & 0x00F0) >> 4];
                    break;
                case 2:
                    emulator->V[(code & 0x0F00) >> 8] &= emulator->V[(code & 0x00F0) >> 4];
                    break;
                case 3:
                    emulator->V[(code & 0x0F00) >> 8] ^= emulator->V[(code & 0x00F0) >> 4];
                    break;
                case 4:
                    emulator->V[(code & 0x0F00) >> 8] += vy;
                    if (vx + vy > 255) {
                        emulator->V[0xF] = 1;
                    } else {
                        emulator->V[0xF] = 0;
                    }
                    break;
                case 5:
                    emulator->V[(code & 0x0F00) >> 8] -= vy;
                    if (vx >= vy) {
                        emulator->V[0xF] = 1;
                    } else {
                        emulator->V[0xF] = 0;
                    }
                    break;
                case 6:
                    emulator->V[(code & 0x0F00) >> 8] >>= 1;
                    emulator->V[0xF] = vx & 1;
                    break;
                case 7:
                    emulator->V[(code & 0x0F00) >> 8] = vy - vx;
                    if (vy >= vx) {
                        emulator->V[0xF] = 1;
                    } else {
                        emulator->V[0xF] = 0;
                    }
                    break;
                case 0xE:
                    emulator->V[(code & 0x0F00) >> 8] <<= 1;
                    emulator->V[0xF] = (vx & 0b10000000) >> 7;
                    break;
            }
            emulator->pc += 2;
            break;
        case 0x9000:
            if (vx != vy) {
                emulator->pc += 2;
            }
            emulator->pc += 2;
            break;
        case 0xA000:
            emulator->I = nnn;
            emulator->pc += 2;
            break;
        case 0xB000:
            emulator->pc = nnn + emulator->V[0];
            break;
        case 0xC000:
            int r = rand() % 255 + 1;
            emulator->V[(code & 0x0F00) >> 8] = r & nn;
            emulator->pc += 2;
            break;
        case 0xD000:
            emulator->V[0xF] = 0;
            uint8_t x = emulator->V[(code & 0x0F00) >> 8] % 64;
            uint8_t y = emulator->V[(code & 0x00F0) >> 4] % 32;
            uint8_t n = code & 0x000F;
            for (int r = 0; r < n && y + r < 32; r++){
                for (int c = 0; c < 8 && x + c < 64; c++) {
                    uint8_t row_b = emulator->memory[emulator->I + r];
                    int loc = (y + r) * 64 + (x + c);
                    uint8_t b = (row_b >> (7 - c)) & 1;
                    if ((emulator->display[loc] ^ b) == 0) {
                        emulator->V[0xF] = 1;
                    }
                    emulator->display[loc] ^= b;
                }
            }
            update_display(emulator, SDLPack);
            emulator->pc += 2;
            break;
        case 0xE000:
            switch (code & 0x00FF) {
                case 0x009E:
                    if (emulator->key[vx] == 1) {
                        emulator->pc += 2;
                    }
                    break;
                case 0x00A1:
                    if (emulator->key[vx] != 1) {
                        emulator->pc += 2;
                    }
                    break;
            }
            emulator->pc += 2;
            break;
        case 0xF000:
            switch (code & 0x00FF) {
                case 0x0033:
                    uint8_t h = emulator->V[(code & 0x0F00) >> 8] / 100;
                    uint8_t t = (emulator->V[(code & 0x0F00) >> 8] % 100) / 10;
                    uint8_t d = emulator->V[(code & 0x0F00) >> 8] % 10;
                    emulator->memory[emulator->I] = h;
                    emulator->memory[emulator->I + 1] = t;
                    emulator->memory[emulator->I + 2] = d;
                    break;
                case 0x0055:
                    uint8_t l = (code & 0x0F00) >> 8;
                    for (int i = 0; i <= l; i++) {
                        emulator->memory[emulator->I + i] = emulator->V[i];
                    }
                    break;
                case 0x0065:
                    l = (code & 0x0F00) >> 8;
                    for (int i = 0; i <= l; i++) {
                        emulator->V[i] = emulator->memory[emulator->I + i];
                    }
                    break;
                case 0x0007:
                    emulator->V[(code & 0x0F00) >> 8] = emulator->delay_timer;
                    break;
                case 0x0015:
                    emulator->delay_timer = emulator->V[(code & 0x0F00) >> 8];
                    break;
                case 0x0018:
                    emulator->sound_timer = emulator->V[(code & 0x0F00) >> 8];
                    break;
                case 0x000A:
                    emulator->waiting = true;
                    emulator->wait_register = emulator->opcode;
                    break;
                case 0x001E:
                    emulator->I += emulator->V[(code & 0x0F00) >> 8];
                    break;
                case 0x0029:
                    emulator->I = emulator->V[(code & 0x0F00) >> 8];
                    break;
            }
            emulator->pc += 2;
            break;
        default:
            printf("invalid opcode!!!!\n");
            break;
    }
}

int to_key(SDL_KeyCode key) {
    switch (key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;

        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;

        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;

        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
    }
    return -1;
}

