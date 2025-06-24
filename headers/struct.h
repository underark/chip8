#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define MEMORY_SIZE 4096
#define DISPLAY_SIZE (64 * 32)
#define REGISTER_SIZE 16

struct Chip8 {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[REGISTER_SIZE];
    uint8_t display[DISPLAY_SIZE];
    uint16_t opcode;
    uint16_t wait_register;
    uint16_t I;
    uint16_t pc;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[REGISTER_SIZE];
    uint16_t sp;
    uint8_t key[REGISTER_SIZE];
    bool waiting;
};

struct SDLPack {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};