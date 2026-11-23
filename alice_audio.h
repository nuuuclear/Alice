/* alice_audio.h - a small library for your audio needs

In exactly ONE source file:
    #define ALICE_AUDIO_IMPLEMENTATION
    #include "alice_audio.h"

Backends (selected automatically):
    Windows (WinMM)
        link -lwinmm
    Linux (ALSA)
        link -lasound -lpthread
    macOS (AudioQueue)
        link -framework AudioToolbox -framework CoreFoundation

Other platforms and custom backends:
    before the implementation include:
    #define ALICE_AUDIO_NO_BACKEND

    No device will be opened
    You can feed you own audio callback (such as SDL, JACK, audio file writer, etc..) by calling alice_audio_read()

Optional configuration macros (define before the implementation include):
    ALICE_AUDIO_STATIC
        make all functions static
    ALICE_AUDIO_MALLOC / ALICE_AUDIO_CALLOC / ALICE_AUDIO_FREE
        override the allocator

How it works:
    push samples into a software queue with alice_audio_write()
    a backend drain that queue into the output devic
    if the queue runs empty, the device will go silent until new samples are pushed

all functions are thread safe and may be called from any thread
synth voices are plain structs and are not thread safe
*/
#ifndef ALICE_AUDIO_H_INCLUDED
#define ALICE_AUDIO_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

#ifndef ALICE_AUDIO_API
    #ifdef ALICE_AUDIO_STATIC
        #define ALICE_AUDIO_API static
    #else
        #define ALICE_AUDIO_API extern
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
//   Type   ////////////////////////////////////////
////////////////////////////////////////////

typedef struct alice_audio alice_audio;

typedef enum alice_audio_result {
    ALICE_AUDIO_OK = 0,
    ALICE_AUDIO_ERROR_ARGUMENT,
    ALICE_AUDIO_ERROR_MEMORY,
    ALICE_AUDIO_ERROR_DEVICE,
    ALICE_AUDIO_ERROR_THREAD,
    ALICE_AUDIO_ERROR_UNSUPPORTED
} alice_audio_result;

typedef struct alice_audio_config {
    uint32_t sample_rate;    // usually 44100 or 48000
    uint32_t channels;
    uint32_t buffer_frames;  // software queue size
    uint32_t period_frames;  // chunk size
    const char *device_name; // ALSA device name
    uint8_t start_playing;   // nonzero starts playback
    uint8_t low_latency;     // nonzero selects smaller periods
} alice_audio_config;

typedef enum alice_waveform {
    ALICE_WAVE_SINE = 0,
    ALICE_WAVE_SQUARE,
    ALICE_WAVE_TRIANGLE,
    ALICE_WAVE_SAWTOOTH,
    ALICE_WAVE_NOISE
} alice_waveform;

typedef struct alice_synth_voice {
    double   frequency;       // Hz
    double   sample_rate;     // Samples per second
    double   amplitude;       // 0.0 to 1.0
    double   attack_seconds;
    double   release_seconds;
    uint32_t waveform;        // alice_waveform
    uint32_t gate;            // nonzero enables the note

    // internal use
    double   phase;           // fraction of a cycle (0.0 to 1.0)
    double   phase_step;      // cycles per sample (frequency / sample_rate)
    double   envelope;        // the envelope level (0.0 to 1.0)
    uint32_t noise_state;     // xorshift32 state
} alice_synth_voice;

////////////////////////////////////////////////////////////
//   Audio output   ////////////////////////////////
////////////////////////////////////////////

// initialize the audio engine with a config
ALICE_AUDIO_API void alice_audio_config_init(alice_audio_config *config);

// open an output device
// returns NULL upon failure
ALICE_AUDIO_API alice_audio *alice_audio_open(const alice_audio_config *config);

// close the audio devide
// NULL tolerated
ALICE_AUDIO_API void alice_audio_close(alice_audio *audio);

// Start playback
ALICE_AUDIO_API alice_audio_result alice_audio_start(alice_audio *audio);

// Pause playback, preserving the sample queue
ALICE_AUDIO_API alice_audio_result alice_audio_pause(alice_audio *audio);

// push samples to the queue
// samples are interleaved signed 16-bit PCM
// returns the number of frames accepted (sometimes none or less)
ALICE_AUDIO_API uint32_t alice_audio_write(alice_audio *audio, const int16_t *samples, uint32_t frame_count);

// same as alice_audio_write() for interleaved float samples, clamped to [-1, 1]
ALICE_AUDIO_API uint32_t alice_audio_write_float(alice_audio *audio, const float *samples, uint32_t frame_count);

