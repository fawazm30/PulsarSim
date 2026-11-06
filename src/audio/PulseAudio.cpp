// PulseAudio.cpp
// Pulse tick audio using miniaudio. The main thread requests a tick; the audio
// thread synthesizes a decaying 140 Hz tone with a burst of noise for the click.

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "PulseAudio.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <iostream>

struct AudioState {
    ma_device device;
    std::atomic<float> pendingStrength{0.0f}; // written by main thread, read by audio thread

    // Only touched by the audio thread
    float envelope = 0.0f;
    float phase = 0.0f;
    uint32_t noiseSeed = 12345;
};

// Audio thread callback: fills the output buffer with the current tick's samples
static void dataCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount) {
    AudioState* s = static_cast<AudioState*>(device->pUserData);
    float* out = static_cast<float*>(output);

    // Grab any pending trigger and reset it
    float strength = s->pendingStrength.exchange(0.0f);
    if (strength > 0.0f) {
        s->envelope = strength;
        s->phase = 0.0f;
    }

    const float twoPi = 6.2831853f;
    const float phaseStep = twoPi * 140.0f / (float)device->sampleRate; // 140 Hz thump

    for (ma_uint32 i = 0; i < frameCount; i++) {
        float sample = 0.0f;
        if (s->envelope > 0.0001f) {
            // Cheap noise for a sharp "click" at the start of each tick
            s->noiseSeed = s->noiseSeed * 1664525u + 1013904223u;
            float noise = ((s->noiseSeed >> 9) / 4194304.0f) - 1.0f;

            sample = s->envelope * (0.85f * std::sin(s->phase) + 0.15f * noise);
            s->phase += phaseStep;
            s->envelope *= 0.9992f; // exponential decay, roughly 25 ms
        }
        out[i] = sample * 0.5f; // overall volume
    }
}

// Open a mono 48 kHz playback device and start it
bool PulseAudio::init() {
    state = new AudioState();

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 1;
    config.sampleRate = 48000;
    config.dataCallback = dataCallback;
    config.pUserData = state;

    if (ma_device_init(nullptr, &config, &state->device) != MA_SUCCESS) {
        std::cerr << "Audio: failed to initialize device\n";
        delete state;
        state = nullptr;
        return false;
    }
    if (ma_device_start(&state->device) != MA_SUCCESS) {
        std::cerr << "Audio: failed to start device\n";
        ma_device_uninit(&state->device);
        delete state;
        state = nullptr;
        return false;
    }
    return true;
}

// Queue a tick for the audio thread to pick up on its next callback
void PulseAudio::trigger(float strength) {
    if (state) state->pendingStrength.store(strength);
}

// Stop the device and free the audio state
void PulseAudio::shutdown() {
    if (state) {
        ma_device_uninit(&state->device);
        delete state;
        state = nullptr;
    }
}

// Make sure the device is released when the object goes away
PulseAudio::~PulseAudio() {
    shutdown();
}