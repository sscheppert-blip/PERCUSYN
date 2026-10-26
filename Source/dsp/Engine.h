#pragma once

// Nine instruments, mixer (mute/solo, per-voice outputs), FX sends and master.

#include "dsp/Fx.h"
#include "dsp/Voice.h"

namespace percusyn::dsp
{
class Engine
{
public:
    void prepare (double sampleRate);
    void reset();

    // Call once per block before render().
    void setParameters (const std::array<VoiceParams, numVoices>& voices, const GlobalParams& globals);
    void setPitchWheel (float bipolar) noexcept { wheel = bipolar; }

    void noteOn (int voice, int velocity, int gateSamples);
    void noteOff (int voice);

    // Renders [start, start + n). aux[v] null -> that voice goes to the main output.
    void render (float* mainL, float* mainR, const std::array<float*, numVoices>& auxL,
                 const std::array<float*, numVoices>& auxR, int start, int n) noexcept;

    const VoiceParams& voiceParams (int v) const noexcept { return params[(size_t) v]; }

private:
    double sampleRate = 48000;
    std::array<Instrument, numVoices> instruments;
    std::array<VoiceParams, numVoices> params {};
    std::array<bool, numVoices> audible {};
    GlobalParams global {};
    Delay delay;
    Reverb reverb;
    float wheel = 0;
    float delayReturn = 0, reverbReturn = 0, master = 1, smoothMaster = 1;
};
} // namespace percusyn::dsp
