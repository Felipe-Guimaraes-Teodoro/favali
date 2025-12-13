#pragma once

#include <SDL3/SDL.h>
#include <array>
#include "player.h"

struct Sample {
    float *buf;
    size_t length;
    float cursor; // position where audio will play in frames
    float volume;
    float pitch; 
    float pan; // audio panning [-1, 1]
    bool is_finished;
    bool delete_on_finished;

    inline bool finished() {
        if (!cursor < length) {

        }
        return !(cursor < length);
    }
};

typedef struct {
    void(*callback)(void* ud);
} Synthesizer;


#define MAX_SAMPLE_CHANNELS 16
typedef std::array<Sample*, MAX_SAMPLE_CHANNELS> Samples;

// todo: actual audio positioning
// doppler effect, distance attenuation and panning
typedef struct {
    SDL_AudioStream *stream = NULL;
    Player *ud;
    Samples samples;
    // todo: synthesizer, where on the user defines how its frames
    // should be generated on the fly
    SDL_AudioSpec spec;
} AudioCtx;

extern AudioCtx *audio_ctx;

Sample *load_wav(const char *path, float volume, bool delete_on_finished);

void init_audio();

void audio_quit();