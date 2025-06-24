#include "chip8.h"
#include "display.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
    struct SDLPack* SDLPack = malloc(sizeof(struct SDLPack));
    if (!SDLPack) {
        printf("Unable to create SDL environment");
        return 1;
    }
    struct Chip8 *emulator = malloc(sizeof(struct Chip8));
    if (!emulator) {
        printf("Unable to create emulator");
        return 1;
    }
    bool SDLsetup = setup(SDLPack);
    if (!SDLsetup) {
        printf("Unable to create SDL environment");
        return 1;
    }
    initialize(emulator);
    bool read = read_to_memory(argv[1], emulator);
    if (!read) {
        printf("Unable to read rom");
        return 1;
    }

    emulator->pc = 0x200;
    uint32_t cpu_tick, timer_tick = SDL_GetTicks();
    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                int k = to_key(e.key.keysym.sym);
                emulator->key[k] = 1;
            } else if (e.type == SDL_KEYUP) {
                int k = to_key(e.key.keysym.sym);
                emulator->key[k] = 0;
                if (emulator->waiting) {
                    emulator->V[(emulator->wait_register & 0x0F00) >> 8] = k;
                    emulator->waiting = false;
                };
            }
        }

        if (emulator->sound_timer > 0) {
            emulator->sound_timer--;
        }

        if (emulator->delay_timer > 0) {
            emulator->delay_timer--;
        }

        if (!emulator->waiting) {
            for (int i = 0; i < 11; i++) {
                printf("opcode is %x\n", emulator->opcode);
                emulator->opcode = emulator->memory[emulator->pc] << 8 | emulator->memory[emulator->pc + 1];
                decode_execute(emulator->opcode, emulator, SDLPack);
            }
        }
        SDL_Delay(16);
    }
    free(emulator);
    SDL_DestroyWindow(SDLPack->window);
    SDL_Quit();
    return 0;
}