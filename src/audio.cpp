#include "audio.h"

AudioCtx *audio_ctx = NULL;

Sample *load_wav(const char *path) {
    Sample *smp = (Sample*) calloc(1, sizeof(Sample));

    SDL_IOStream *file = SDL_IOFromFile(path, "rb");
    if (!file) {
        SDL_Log("open error: %s", SDL_GetError());
        return smp;
    }

    SDL_AudioSpec file_spec;
    Uint8 *file_buf = NULL;
    Uint32 file_len = 0;

    if (!SDL_LoadWAV_IO(file, true, &file_spec, &file_buf, &file_len)) {
        SDL_Log("WAV load error: %s", SDL_GetError());
        return smp;
    }

    SDL_AudioStream *stream = SDL_CreateAudioStream(
        &audio_ctx->spec,
        &file_spec
    );

    SDL_PutAudioStreamData(stream, file_buf, file_len);
    SDL_free(file_buf);
    SDL_FlushAudioStream(stream);

    size_t out = SDL_GetAudioStreamAvailable(stream);
    float *data = (float *)SDL_malloc(out);
    SDL_GetAudioStreamData(stream, data, out);

    smp->buf = data;
    smp->length = out / sizeof(float);
    smp->cursor = 0;
    smp->pitch = 1.0;
    smp->volume = 0.1;

    SDL_DestroyAudioStream(stream);
    return smp;
}

void audio_cb(void *userdata, SDL_AudioStream *stream, int additional_amm, int total_amm) {
    static int sample = 0;
    Player *p = (Player*)audio_ctx->ud;
    Sample *s = (Sample*)audio_ctx->smp;

    additional_amm /= sizeof(float); // from bytes to samples

    while (additional_amm > 0 && p) {
        float samples[512];
        const int total = SDL_min(additional_amm, SDL_arraysize(samples));
        int i;
    /*
        for (i = 0; i < total; i++) {
            const int f = 220;
            const float phase = sample * f / 44100.0f;
            samples[i] = SDL_sinf(phase * 2 * SDL_PI_F) * 0.01;
            sample++;
        }
      */  
        if (s && s->buf && !s->finished()) {
            size_t remain = s->length - s->cursor;
            size_t n = SDL_min(total, remain);

            for (i = 0; i < (int)n; i++) {
                float v = s->buf[s->cursor + i];

                // i need to protect my hearing
                if (v > 1.0f)  v = 1.0f;
                if (v < -1.0f) v = -1.0f;

                samples[i] = v * s->volume;
            }

            s->cursor += n;
        }

        if (s && s->finished()) {
            if (s->buf)
                SDL_free(s->buf);
            s->buf = NULL;
        }

        sample %= 44100; // avoid float precision errors;

        SDL_PutAudioStreamData(stream, samples, total * sizeof(float));

        additional_amm -= total;
    }
}

void init_audio() {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec settings = {}; 
    settings.freq = 44100; // Our sampling rate
    settings.format = SDL_AUDIO_F32;
    settings.channels = 2; // stereo

    audio_ctx = (AudioCtx*)calloc(1, sizeof(AudioCtx));

    audio_ctx->spec = settings;
    audio_ctx->stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 
        &settings,
        audio_cb,
        NULL
    );
    if (!audio_ctx->stream) {
        SDL_Log("could not create audio stream: %s", SDL_GetError());
        return;
    }

    SDL_ResumeAudioStreamDevice(audio_ctx->stream);
}

void audio_quit() {
    SDL_DestroyAudioStream(audio_ctx->stream);
}