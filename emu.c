#include "chip8.h"
#include "display.h"
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("./emulator <rom> <mode>\n");
        printf("mode s = schip, c = chip8");
        return 1;
    }


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

    if (strcmp(argv[2], "s") == 0) {
        emulator->schip = true;
        printf("schip on\n");
    } else if (strcmp(argv[2], "c") == 0) {
        emulator->schip = false;
    } else {
        printf("mode s = schip, c = chip8");
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

    SDL_Event e;
    while (emulator->running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                emulator->running = false;
            } else if (e.type == SDL_KEYDOWN) {
                int k = to_key(e.key.keysym.sym);
                if (k >= 0) {
                    emulator->key[k] = 1;
                }
            } else if (e.type == SDL_KEYUP) {
                int k = to_key(e.key.keysym.sym);
                if (k >= 0 && emulator->waiting) {
                    emulator->key[k] = 0;
                    emulator->V[(emulator->wait_register & 0x0F00) >> 8] = k;
                    emulator->waiting = false;
                } else if (k >= 0) {
                    emulator->key[k] = 0;
                }
            }
        }

        if (emulator->sound_timer > 0) {
            emulator->sound_timer--;
        }

        if (emulator->delay_timer > 0) {
            emulator->delay_timer--;
        }

        // Better to fetch -> increment -> execute. Incrementing inside opcodes is unexpected
        if (!emulator->waiting) {
            for (int i = 0; i < 33; i++) {
                emulator->draw = false;
                emulator->opcode = emulator->memory[emulator->pc] << 8 | emulator->memory[emulator->pc + 1];
                emulator->pc += 2;
                decode_execute(emulator->opcode, emulator, SDLPack);
                update_display(emulator, SDLPack);
                if (emulator->waiting || emulator->draw) {
                    break;
                }
            }
        }
        SDL_Delay(16);
    }
    free(emulator);
    SDL_DestroyWindow(SDLPack->window);
    SDL_Quit();
    return 0;
}