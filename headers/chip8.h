#include "struct.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

void initialize(struct Chip8* emulator);
bool read_to_memory(char* filename, struct Chip8* emulator);
bool decode_execute(uint16_t code, struct Chip8* emulator, struct SDLPack* SDLPack);
void to_pixels(uint8_t display[], uint32_t buffer[]);
int to_key(SDL_KeyCode key);
bool is_in_bounds(int v1, int v2);