// discards frames from the queue up to frame_count and copies them out
// returns the number of frames copied
// when using ALICE_AUDIO_NO_BACKEND, this is how you'd drain the queue to you own audio callback
ALICE_AUDIO_API uint32_t alice_audio_read(alice_audio *audio, int16_t *out, uint32_t frame_count);

// currently queued frames
ALICE_AUDIO_API uint32_t alice_audio_queued(const alice_audio *audio);

ALICE_AUDIO_API uint32_t alice_audio_capacity(const alice_audio *audio);
ALICE_AUDIO_API uint32_t alice_audio_sample_rate(const alice_audio *audio);
ALICE_AUDIO_API uint32_t alice_audio_channels(const alice_audio *audio);

// discard all queued samples
ALICE_AUDIO_API alice_audio_result alice_audio_flush(alice_audio *audio);

// last error (0 if there was none)
ALICE_AUDIO_API int32_t alice_audio_last_error(const alice_audio *audio);

//// synth
ALICE_AUDIO_API void alice_synth_init(alice_synth_voice *voice, uint32_t sample_rate, alice_waveform waveform);
ALICE_AUDIO_API void alice_synth_set_frequency(alice_synth_voice *voice, double frequency_hz);
// standard midi, A4 is note 69 at 440 Hz
ALICE_AUDIO_API double alice_synth_midi_frequency(int midi_note);
ALICE_AUDIO_API void alice_synth_set_amplitude(alice_synth_voice *voice, double amplitude);
ALICE_AUDIO_API void alice_synth_gate(alice_synth_voice *voice, int enabled);
ALICE_AUDIO_API void alice_synth_set_envelope(alice_synth_voice *voice, double attack_seconds, double release_seconds);
ALICE_AUDIO_API float alice_synth_next(alice_synth_voice *voice);
ALICE_AUDIO_API void alice_synth_mix(alice_synth_voice *voice, float *output, uint32_t frames, uint32_t channels, float pan);

#ifdef __cplusplus
}
#endif

#endif /* ALICE_AUDIO_H_INCLUDED */

////////////////////////////////////////////////////////////
//   Implementation   //////////////////////////////
////////////////////////////////////////////

#ifdef ALICE_AUDIO_IMPLEMENTATION
#ifndef ALICE_AUDIO_IMPLEMENTATION_ONCE
#define ALICE_AUDIO_IMPLEMENTATION_ONCE

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef ALICE_AUDIO_MALLOC
    #define ALICE_AUDIO_MALLOC(size)        malloc(size)
    #define ALICE_AUDIO_CALLOC(count, size) calloc((count), (size))
    #define ALICE_AUDIO_FREE(ptr)           free(ptr)
#endif

// platform selector
#if defined(ALICE_AUDIO_NO_BACKEND)
    #define ALICE_BACKEND_NONE
#elif defined(_WIN32)
    #define ALICE_BACKEND_WINMM
#elif defined(__APPLE__)
    #define ALICE_BACKEND_COREAUDIO
#elif defined(__linux__)
    #define ALICE_BACKEND_ALSA
#else
    #error "alice_audio.h: unsupported platform. Define ALICE_AUDIO_NO_BACKEND and use alice_audio_read()."
#endif

#if defined(ALICE_BACKEND_WINMM)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <mmsystem.h>
    #ifdef _MSC_VER
    #pragma comment(lib, "winmm.lib")
    #endif

    typedef CRITICAL_SECTION alice_mutex;
    #define alice_mutex_init(m)    InitializeCriticalSection(m)
    #define alice_mutex_destroy(m) DeleteCriticalSection(m)
    #define alice_mutex_lock(m)    EnterCriticalSection(m)
    #define alice_mutex_unlock(m)  LeaveCriticalSection(m)
#else
    #include <pthread.h>

    typedef pthread_mutex_t alice_mutex;
    #define alice_mutex_init(m)    pthread_mutex_init((m), NULL)
    #define alice_mutex_destroy(m) pthread_mutex_destroy(m)
    #define alice_mutex_lock(m)    pthread_mutex_lock(m)
    #define alice_mutex_unlock(m)  pthread_mutex_unlock(m)
#endif

#if defined(ALICE_BACKEND_ALSA)
    #include <errno.h>
    #include <poll.h>
    #include <alsa/asoundlib.h>
    #define alice_sleep_ms(ms) poll(NULL, 0, (ms))
#endif

#if defined(ALICE_BACKEND_COREAUDIO)
    #include <AudioToolbox/AudioToolbox.h>
#endif

// shared state

#define ALICE_DEFAULT_RATE          48000u
#define ALICE_DEFAULT_QUEUE_FRAMES  2048u
#define ALICE_MIN_PERIOD_FRAMES     32u
#define ALICE_MAX_QUEUE_FRAMES      (1u << 24)

