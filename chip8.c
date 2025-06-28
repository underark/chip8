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
    emulator->pc = 0x200;
    emulator->sound_timer = 0;
    emulator->delay_timer = 0;
    emulator->running = true;
    emulator->hires = false;

    uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    uint16_t font10[] = {
        0xC67C, 0xDECE, 0xF6D6, 0xC6E6, 0x007C, // 0
        0x3010, 0x30F0, 0x3030, 0x3030, 0x00FC, // 1
        0xCC78, 0x0CCC, 0x3018, 0xCC60, 0x00FC, // 2
        0xCC78, 0x0C0C, 0x0C38, 0xCC0C, 0x0078, // 3
        0x1C0C, 0x6C3C, 0xFECC, 0x0C0C, 0x001E, // 4
        0xC0FC, 0xC0C0, 0x0CF8, 0xCC0C, 0x0078, // 5
        0x6038, 0xC0C0, 0xCCF8, 0xCCCC, 0x0078, // 6
        0xC6FE, 0x06C6, 0x180C, 0x3030, 0x0030, // 7
        0xCC78, 0xECCC, 0xDC78, 0xCCCC, 0x0078, // 8
        0xC67C, 0xC6C6, 0x0C7E, 0x3018, 0x0070, // 9
        0x7830, 0xCCCC, 0xFCCC, 0xCCCC, 0x00CC, // A
        0x66FC, 0x6666, 0x667C, 0x6666, 0x00FC, // B
        0x663C, 0xC0C6, 0xC0C0, 0x66C6, 0x003C, // C
        0x6CF8, 0x6666, 0x6666, 0x6C66, 0x00F8, // D
        0x62FE, 0x6460, 0x647C, 0x6260, 0x00FE, // E
        0x66FE, 0x6462, 0x647C, 0x6060, 0x00F0  // F
    };

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
        emulator->flags[i] = 0;
    }

    for (int i = 0; i < 80; i++) {
        emulator->memory[i] = font[i];
    }

    for (int i = 80; i < 160; i++) {
        emulator->memory[i] = font10[i];
    }

    srand(time(NULL));
}

bool read_to_memory(char* filename, struct Chip8* emulator) {
    FILE* rom = fopen(filename, "rb");
    if (!rom) {
        printf("file not found");
        return false;
    }

    uint8_t buffer;
    int c = 0x200;
    while (fread(&buffer, sizeof(uint8_t), 1, rom) > 0) {
        if (c >= MEMORY_SIZE) {
            printf("Rom too big\n");
            return false;
        }
        emulator->memory[c] = buffer;
        c++;
    }
    fclose(rom);
    return true;
}

