#pragma once

struct AudioState; // defined in the .cpp so this header stays light

class PulseAudio {
public:
    PulseAudio() = default;
    ~PulseAudio();

    // Non-copyable: it owns an audio device
    PulseAudio(const PulseAudio&) = delete;
    PulseAudio& operator=(const PulseAudio&) = delete;

    bool init();
    void shutdown();
    void trigger(float strength); // strength in [0, 1]

private:
    AudioState* state = nullptr;
};