struct alice_audio {
    uint32_t    rate;
    uint32_t    channels;
    uint32_t    capacity;   // queue size in frames
    uint32_t    period;     // chunk size in frames

    // lock protected
    alice_mutex lock;
    int16_t    *ring;       // capacity * channels samples
    uint32_t    read_pos;   // in frames
    uint32_t    write_pos;  // in frames
    uint32_t    queued;     // in frames
    int         playing;
    int         stop;
    int32_t     error;

    // backend stuff
#if defined(ALICE_BACKEND_WINMM)
    #define ALICE_WINMM_BLOCKS 4
    HWAVEOUT   wave;
    HANDLE     event;
    HANDLE     thread;
    WAVEHDR    headers[ALICE_WINMM_BLOCKS];
    int16_t   *blocks[ALICE_WINMM_BLOCKS];
    uint32_t   block_frames;
#elif defined(ALICE_BACKEND_ALSA)
    snd_pcm_t *pcm;
    pthread_t  thread;
    int        thread_started;
    int        can_pause;
    int16_t   *scratch;
#elif defined(ALICE_BACKEND_COREAUDIO)
    #define ALICE_COREAUDIO_BUFFERS 3
    AudioQueueRef       queue;
    AudioQueueBufferRef buffers[ALICE_COREAUDIO_BUFFERS];
#endif
};

#if !defined(ALICE_BACKEND_NONE)
static void alice_audio_set_error(alice_audio *a, int32_t code) {
    alice_mutex_lock(&a->lock);
    a->error = code;
    alice_mutex_unlock(&a->lock);
}

static void alice_audio_get_flags(alice_audio *a, int *playing, int *stop) {
    alice_mutex_lock(&a->lock);
    *playing = a->playing;
    *stop    = a->stop;
    alice_mutex_unlock(&a->lock);
}
#endif

// ring buffer

uint32_t alice_audio_write(alice_audio *a, const int16_t *samples, uint32_t frames) {
    uint32_t count, first;
    size_t ch;

    if (!a || !samples || frames == 0) return 0;
    ch = a->channels;

    alice_mutex_lock(&a->lock);
    count = a->capacity - a->queued;
    if (count > frames)
        count = frames;

    first = a->capacity - a->write_pos;
    if (first > count)
        first = count;

    memcpy(a->ring + (size_t)a->write_pos * ch, samples, (size_t)first * ch * sizeof(int16_t));
    if (count > first)
        memcpy(a->ring, samples + (size_t)first * ch, (size_t)(count - first) * ch * sizeof(int16_t));

    a->write_pos = (a->write_pos + count) % a->capacity;
    a->queued += count;
    alice_mutex_unlock(&a->lock);

    return count;
}

uint32_t alice_audio_read(alice_audio *a, int16_t *out, uint32_t frames) {
    uint32_t count, first;
    size_t ch;

    if (!a || !out || frames == 0) return 0;
    ch = a->channels;

    alice_mutex_lock(&a->lock);
    count = a->queued < frames ? a->queued : frames;

    first = a->capacity - a->read_pos;
    if (first > count)
        first = count;

    memcpy(out, a->ring + (size_t)a->read_pos * ch, (size_t)first * ch * sizeof(int16_t));
    if (count > first) {
        memcpy(out + (size_t)first * ch, a->ring, (size_t)(count - first) * ch * sizeof(int16_t));
    }

    a->read_pos = (a->read_pos + count) % a->capacity;
    a->queued -= count;
    alice_mutex_unlock(&a->lock);

    return count;
}

uint32_t alice_audio_queued(const alice_audio *audio) {
    alice_audio *a = (alice_audio *)audio;
    uint32_t n;

    if (!a) return 0;

    alice_mutex_lock(&a->lock);
    n = a->queued;
    alice_mutex_unlock(&a->lock);

    return n;
}

alice_audio_result alice_audio_flush(alice_audio *a) {
    if (!a) return ALICE_AUDIO_ERROR_ARGUMENT;

    alice_mutex_lock(&a->lock);
    a->read_pos = 0;
    a->write_pos = 0;
    a->queued = 0;
    alice_mutex_unlock(&a->lock);
    return ALICE_AUDIO_OK;
}

uint32_t alice_audio_capacity(const alice_audio *a)    { return a ? a->capacity : 0; }
uint32_t alice_audio_sample_rate(const alice_audio *a) { return a ? a->rate : 0; }
uint32_t alice_audio_channels(const alice_audio *a)    { return a ? a->channels : 0; }

