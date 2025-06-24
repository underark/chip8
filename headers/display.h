#include <SDL2/SDL.h>
#include <stdbool.h>
#include "struct.h"

bool setup(struct SDLPack* SDLPack);
void update_display(struct Chip8* emulator, struct SDLPack* SDLPack);
void to_pixels(uint8_t display[], uint32_t buffer[]);