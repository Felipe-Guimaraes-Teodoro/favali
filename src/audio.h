#pragma once

#include <SDL3/SDL.h>
#include "player.h"

struct Sample {
    float *buf;
    size_t length;
    size_t cursor;
    float volume;
    float pitch; 

    inline bool finished() {
        return !(cursor < length);
    }
};

typedef struct {
    SDL_AudioStream *stream = NULL;
    Player *ud;
    Sample *smp;
    SDL_AudioSpec spec;
} AudioCtx;

extern AudioCtx *audio_ctx;

Sample *load_wav(const char *path);

void init_audio();

void audio_quit();