int32_t alice_audio_last_error(const alice_audio *audio) {
    alice_audio *a = (alice_audio *)audio;
    int32_t error;

    if (!a) return ALICE_AUDIO_ERROR_ARGUMENT;

    alice_mutex_lock(&a->lock);
    error = a->error;
    alice_mutex_unlock(&a->lock);
    
    return error;
}

uint32_t alice_audio_write_float(alice_audio *a, const float *samples, uint32_t frames) {
    enum { CHUNK_FRAMES = 512 };
    int16_t chunk[CHUNK_FRAMES * 2];
    uint32_t total = 0;
    uint32_t channels;

    if (!a || !samples || frames == 0) return 0;
    channels = a->channels;

    while (total < frames) {
        uint32_t count = frames - total;
        uint32_t accepted, i;
        const float *src;

        if (count > CHUNK_FRAMES)
            count = CHUNK_FRAMES;
        src = samples + (size_t)total * channels;

        for (i = 0; i < count * channels; ++i) {
            float x = src[i];
            if (!(x == x))
                x = 0.0f;
            if (x > 1.0f)
                x = 1.0f;
            if (x < -1.0f)
                x = -1.0f;
            chunk[i] = (int16_t)(x * 32767.0f + (x >= 0.0f ? 0.5f : -0.5f));
        }

        accepted = alice_audio_write(a, chunk, count);
        total += accepted;
        if (accepted < count)
            break;
    }
    return total;
}

#if defined(ALICE_BACKEND_WINMM)

static uint32_t alice_winmm_blocks_in_flight(const alice_audio *a) {
    uint32_t i, n = 0;
    for (i = 0; i < ALICE_WINMM_BLOCKS; ++i)
        if (a->headers[i].dwFlags & WHDR_INQUEUE)
            ++n;
    return n;
}

static int alice_winmm_submit(alice_audio *a, uint32_t slot) {
    WAVEHDR *h = &a->headers[slot];
    uint32_t available, frames;
    size_t frame_bytes = (size_t)a->channels * sizeof(int16_t);

    if (h->dwFlags & WHDR_INQUEUE)
        return 0;

    available = alice_audio_queued(a);
    if (available == 0)
        return 0;
    if (available < a->block_frames && alice_winmm_blocks_in_flight(a) > 1)
        return 0;

    if (h->dwFlags & WHDR_PREPARED)
        waveOutUnprepareHeader(a->wave, h, sizeof(*h));

    frames = alice_audio_read(a, a->blocks[slot], a->block_frames);
    if (frames == 0) return 0;

    ZeroMemory(h, sizeof(*h));
    h->lpData = (LPSTR)a->blocks[slot];
    h->dwBufferLength = (DWORD)(frames * frame_bytes);

    if (waveOutPrepareHeader(a->wave, h, sizeof(*h)) != MMSYSERR_NOERROR
    ||  waveOutWrite(a->wave, h, sizeof(*h)) != MMSYSERR_NOERROR
    ) {
        alice_audio_set_error(a, -1);
        return 0;
    }

    return 1;
}

static DWORD WINAPI alice_winmm_worker(LPVOID arg) {
    alice_audio *a = (alice_audio *)arg;
    uint32_t next_slot = 0;

    for (;;) {
        int playing, stop;
        alice_audio_get_flags(a, &playing, &stop);
        if (stop) break;

        if (playing) {
            while (alice_winmm_submit(a, next_slot)) {
                next_slot = (next_slot + 1) % ALICE_WINMM_BLOCKS;
            }
        }

        WaitForSingleObject(a->event, 2);
    }
    return 0;
}

static void alice_backend_close(alice_audio *a) {
    uint32_t i;

    if (a->thread) {
        alice_mutex_lock(&a->lock);
        a->stop = 1;
        alice_mutex_unlock(&a->lock);
        SetEvent(a->event);
        WaitForSingleObject(a->thread, INFINITE);
        CloseHandle(a->thread);
        a->thread = NULL;
    }
    if (a->wave) {
        waveOutReset(a->wave);
        for (i = 0; i < ALICE_WINMM_BLOCKS; ++i)
            if (a->headers[i].dwFlags & WHDR_PREPARED)
                waveOutUnprepareHeader(a->wave, &a->headers[i], sizeof(WAVEHDR));
        waveOutClose(a->wave);
        a->wave = NULL;
    }

    if (a->event) CloseHandle(a->event);
    
    for (i = 0; i < ALICE_WINMM_BLOCKS; ++i) {
        ALICE_AUDIO_FREE(a->blocks[i]);
    }
}

