#pragma once

#include <SDL3/SDL.h>
#include <array>
#include "player.h"
#include "glm.hpp"
using glm::vec3;
using glm::quat;

struct Sample {
    float *buf;
    size_t length;
    float cursor; // position where audio will play in frames
    float volume;
    float pitch; 
    float pan; // audio panning [-1, 1]
    bool delete_on_finished;
    bool is_playing;

    struct {
        bool not_positioned; /*
            true: do not calculate audio positioning
            no panning and distance attenuation
        */
        vec3 vel;
        vec3 pos;
        float roll_off; // how fast the audio attenuates over distance (default: 10%)
    } emitter;

    inline bool finished() {
        return cursor >= length;
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

    struct {
        // vec3 last_pos;
        bool should_compute_velocity;
        vec3 vel;

        vec3 pos;
        vec3 dir;
    } listener;
} AudioCtx;

extern AudioCtx *audio_ctx;

Sample *load_wav(const char *path, float volume, bool delete_on_finished);

void init_audio();

void update_audio(vec3 position, vec3 direction);

void audio_quit();