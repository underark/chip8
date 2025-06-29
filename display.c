#include <stdbool.h>
#include "chip8.h"
#include "display.h"

bool setup(struct SDLPack* SDLPack) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDLPack->window = SDL_CreateWindow("emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_SHOWN);
    if (!SDLPack->window) {
        printf("SDL failed");
        return false;
    }
    SDLPack->renderer = SDL_CreateRenderer(SDLPack->window, -1, SDL_RENDERER_ACCELERATED);
    if (!SDLPack->renderer) {
        printf("SDL failed");
        return false;
    }
    SDLPack->texture = SDL_CreateTexture(SDLPack->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 64);
    if (!SDLPack->texture) {
        printf("SDL failed");
        return false;
    }
    return true;
}

void update_display(struct Chip8* emulator, struct SDLPack* SDLPack) {
    SDL_Rect scale = {0, 0, 128 * 5, 64 * 5};
    uint32_t buffer[DISPLAY_SIZE];
    to_pixels(emulator->display, buffer);
    SDL_UpdateTexture(SDLPack->texture, NULL, buffer, 128 * sizeof(uint32_t));
    SDL_RenderClear(SDLPack->renderer);
    SDL_SetTextureScaleMode(SDLPack->texture, SDL_ScaleModeNearest);
    SDL_RenderCopy(SDLPack->renderer, SDLPack->texture, NULL, &scale);
    SDL_RenderPresent(SDLPack->renderer);
}

void clear_display(uint8_t display[]) {
    for (int i = 0; i < DISPLAY_SIZE; i++) {
        display[i] = 0;
    }
}

void to_pixels(uint8_t display[], uint32_t buffer[]) {
    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    for (int i = 0; i < DISPLAY_SIZE; i++) {
        buffer[i] = display[i] ? SDL_MapRGBA(format, 0, 255, 255, 255) : SDL_MapRGBA(format, 0, 0, 0, 255);
    }
}
