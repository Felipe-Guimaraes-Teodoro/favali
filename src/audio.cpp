#include "audio.h"
#include "stdio.h"

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
    smp->length = out / (sizeof(float) * 2);
    smp->cursor = 0;
    smp->pitch = 1.0;
    smp->volume = volume;
    smp->delete_on_finished = delete_on_finished;
    smp->emitter.not_positioned = true;
    smp->emitter.roll_off = 0.1;
    smp->is_playing = true;

    SDL_DestroyAudioStream(stream);
    return smp;
}

#define MAX_FRAMES 128

void audio_cb(void *userdata, SDL_AudioStream *stream, int additional_amm, int total_amm) {
    Samples& smp = audio_ctx->samples;

    additional_amm /= sizeof(float); // from bytes to samples
    int frames_requested = additional_amm / 2;

    while (frames_requested > 0 && audio_ctx) {
        float out[MAX_FRAMES * 2]; // stereo interleaved
        const int frames = SDL_min(frames_requested, MAX_FRAMES);

        SDL_memset(out, 0, frames*2*sizeof(float));

        for (Sample* s : smp) {
            if (!s || !s->buf || s->finished() || !s->is_playing)
                continue;

            float l_gain, r_gain, vol, pan = 0.0;
            double pitch = s->pitch;

            if (s->emitter.not_positioned) { /* should not calculate audio positioning? */
                vol = s->volume;
                pan = s->pan;

                // equal power panning
                l_gain = SDL_cosf((pan + 1.0f) * 0.25f * M_PI) * vol;
                r_gain = SDL_sinf((pan + 1.0f) * 0.25f * M_PI) * vol;
            } else {
                float distance = glm::distance(
                    audio_ctx->listener.pos, s->emitter.pos
                ) * s->emitter.roll_off;

                float attenuation = 1.0 / (1.0 + distance*distance);

                vol = s->volume * attenuation;
                vec3 dir = glm::normalize(s->emitter.pos - audio_ctx->listener.pos);
                pan = glm::dot(glm::cross(audio_ctx->listener.dir, UP), dir);
                pan = glm::clamp(pan, -1.0f, 1.0f);

                float v_listener = glm::dot(audio_ctx->listener.vel, dir);
                float v_source = glm::dot(s->emitter.vel, dir);
                float doppler = (SPEED_OF_SOUND + v_listener) / (SPEED_OF_SOUND + v_source);

                pitch *= doppler;
                pitch = glm::clamp(pitch, 0.1, 10.0);

                l_gain = SDL_cosf((pan + 1.0f) * 0.25f * M_PI) * vol;
                r_gain = SDL_sinf((pan + 1.0f) * 0.25f * M_PI) * vol;
            }

            size_t remain_frames = s->length - (size_t)s->cursor;
            size_t to_mix = SDL_min((size_t)frames, remain_frames);

            for (int i = 0; i < to_mix; i++) {
                double cursor = s->cursor;
                size_t frame_idx = (size_t)s->cursor;

                if (frame_idx + 1 >= s->length || s->force_stop) {
                    s->cursor = s->length; // mark as finished
                    break;
                }
                
                double frac = cursor - frame_idx;
                
                // glm::mix(s->buf[frame_idx * 2 + 0], s->buf[(frame_idx + 1) * 2 + 0], frac);
                float vl = s->buf[frame_idx * 2 + 0] * (1.0f - frac)
                    + s->buf[(frame_idx + 1) * 2 + 0] * frac;

                float vr = s->buf[frame_idx * 2 + 1] * (1.0f - frac)
                    + s->buf[(frame_idx + 1) * 2 + 1] * frac;

                out[i*2 + 0] += vl * l_gain;
                out[i*2 + 1] += vr * r_gain;

                // advance fractional cursor
                s->cursor += pitch;
            }

            if (s->finished()) {
                s->is_playing = false;
                if (s->delete_on_finished && s->buf) {
                    SDL_free(s->buf);
                    s->buf = nullptr;
                }
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
    audio_ctx->listener.should_compute_velocity = true;

    SDL_ResumeAudioStreamDevice(audio_ctx->stream);
}

void update_audio(vec3 position, vec3 direction) {
    if (!audio_ctx)
        return;

    // audio_ctx->listener.last_pos = audio_ctx->listener.pos;
    if (audio_ctx->listener.should_compute_velocity)
        audio_ctx->listener.vel = position - audio_ctx->listener.pos;

    audio_ctx->listener.pos = position;
    audio_ctx->listener.dir = direction;
}

void audio_quit() {
    SDL_DestroyAudioStream(audio_ctx->stream);
}