static alice_audio_result alice_backend_open(alice_audio *a, const alice_audio_config *cfg) {
    WAVEFORMATEX fmt;
    uint32_t i;
    (void)cfg;

    a->block_frames = a->period;
    for (i = 0; i < ALICE_WINMM_BLOCKS; ++i) {
        a->blocks[i] = (int16_t *)ALICE_AUDIO_CALLOC((size_t)a->block_frames * a->channels, sizeof(int16_t));
        if (!a->blocks[i])
            return ALICE_AUDIO_ERROR_MEMORY;
    }

    a->event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!a->event) return ALICE_AUDIO_ERROR_THREAD;

    ZeroMemory(&fmt, sizeof(fmt));
    fmt.wFormatTag      = WAVE_FORMAT_PCM;
    fmt.nChannels       = (WORD)a->channels;
    fmt.nSamplesPerSec  = a->rate;
    fmt.wBitsPerSample  = 16;
    fmt.nBlockAlign     = (WORD)(a->channels * sizeof(int16_t));
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
    if (waveOutOpen(&a->wave, WAVE_MAPPER, &fmt, (DWORD_PTR)a->event, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR) {
        a->wave = NULL;
        return ALICE_AUDIO_ERROR_DEVICE;
    }

    a->thread = CreateThread(NULL, 0, alice_winmm_worker, a, 0, NULL);
    if (!a->thread) return ALICE_AUDIO_ERROR_THREAD;
    
    return ALICE_AUDIO_OK;
}

static void alice_backend_set_playing(alice_audio *a, int playing) {
    if (!a->wave) return;
    if (playing)
        waveOutRestart(a->wave);
    else
        waveOutPause(a->wave);
    
    SetEvent(a->event);
}

#elif defined(ALICE_BACKEND_ALSA)

static void alice_alsa_write_all(alice_audio *a, const int16_t *data, uint32_t frames) {
    while (frames > 0) {
        snd_pcm_sframes_t r = snd_pcm_writei(a->pcm, data, frames);
        if (r < 0) {
            int recovered = snd_pcm_recover(a->pcm, (int)r, 1);
            if (recovered < 0)
            {
                alice_audio_set_error(a, (int32_t)recovered);
                alice_sleep_ms(10);
                return;
            }
            continue;
        }

        data   += (size_t)r * a->channels;
        frames -= (uint32_t)r;
    }
}

static void *alice_alsa_worker(void *arg) {
    alice_audio *a = (alice_audio *)arg;
    int device_paused = 0;

    for (;;) {
        int playing, stop;
        uint32_t frames;

        alice_audio_get_flags(a, &playing, &stop);
        if (stop) break;
        if (!playing) {
            if (!device_paused) {
                if (a->can_pause) {
                    snd_pcm_pause(a->pcm, 1);
                } else {
                    snd_pcm_drop(a->pcm);
                    snd_pcm_prepare(a->pcm);
                }
                device_paused = 1;
            }
            alice_sleep_ms(5);
            continue;
        }
        if (device_paused) {
            if (a->can_pause) snd_pcm_pause(a->pcm, 0);

            device_paused = 0;
        }

        frames = alice_audio_read(a, a->scratch, a->period);
        if (frames == 0) {
            alice_sleep_ms(1);
            continue;
        }

        alice_alsa_write_all(a, a->scratch, frames);
    }

    return NULL;
}

static void alice_backend_close(alice_audio *a) {
    if (a->thread_started) {
        alice_mutex_lock(&a->lock);
        a->stop = 1;
        alice_mutex_unlock(&a->lock);
        pthread_join(a->thread, NULL);
        a->thread_started = 0;
    }
    if (a->pcm) {
        snd_pcm_close(a->pcm);
        a->pcm = NULL;
    }
    ALICE_AUDIO_FREE(a->scratch);
    a->scratch = NULL;
}

