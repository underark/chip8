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
                // 00E0 clears display
                case 0x00E0:
                    clear_display(emulator->display);
                    break;
                // 00EE RET from subroutine
                case 0x00EE:
                    emulator->sp--;
                    emulator->pc = emulator->stack[emulator->sp];
                    break;
                // 00FB Shifts display to the right four pixels (2 in lores mode)
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
                // 00FC shifts display four to the left (2 in lores)
                case 0x00FC:
                    for (int r = 0; r < 64; r++) {
                        for (int c = 0; c + 4 < 128; c++) {
                            int pos = r * 128 + c;
                            emulator->display[pos] = emulator->display[pos + 4];
                        }
                    }
                // Instantly causes the interpreter to stop running; probably isn't desirable
                case 0x00FD:
                    //emulator->running = false;
                    break;
                // 00FE Disables hires mode in SCHIP
                case 0x00FE:
                    emulator->hires = false;
                    break;
                // 00FF Enables hires mode in SCHIP
                case 0x00FF:
                    emulator->hires = true;
                    break;
                // 00CN Shifts display down by N (N/2 in lores)
                // 00C0 is technically not a valid opcode
                default:
                    for (int r = 63; r - n >= 0; r--) {
                        for (int c = 127; c >= 0; c--) {
                            int pos = r * 128 + c;
                            emulator->display[pos] = emulator->display[pos - 128 * n];
                            emulator->display[pos - 128 * n] = 0;
                        }
                    }
                    break;
            }
            break;
        // 1NNN Jump to NNN
        case 0x1000:
            emulator->pc = nnn;
            break;
        /// 2NNN Enter subroutine at NNN and store RET address on the stack
        case 0x2000:
            emulator->stack[emulator->sp] = emulator->pc;
            if (emulator->sp < 15) {
                emulator->sp++;
            }
            emulator->pc = nnn;
            break;
        // 3XNN skip next opcode if vX == NN
        case 0x3000:
            if (vx == nn) {
                emulator->pc += 2;
            }
            break;
        // 4XNN skip next opcode if vX != NN
        case 0x4000:
            if (vx != nn) {
                emulator->pc += 2;
            }
            break;
        // 5XY0 skip next opcode if vX == vY
        case 0x5000:
            if (vx == vy) {
                emulator->pc += 2;
            }
            break;
        // 6XNN set vX to NN
        case 0x6000:
            emulator->V[(code & 0x0F00) >> 8] = nn;
            break;
        // 7XNN add NN to vX
        case 0x7000:
            emulator->V[(code & 0x0F00) >> 8] += nn;
            break;
        case 0x8000:
            switch (code & 0x000F) {
                // 8XY0 set vX to the value of vY
                case 0:
                    emulator->V[(code & 0x0F00) >> 8] = vy;
                    break;
                // 8XY1 set vX to the result of bitwise vX OR vY - original chip8 also resets VF
                case 1:
                    emulator->V[(code & 0x0F00) >> 8] |= vy;
                    if (!emulator->schip) emulator->V[0xF] = 0;
                    break;
                // 8XY2 set vX to the result of bitwise vX AND vY - see above
                case 2:
                    emulator->V[(code & 0x0F00) >> 8] &= vy;
                    if (!emulator->schip) emulator->V[0xF] = 0;
                    break;
                // 8XY3 set vX to the result of bitwise vX XOR vY
                case 3:
                    emulator->V[(code & 0x0F00) >> 8] ^= vy;
                    if (!emulator->schip) emulator->V[0xF] = 0;
                    break;
                // 8XY4 add vY to vX, vF is set to 1 if an overflow happened, to 0 if not, even if X=F!
                case 4:
                    emulator->V[(code & 0x0F00) >> 8] += vy;
                    if (vx + vy > 255) {
                        emulator->V[0xF] = 1;
                    } else {
                        emulator->V[0xF] = 0;
                    }
                    break;
                // 8XY5 subtract vY from vX, vF is set to 0 if an underflow happened, to 1 if not, even if X=F!
                case 5:
                    emulator->V[(code & 0x0F00) >> 8] -= vy;
                    if (vx >= vy) {
                        emulator->V[0xF] = 1;
                    } else {
                        emulator->V[0xF] = 0;
                    }
                    break;
                // 8XY6 set vX to vY and shift vX one bit to the right, set vF to the bit shifted out, even if X=F! 
                // SCHIP doesn't set vX to vY - shifts vX itself
                case 6:
                    switch (emulator->schip) {
                        case true:
                            emulator->V[(code & 0x0F00) >> 8] >>= 1;
                            emulator->V[0xF] = vx & 1;
                            break;
                        case false:
                            emulator->V[(code & 0x0F00) >> 8] = vy;
                            emulator->V[(code & 0x0F00) >> 8] >>= 1;
                            emulator->V[0xF] = vy & 1;
                            break;
                    }
                    break;
                // 8XY7 set vX to the result of subtracting vX from vY, vF is set to 0 if an underflow happened, to 1 if not, even if X=F!
                case 7:
                    emulator->V[(code & 0x0F00) >> 8] = vy - vx;
                    if (vy >= vx) {
                        emulator->V[0xF] = 1;
                    } else {
                        emulator->V[0xF] = 0;
                    }
                    break;
                // 8XYE set vX to vY and shift vX one bit to the left, set vF to the bit shifted out, even if X=F!
                // SCHIP shifts in place - original chip sets to vY first
                case 0xE:
                    switch (emulator->schip) {
                        case true:
                            emulator->V[(code & 0x0F00) >> 8] <<= 1;
                            emulator->V[0xF] = (vx & 0b10000000) >> 7;
                            break;
                        case false:
                            emulator->V[(code & 0x0F00) >> 8] = vy;
                            emulator->V[(code & 0x0F00) >> 8] <<= 1;
                            emulator->V[0xF] = (vy & 0b10000000) >> 7;
                            break;
                    }
                    break;
            }
            break;
        // 9XY0 skip next opcode if vX != vY
        case 0x9000:
            if (vx != vy) {
                emulator->pc += 2;
            }
            break;
        // ANNN set I to NNN
        case 0xA000:
            emulator->I = nnn;
            break;
        // BNNN jump to address XNN + vX
        // Original chip8 sets to NNN + v0; SCHIP is NNN + vX. This isn't documented widely as a quirk
        case 0xB000:
            switch (emulator->schip) {
                case true:
                    emulator->pc = nnn + vx;
                    break;
                case false:
                    emulator->pc = nnn + emulator->V[0];
                    break;
            }
            break;
        // CXNN set vx to a random value masked (bitwise AND) with NN
        case 0xC000:
            int r = rand() % 255 + 1;
            emulator->V[(code & 0x0F00) >> 8] = r & nn;
            break;
        case 0xD000:
            switch (code & 0x000F) {
                // DXY0 draw 16x16 pixel sprite at position vX, vY with data starting at the address in I, I is not changed 
                case 0:
                    switch (emulator->hires) {
                        case true:
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
                        // 16x8 sprite in lores. This may be incorrect.
                        case false:
                            emulator->V[0xF] = 0;
                            for (int r = 0; r < 8 && y + r < 64; r++) {
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
                    }    
                    break;
                // DXYN draw 8xN pixel sprite at position vX, vY with data starting at the address in I, I is not changed
                default:
                    switch (emulator->hires) {
                        // Starting position always wraps; pixels outside of display are cut
                        // hires mode draws to the screen on a 1:1 pixel ratio (128x64 display)
                        // in SCHIP, vF is set to rows with collisions + rows cut off at bottom of screen
                        case true:
                            x = vx % 128;
                            y = vy % 64;
                            emulator->V[0xF] = 0;
                            for (int r = 0; r < n && y + r < 64; r++) {
                                bool collision = false;
                                for (int c = 0; c < 8 && x + c < 128; c++) {
                                    int loc = (y + r) * 128 + (x + c);

                                    row_b = emulator->memory[emulator->I + r];
                                    b = (row_b >> (7 - c)) & 1;

                                    if (emulator->display[loc] & b == 1) collision = true;
                                    emulator->display[loc] ^= b;
                                }
                                if (collision) emulator->V[0xF] += 1;
                                collision = false;
                            }
                            if (y + n > 64) emulator->V[0xF] += y + n - 64;
                            break;
                        // lores mode draws scaled up as the original hardware does (1 pixel is 2x2 block)
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
                                    
                                    if (emulator->V[0xF] == 0) emulator->V[0xF] = emulator->display[loc1] & b;
                                    emulator->display[loc1] ^= b;
                                    if (emulator->V[0xF] == 0) emulator->V[0xF] = emulator->display[loc1 + 1] & b;
                                    emulator->display[loc1 + 1] ^= b;
                                    if (emulator->V[0xF] == 0) emulator->V[0xF] = emulator->display[loc2] & b;
                                    emulator->display[loc2] ^= b;
                                    if (emulator->V[0xF] == 0) emulator->V[0xF] = emulator->display[loc2 + 1] & b;
                                    emulator->display[loc2 + 1] ^= b;
                                }
                            }
                            if (!emulator->schip) emulator->draw = true;
                            break;
                    }
            }
            break;
        case 0xE000:
            switch (code & 0x00FF) {
                // EX9E skip next opcode if key in the lower 4 bits of vX is pressed
                case 0x009E:
                    if (emulator->key[vx & 0xF] == 1) {
                        emulator->pc += 2;
                    }
                    break;
                // EXA1 skip next opcode if key in the lower 4 bits of vX is not pressed
                case 0x00A1:
                    if (emulator->key[vx & 0xF] != 1) {
                        emulator->pc += 2;
                    }
                    break;
            }
            break;
        case 0xF000:
            switch (code & 0x00FF) {
                // FX33 write the value of vX as BCD value at the addresses I, I+1 and I+2
                case 0x0033:
                    uint8_t h = vx / 100;
                    uint8_t t = (vx % 100) / 10;
                    uint8_t d = vx % 10;
                    emulator->memory[emulator->I] = h;
                    emulator->memory[emulator->I + 1] = t;
                    emulator->memory[emulator->I + 2] = d;
                    break;
                // FX55 write the content of v0 to vX at the memory pointed to by I, I is incremented by X+1 
                // CHIP-48/SCHIP1.0 increment I only by X, SCHIP1.1/SCHIP-MODERN not at all
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
                // FX65 read the bytes from memory pointed to by I into the registers v0 to vX, I is incremented by X+1 
                // See above
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
                // FX07 set vX to the value of the delay timer
                case 0x0007:
                    emulator->V[(code & 0x0F00) >> 8] = emulator->delay_timer;
                    break;
                // FX15 set delay timer to vX
                case 0x0015:
                    emulator->delay_timer = vx;
                    break;
                // FX18 set sound timer to vX, sound is played as long as the sound timer reaches zero
                case 0x0018:
                    emulator->sound_timer = vx;
                    break;
                // FX0A wait for a key pressed and released and set vX to it
                case 0x000A:
                    emulator->waiting = true;
                    emulator->wait_register = emulator->opcode;
                    break;
                // FX1E add vX to I
                // Common myth that VF has a carry flag here if I overflows. Results in unexpected behaviour
                case 0x001E:
                    emulator->I += vx;
                    break;
                // FX29set I to the 5 line high hex sprite for the lowest nibble in vX
                case 0x0029:
                    emulator->I = (vx & 0xF) * 5;
                    break;
                // FX30 set I to the 10 lines high hex sprite for the lowest nibble in vX
                case 0x0030:                        
                    emulator->I = (vx & 0xF) * 10;
                    break;
                // FX75 store the content of the registers v0 to vX into flags storage (outside of the addressable ram)
                // These should be continuous between emulator startups - original hardware was between program startups
                // Currently not implemented so would be a good addition
                case 0x0075:
                    u = (((code & 0x0F00) >> 8) > 7) ? 7 : (code & 0x0F00) >> 8;
                    for (int i = 0; i <= u; i++) {
                        emulator->flags[i] = emulator->V[i];
                    }
                    break;
                // FX85 load the registers v0 to vX from flags storage (outside the addressable ram)
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