bool decode_execute(uint16_t code, struct Chip8* emulator, struct SDLPack* SDLPack) {
    int u;
    uint16_t nnn = code & 0x0FFF;
    uint8_t vx = emulator->V[(code & 0x0F00) >> 8];
    uint8_t vy = emulator->V[(code & 0x00F0) >> 4];
    uint8_t nn = code & 0x00FF;
    uint8_t n = code & 0x000F;
    uint8_t x;
    uint8_t y;
    uint8_t row_b;
    uint8_t b;
    switch (code & 0xF000) {
        case 0x0000:
            switch (code & 0x0FFF) {
                case 0x00E0:
                    for (int i = 0; i < DISPLAY_SIZE; i++) {
                        emulator->display[i] = 0;
                    }
                    break;
                case 0x00EE:
                    emulator->sp--;
                    emulator->pc = emulator->stack[emulator->sp];
                    break;
                case 0x00FB:
                    for (int r = 0; r < 64; r++) {
                        for (int c = 127; c >= 0; c--) {
                            int pos = r * 128 + c;
                            if (c - 4 >= 0) {
                                emulator->display[pos] = emulator->display[pos - 4];
                            }
                        }
                    }
                    break;
                case 0x00FC:
                    for (int r = 0; r < 64; r++) {
                        for (int c = 0; c < 128; c++) {
                            int pos = r * 128 + c;
                            if (emulator->hires && c + 4 < 128) {
                                emulator->display[pos] = emulator->display[pos + 4];
                            } else if (c + 2 < 128) {
                                emulator->display[pos] = emulator->display[pos + 2];
                            }
                        }
                    }
                case 0x00FD:
                    //emulator->running = false;
                    break;
                case 0x00FE:
                    emulator->hires = false;
                    break;
                case 0x00FF:
                    emulator->hires = true;
                    break;
                default:

                    break;
            }
            break;
        case 0x1000:
            emulator->pc = nnn;
            break;
        case 0x2000:
            emulator->stack[emulator->sp] = emulator->pc;
            if (emulator->sp < 15) {
                emulator->sp++;
            }
            emulator->pc = nnn;
            break;
        case 0x3000:
            if (vx == nn) {
                emulator->pc += 2;
            }
            break;
        case 0x4000:
            if (vx != nn) {
                emulator->pc += 2;
            }
            break;
        case 0x5000:
            if (vx == vy) {
                emulator->pc += 2;
            }
            break;
        case 0x6000:
            emulator->V[(code & 0x0F00) >> 8] = nn;
            break;
        case 0x7000:
            emulator->V[(code & 0x0F00) >> 8] += nn;
            break;
        case 0x8000:
            switch (code & 0x000F) {
                case 0:
                    emulator->V[(code & 0x0F00) >> 8] = vy;
                    break;
                case 1:
                    emulator->V[(code & 0x0F00) >> 8] |= vy;
                    emulator->V[0xF] = 0;
                    break;
                case 2:
                    emulator->V[(code & 0x0F00) >> 8] &= vy;
                    emulator->V[0xF] = 0;
                    break;
                case 3:
                    emulator->V[(code & 0x0F00) >> 8] ^= vy;
                    emulator->V[0xF] = 0;
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
                    emulator->V[(code & 0x0F00) >> 8] = vy;
                    emulator->V[(code & 0x0F00) >> 8] >>= 1;
                    emulator->V[0xF] = vy & 1;
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
                    emulator->V[(code & 0x0F00) >> 8] = vy;
                    emulator->V[(code & 0x0F00) >> 8] <<= 1;
                    emulator->V[0xF] = (vy & 0b10000000) >> 7;
                    break;
            }
            break;
        case 0x9000:
            if (vx != vy) {
                emulator->pc += 2;
            }
            break;
        case 0xA000:
            emulator->I = nnn;
            break;
        case 0xB000:
            emulator->pc = nnn + emulator->V[0];
            break;
        case 0xC000:
            int r = rand() % 255 + 1;
            emulator->V[(code & 0x0F00) >> 8] = r & nn;
            break;
        case 0xD000:
            switch (code & 0x000F) {
                case 0:
                    emulator->V[0xF] = 0;
                    for (int r = 0; r < 16 && y + r < 64; r++) {
                        for (int c = 0; c < 16 && x + c < 128; c++) {
                            int loc = (y + r) * 128 + (x + c);
                            uint16_t row_b = emulator->memory[emulator->I + r] << 8 | emulator->memory[emulator->I + r + 1];
                            uint16_t b = (row_b >> (15 - c)) & 1;
                            
                            if (emulator->display[loc] == 1) {
                                emulator->V[0xF] = 1;
                            }
                            emulator->display[loc] ^= b;
                        }
                    }    
                    break;
                default:
                    switch (emulator->hires) {
                        case true:
                            x = vx % 128;
                            y = vy % 64;
                            emulator->V[0xF] = 0;
                            for (int r = 0; r < n && y + r < 64; r++) {
                                for (int c = 0; c < 8 && x + c < 128; c++) {
                                    int loc = (y +r) * 128 + (x + c);

                                    row_b = emulator->memory[emulator->I + r];
                                    b = (row_b >> (7 - c)) & 1;

                                    emulator->V[0xF] = emulator->display[loc] & b;
                                    emulator->display[loc] ^= b;
                                }
                            }
                            break;
                        case false:
                            x = vx % 64;
                            y = vy % 32;
                            emulator->V[0xF] = 0;
                            for (int r = 0; r < n && y + r < 32; r++) {
                                for (int c = 0; c < 8 && x + c < 64; c++) {
                                    int loc1 = 2 * ((y + r) * 128 + (x + c));
                                    int loc2 = loc1 + 128;

                                    row_b = emulator->memory[emulator->I + r];
                                    b = (row_b >> (7 - c)) & 1;
                                    

                                    emulator->V[0xF] = emulator->display[loc1] & b;
                                    emulator->display[loc1] ^= b;
                                    emulator->V[0xF] = emulator->display[loc1 + 1] & b;
                                    emulator->display[loc1 + 1] ^= b;
                                    emulator->V[0xF] = emulator->display[loc2] & b;
                                    emulator->display[loc2] ^= b;
                                    emulator->V[0xF] = emulator->display[loc2 + 1] & b;
                                    emulator->display[loc2 + 1] ^= b;
                                }
                            }
                            break;
                    }
            }
            emulator->draw = true;
            break;
        case 0xE000:
            switch (code & 0x00FF) {
                case 0x009E:
                    if (emulator->key[vx & 0xF] == 1) {
                        emulator->pc += 2;
                    }
                    break;
                case 0x00A1:
                    if (emulator->key[vx & 0xF] != 1) {
                        emulator->pc += 2;
                    }
                    break;
            }
            break;
        case 0xF000:
            switch (code & 0x00FF) {
                case 0x0033:
                    uint8_t h = vx / 100;
                    uint8_t t = (vx % 100) / 10;
                    uint8_t d = vx % 10;
                    emulator->memory[emulator->I] = h;
                    emulator->memory[emulator->I + 1] = t;
                    emulator->memory[emulator->I + 2] = d;
                    break;
                case 0x0055:
                    uint8_t l = (code & 0x0F00) >> 8;
                    switch (emulator->schip) {
                        case true:
                            for (int i = 0; i <= l; i++) {
                                emulator->memory[emulator->I + i] = emulator->V[i];
                            }
                            break;
                        case false:
                            for (int i = 0; i <= l; i++) {
                                emulator->memory[emulator->I] = emulator->V[i];
                                emulator->I++;
                            }
                            break;
                    }
                    break;
                case 0x0065:
                    l = (code & 0x0F00) >> 8;
                    switch (emulator->schip) {
                        case true:
                            for (int i = 0; i <= l; i++) {
                                emulator->V[i] = emulator->memory[emulator->I + i];
                            }
                            break;
                        case false:
                            for (int i = 0; i <= l; i++) {
                                emulator->V[i] = emulator->memory[emulator->I];
                                emulator->I++;
                            }
                            break;
                    }
                    break;
                case 0x0007:
                    emulator->V[(code & 0x0F00) >> 8] = emulator->delay_timer;
                    break;
                case 0x0015:
                    emulator->delay_timer = vx;
                    break;
                case 0x0018:
                    emulator->sound_timer = vx;
                    break;
                case 0x000A:
                    emulator->waiting = true;
                    emulator->wait_register = emulator->opcode;
                    break;
                case 0x001E:
                    emulator->I += vx;
                    if (emulator->schip && emulator->I > 0x0FFF) {
                        emulator->V[0xF] = 1;
                    } else if (emulator->schip && emulator->I <= 0x0FFF) {
                        emulator->V[0xF] = 0;
                    }
                    break;
                case 0x0029:
                    emulator->I = (vx & 0xF) * 5;
                    break;
                case 0x0030:                        
                    emulator->I = (vx & 0xF) * 10;
                    break;
                case 0x0075:
                    u = (((code & 0x0F00) >> 8) > 7) ? 7 : (code & 0x0F00) >> 8;
                    for (int i = 0; i <= u; i++) {
                        emulator->flags[i] = emulator->V[i];
                    }
                    break;
                case 0x0085:
                    u = (((code & 0x0F00) >> 8) > 7) ? 7 : (code & 0x0F00) >> 8;
                    for (int i = 0; i <= u; i++) {
                        emulator->V[i] = emulator->flags[i];
                    }
                    break;
            }
            break;
        default:
            printf("invalid opcode!!!!\n");
            break;
    }
    return true;
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

// v1 is value to be checked; v2 is expected value upper limit
bool is_in_bounds(int v1, int v2) {
    if (v1 >= v2) {
        return false;
    }
    return true;
}

