#include "audio.h"

AudioCtx *audio_ctx = NULL;

Sample *load_wav(const char *path, float volume, bool delete_on_finished) {
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
    smp->volume = volume;
    smp->delete_on_finished = delete_on_finished;

    SDL_DestroyAudioStream(stream);
    return smp;
}

#define MAX_FRAMES 128

void audio_cb(void *userdata, SDL_AudioStream *stream, int additional_amm, int total_amm) {
    Samples& smp = audio_ctx->samples;

    additional_amm /= sizeof(float); // from bytes to samples
    int frames_requested = additional_amm / 2;

    while (frames_requested > 0) {
        float out[MAX_FRAMES * 2]; // stereo interleaved
        const int frames = SDL_min(frames_requested, MAX_FRAMES);

        SDL_memset(out, 0, frames*2*sizeof(float));
        
        for (Sample* s : smp) {
            if (!s || !s->buf || s->finished())
                continue;

            float vol = s->volume;

            // equal power panning
            float l_gain = SDL_cosf((s->pan + 1.0f) * 0.25f * M_PI) * vol;
            float r_gain = SDL_sinf((s->pan + 1.0f) * 0.25f * M_PI) * vol;

            size_t remain_frames = (s->length - s->cursor) / 2;
            size_t to_mix = SDL_min((size_t)frames, remain_frames);

            for (int i = 0; i < to_mix; i++) {
                float cursor = s->cursor;
                int frame_idx = (int)cursor;

                if (frame_idx + 1 >= s->length / 2) {
                    break;
                }
                
                float frac = cursor - frame_idx;

                float vl = s->buf[frame_idx*2 + 0] * (1.0f - frac) + s->buf[(frame_idx+1)*2 + 0] * frac + 1e-20f;
                float vr = s->buf[frame_idx*2 + 1] * (1.0f - frac) + s->buf[(frame_idx+1)*2 + 1] * frac + 1e-20f;

                out[i*2 + 0] += vl * l_gain;
                out[i*2 + 1] += vr * r_gain;

                // advance fractional cursor
                s->cursor += s->pitch;
            }

            if (s->finished() && s->delete_on_finished) {
                SDL_free(s->buf);
                s->buf = nullptr;
            }
        } // for sample

        // global (soft) limiter
        for (int i = 0; i < frames * 2; i++) {
            float x = out[i];
            out[i] = x / (1.0f + fabsf(x));
        }

        SDL_PutAudioStreamData(stream, out, frames * 2 * sizeof(float));

        frames_requested -= frames;
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