static alice_audio_result alice_backend_open(alice_audio *a, const alice_audio_config *cfg) {
    snd_pcm_hw_params_t *hw;
    unsigned int rate;
    snd_pcm_uframes_t period, buffer;
    int dir = 0;

    if (snd_pcm_open(&a->pcm, cfg->device_name ? cfg->device_name : "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        a->pcm = NULL;
        return ALICE_AUDIO_ERROR_DEVICE;
    }

    snd_pcm_hw_params_alloca(&hw);
    if (snd_pcm_hw_params_any(a->pcm, hw) < 0
    ||  snd_pcm_hw_params_set_access(a->pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED) < 0
    ||  snd_pcm_hw_params_set_format(a->pcm, hw, SND_PCM_FORMAT_S16_LE) < 0
    ||  snd_pcm_hw_params_set_channels(a->pcm, hw, a->channels) < 0
    ) return ALICE_AUDIO_ERROR_DEVICE;

    snd_pcm_hw_params_set_rate_resample(a->pcm, hw, 1);
    rate = a->rate;
    if (snd_pcm_hw_params_set_rate_near(a->pcm, hw, &rate, &dir) < 0)
        return ALICE_AUDIO_ERROR_DEVICE;
    
    a->rate = rate;

    period = a->period;
    if (snd_pcm_hw_params_set_period_size_near(a->pcm, hw, &period, &dir) < 0)
        return ALICE_AUDIO_ERROR_DEVICE;

    buffer = period * 4;
    if (buffer < a->rate / 50)
        buffer = a->rate / 50;

    if (snd_pcm_hw_params_set_buffer_size_near(a->pcm, hw, &buffer) < 0)
        return ALICE_AUDIO_ERROR_DEVICE;

    if (snd_pcm_hw_params(a->pcm, hw) < 0)
        return ALICE_AUDIO_ERROR_DEVICE;

    snd_pcm_hw_params_get_period_size(hw, &period, &dir);
    a->period = (uint32_t)period;
    a->can_pause = snd_pcm_hw_params_can_pause(hw) != 0;

    if (snd_pcm_prepare(a->pcm) < 0)
        return ALICE_AUDIO_ERROR_DEVICE;

    a->scratch = (int16_t *)ALICE_AUDIO_CALLOC((size_t)a->period * a->channels, sizeof(int16_t));
    if (!a->scratch)
        return ALICE_AUDIO_ERROR_MEMORY;

    if (pthread_create(&a->thread, NULL, alice_alsa_worker, a) != 0)
        return ALICE_AUDIO_ERROR_THREAD;

    a->thread_started = 1;

    return ALICE_AUDIO_OK;
}

static void alice_backend_set_playing(alice_audio *a, int playing) {
    (void)a;
    (void)playing;
}

// TODO: fix, it's buggy
#elif defined(ALICE_BACKEND_COREAUDIO)

static void alice_coreaudio_callback(void *user, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    alice_audio *a = (alice_audio *)user;
    uint32_t capacity_frames = buffer->mAudioDataBytesCapacity / (a->channels * sizeof(int16_t));
    uint32_t frames = alice_audio_read(a, (int16_t *)buffer->mAudioData, capacity_frames);

    if (frames < capacity_frames) {
        memset(
            (int16_t *)buffer->mAudioData + (size_t)frames * a->channels,
            0,
            (size_t)(capacity_frames - frames) * a->channels * sizeof(int16_t)
        );
    }
    buffer->mAudioDataByteSize = capacity_frames * a->channels * (uint32_t)sizeof(int16_t);
    AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
}

static void alice_backend_close(alice_audio *a) {
    if (a->queue) {
        AudioQueueStop(a->queue, true);
        AudioQueueDispose(a->queue, true);
        a->queue = NULL;
    }
}

static alice_audio_result alice_backend_open(alice_audio *a, const alice_audio_config *cfg) {
    AudioStreamBasicDescription fmt;
    uint32_t i, bytes;

    memset(&fmt, 0, sizeof(fmt));
    fmt.mSampleRate       = (Float64)a->rate;
    fmt.mFormatID         = kAudioFormatLinearPCM;
    fmt.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    fmt.mBitsPerChannel   = 16;
    fmt.mChannelsPerFrame = a->channels;
    fmt.mFramesPerPacket  = 1;
    fmt.mBytesPerFrame    = a->channels * (uint32_t)sizeof(int16_t);
    fmt.mBytesPerPacket   = fmt.mBytesPerFrame;

    if (AudioQueueNewOutput(&fmt, alice_coreaudio_callback, a, NULL, NULL, 0, &a->queue) != 0) {
        a->queue = NULL;
        return ALICE_AUDIO_ERROR_DEVICE;
    }

    bytes = a->period * fmt.mBytesPerFrame;
    for (i = 0; i < ALICE_COREAUDIO_BUFFERS; ++i) {
        if (AudioQueueAllocateBuffer(a->queue, bytes, &a->buffers[i]) != 0) {
            return ALICE_AUDIO_ERROR_MEMORY;
        }
        a->buffers[i]->mAudioDataByteSize = bytes;
        memset(a->buffers[i]->mAudioData, 0, bytes);
        AudioQueueEnqueueBuffer(a->queue, a->buffers[i], 0, NULL);
    }

    if (cfg->start_playing && AudioQueueStart(a->queue, NULL) != 0)
        return ALICE_AUDIO_ERROR_DEVICE;

    return ALICE_AUDIO_OK;
}

static void alice_backend_set_playing(alice_audio *a, int playing) {
    if (!a->queue) return;
    if (playing)
        AudioQueueStart(a->queue, NULL);
    else
        AudioQueuePause(a->queue);
}

#else // No backend

static void alice_backend_close(alice_audio *a) { (void)a; }
static alice_audio_result alice_backend_open(alice_audio *a, const alice_audio_config *cfg) {
    (void)a;
    (void)cfg;
    return ALICE_AUDIO_OK;
}
static void alice_backend_set_playing(alice_audio *a, int playing) { (void)a; (void)playing; }

#endif /* backends */

void alice_audio_config_init(alice_audio_config *config) {
    if (!config) return;
    config->sample_rate   = ALICE_DEFAULT_RATE;
    config->channels      = 2;
    config->buffer_frames = ALICE_DEFAULT_QUEUE_FRAMES;
    config->period_frames = 128;
    config->device_name   = NULL;
    config->start_playing = 1;
    config->low_latency   = 1;
}

static int alice_audio_sanitize_config(alice_audio_config *c) {
    uint32_t min_period = ALICE_MIN_PERIOD_FRAMES;

    if (c->sample_rate == 0 || (c->channels != 1 && c->channels != 2))
        return 0;
    if (c->buffer_frames > ALICE_MAX_QUEUE_FRAMES)
        return 0;

    if (c->buffer_frames == 0)
        c->buffer_frames = ALICE_DEFAULT_QUEUE_FRAMES;
    if (c->period_frames == 0)
        c->period_frames = c->low_latency ? 128 : 512;

#if defined(ALICE_BACKEND_WINMM)
    // WinMM blocks that are shorter than 10ms have issues
    if (min_period < c->sample_rate / 100)
        min_period = c->sample_rate / 100;
#endif
    if (c->period_frames < min_period)
        c->period_frames = min_period;

    if (c->buffer_frames < c->period_frames * 2)
        c->buffer_frames = c->period_frames * 2;

    return 1;
}

alice_audio *alice_audio_open(const alice_audio_config *input) {
    alice_audio_config cfg;
    alice_audio *a;

    alice_audio_config_init(&cfg);
    if (input) cfg = *input;

    if (!alice_audio_sanitize_config(&cfg))
        return NULL;

    a = (alice_audio *)ALICE_AUDIO_CALLOC(1, sizeof(*a));
    if (!a) return NULL;
    a->rate     = cfg.sample_rate;
    a->channels = cfg.channels;
    a->capacity = cfg.buffer_frames;
    a->period   = cfg.period_frames;
    a->playing  = cfg.start_playing != 0;

    a->ring = (int16_t *)ALICE_AUDIO_CALLOC((size_t)a->capacity * a->channels, sizeof(int16_t));
    if (!a->ring) {
        ALICE_AUDIO_FREE(a);
        return NULL;
    }
    alice_mutex_init(&a->lock);

    if (alice_backend_open(a, &cfg) != ALICE_AUDIO_OK) {
        alice_backend_close(a);
        alice_mutex_destroy(&a->lock);
        ALICE_AUDIO_FREE(a->ring);
        ALICE_AUDIO_FREE(a);
        return NULL;
    }

    return a;
}

void alice_audio_close(alice_audio *a) {
    if (!a) return;
    alice_backend_close(a);
    alice_mutex_destroy(&a->lock);
    ALICE_AUDIO_FREE(a->ring);
    ALICE_AUDIO_FREE(a);
}

static alice_audio_result alice_audio_set_playing(alice_audio *a, int playing) {
    if (!a) return ALICE_AUDIO_ERROR_ARGUMENT;

    alice_mutex_lock(&a->lock);
    a->playing = playing;
    alice_mutex_unlock(&a->lock);
    alice_backend_set_playing(a, playing);

    return ALICE_AUDIO_OK;
}

alice_audio_result alice_audio_start(alice_audio *a) { return alice_audio_set_playing(a, 1); }
alice_audio_result alice_audio_pause(alice_audio *a) { return alice_audio_set_playing(a, 0); }

////////////////////////////////////////////////////////////
//   Synth   ///////////////////////////////////////
////////////////////////////////////////////

#define ALICE_PI 3.14159265358979323846

static double alice_poly_blep(double t, double dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0;
    }

    if (t > 1.0 - dt) {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }
    return 0.0;
}

