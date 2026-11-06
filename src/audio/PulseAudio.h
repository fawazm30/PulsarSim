// PulseAudio.h
// Plays a short synthesized "tick" each time the pulsar completes a rotation.

#pragma once

struct AudioState; // defined in the .cpp so this header stays light

class PulseAudio {
public:
    PulseAudio() = default;
    ~PulseAudio(); // calls shutdown()

    // Non-copyable: it owns an audio device
    PulseAudio(const PulseAudio&) = delete;
    PulseAudio& operator=(const PulseAudio&) = delete;

    bool init(); // open and start the playback device; false on failure
    void shutdown(); // stop and release the device (safe to call twice)
    void trigger(float strength); // play one tick, strength in [0, 1]

private:
    AudioState* state = nullptr;
};