static double alice_clamp(double x, double lo, double hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

static double alice_synth_rate(const alice_synth_voice *v) {
    return v->sample_rate > 0.0 ? v->sample_rate : (double)ALICE_DEFAULT_RATE;
}

void alice_synth_init(alice_synth_voice *v, uint32_t rate, alice_waveform wave) {
    if (!v) return;
    memset(v, 0, sizeof(*v));
    v->sample_rate     = rate ? (double)rate : (double)ALICE_DEFAULT_RATE;
    v->amplitude       = 0.25;
    v->attack_seconds  = 0.002;    // short ramp to avoid on/off clicking
    v->release_seconds = 0.02;
    v->waveform        = (uint32_t)wave;
    v->noise_state     = 0x1234ABCDu;
    alice_synth_set_frequency(v, 440.0);
}

double alice_synth_midi_frequency(int note) {
    return 440.0 * pow(2.0, ((double)note - 69.0) / 12.0);
}

void alice_synth_set_frequency(alice_synth_voice *v, double hz) {
    if (!v) return;

    v->frequency  = alice_clamp(hz, 0.0, alice_synth_rate(v) * 0.5);
    v->phase_step = v->frequency / alice_synth_rate(v);
}

void alice_synth_set_amplitude(alice_synth_voice *v, double amp) {
    if (v) v->amplitude = alice_clamp(amp, 0.0, 1.0);
}

void alice_synth_gate(alice_synth_voice *v, int enabled) {
    if (v) v->gate = enabled ? 1u : 0u;
}

void alice_synth_set_envelope(alice_synth_voice *v, double attack, double release) {
    if (!v) return;
    v->attack_seconds  = attack  > 0.0 ? attack  : 0.0;
    v->release_seconds = release > 0.0 ? release : 0.0;
}

static double alice_synth_oscillator(alice_synth_voice *v) {
    double t = v->phase;
    double dt = v->phase_step;
    double x;

    switch (v->waveform) {
    case ALICE_WAVE_SQUARE:
        x = t < 0.5 ? 1.0 : -1.0;
        x += alice_poly_blep(t, dt);
        x -= alice_poly_blep(t < 0.5 ? t + 0.5 : t - 0.5, dt);
        return x;

    case ALICE_WAVE_TRIANGLE:
        return 1.0 - 4.0 * fabs(t - 0.5);

    case ALICE_WAVE_SAWTOOTH:
        return 2.0 * t - 1.0 - alice_poly_blep(t, dt);

    case ALICE_WAVE_NOISE:
        // xorshift32
        v->noise_state ^= v->noise_state << 13;
        v->noise_state ^= v->noise_state >> 17;
        v->noise_state ^= v->noise_state << 5;
        return (double)v->noise_state / 2147483647.5 - 1.0;

    case ALICE_WAVE_SINE:
    default:
        return sin(2.0 * ALICE_PI * t);
    }
}

static void alice_synth_envelope_rates(const alice_synth_voice *v, double *up, double *down) {
    double step = 1.0 / alice_synth_rate(v);

    *up   = v->attack_seconds  > 0.0 ? step / v->attack_seconds  : 1.0;
    *down = v->release_seconds > 0.0 ? step / v->release_seconds : 1.0;
}

static float alice_synth_tick(alice_synth_voice *v, double env_up, double env_down) {
    double x = alice_synth_oscillator(v);

    v->phase += v->phase_step;
    if (v->phase >= 1.0)
        v->phase -= 1.0;

    if (v->gate) {
        v->envelope += env_up;
        if (v->envelope > 1.0)
            v->envelope = 1.0;
    } else {
        v->envelope -= env_down;
        if (v->envelope < 0.0)
            v->envelope = 0.0;
    }

    return (float)(x * v->amplitude * v->envelope);
}

float alice_synth_next(alice_synth_voice *v) {
    double up, down;

    if (!v)
        return 0.0f;
    alice_synth_envelope_rates(v, &up, &down);

    return alice_synth_tick(v, up, down);
}

void alice_synth_mix(alice_synth_voice *v, float *out, uint32_t frames, uint32_t channels, float pan) {
    uint32_t i;
    float left, right;
    double up, down;

    if (!v || !out || (channels != 1 && channels != 2))
        return;

    if (!v->gate && v->envelope <= 0.0) {
        v->phase = fmod(v->phase + (double)frames * v->phase_step, 1.0);
        return;
    }

    // equal power panning
    pan = (float)alice_clamp((double)pan, -1.0, 1.0);
    left  = (float)cos((pan + 1.0f) * (float)(ALICE_PI / 4.0));
    right = (float)sin((pan + 1.0f) * (float)(ALICE_PI / 4.0));
    alice_synth_envelope_rates(v, &up, &down);

    for (i = 0; i < frames; ++i) {
        float s = alice_synth_tick(v, up, down);
        if (channels == 1) {
            out[i] += s;
        } else {
            out[i * 2]     += s * left;
            out[i * 2 + 1] += s * right;
        }
    }
}

#endif /* ALICE_AUDIO_IMPLEMENTATION_ONCE */
#endif /* ALICE_AUDIO_IMPLEMENTATION */