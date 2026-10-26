#pragma once

// One percussion instrument: two overlapping hits (monophonic retrigger with a short fade),
// then the per-voice Comb and Resonator stages, level and pan. Semantics: docs/04_DSP_DESIGN.md.

#include "dsp/Types.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace percusyn::dsp
{
constexpr float pi = 3.14159265358979f;
constexpr float twoPi = 6.28318530717959f;

inline float dbToGain (float db) noexcept { return db <= -59.9f ? 0.0f : std::pow (10.0f, db / 20.0f); }

// Coefficient that reaches -60 dB after `ms` milliseconds.
inline float t60Coefficient (float ms, float sampleRate) noexcept
{
    return std::exp (-6.9077553f / std::max (1.0e-4f, ms * 0.001f * sampleRate));
}

struct Rng
{
    std::uint32_t state = 0x9e3779b9u;
    void seed (std::uint32_t s) noexcept { state = s != 0 ? s : 0x9e3779b9u; }
    std::uint32_t next() noexcept
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }
    float bipolar() noexcept { return (float) (next() >> 8) * (2.0f / 16777216.0f) - 1.0f; }
    float unit() noexcept { return (float) (next() >> 8) * (1.0f / 16777216.0f); }
};

// Topology-preserving state-variable filter (LP / HP / BP / BR).
struct Svf
{
    float g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0;
    float ic1 = 0, ic2 = 0;
    int type = 0;

    void set (float cutoff, float resonance01, int filterType, float sampleRate) noexcept
    {
        cutoff = std::clamp (cutoff, 10.0f, sampleRate * 0.45f);
        g = std::tan (pi * cutoff / sampleRate);
        k = 2.0f - 1.94f * std::clamp (resonance01, 0.0f, 1.0f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
        type = filterType;
    }
    void reset() noexcept { ic1 = ic2 = 0; }
    float process (float x) noexcept
    {
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        switch (type)
        {
            case 1: return x - k * v1 - v2;   // HP
            case 2: return k * v1;            // BP (unity peak)
            case 3: return x - k * v1;        // BR
            default: return v2;               // LP
        }
    }
};

struct OnePole
{
    float a = 0, z = 0;
    void setCutoff (float hz, float sampleRate) noexcept
    {
        a = std::exp (-twoPi * std::clamp (hz, 5.0f, sampleRate * 0.49f) / sampleRate);
    }
    float lowpass (float x) noexcept { return z = x + a * (z - x); }
    float highpass (float x) noexcept { return x - lowpass (x); }
};

struct DcBlocker
{
    float x1 = 0, y1 = 0;
    float process (float x) noexcept
    {
        const float y = x - x1 + 0.995f * y1;
        x1 = x;
        y1 = y;
        return y;
    }
    void reset() noexcept { x1 = y1 = 0; }
};

// Feedback comb with fractional (Hermite) delay, damping lowpass and a soft limit in the loop.
class Comb
{
public:
    void prepare (float sampleRate);
    void reset();
    void set (float tuneHz, float feedback01, float damping01, float mix01);
    float process (float x) noexcept;

private:
    std::vector<float> buffer;
    int mask = 0, write = 0;
    float sampleRate = 48000, delay = 40, feedback = 0, damping = 0, mix = 0, lp = 0;
    DcBlocker dc;
};

// Bank of six two-pole modal resonators excited by the voice. Ratio tables per model.
class Resonator
{
public:
    static constexpr int numModes = 6;
    void prepare (float sampleRate);
    void reset();
    void set (int model, float tuneHz, float decayMs, float damping01, float mix01);
    float process (float x) noexcept;

private:
    struct Mode { float b0 = 0, c1 = 0, c2 = 0, y1 = 0, y2 = 0; bool on = false; };
    std::array<Mode, numModes> modes {};
    float sampleRate = 48000, mix = 0;
    DcBlocker dc;
};

// A single hit: oscillator layer, noise layer, drive, filter, amp envelope.
class Hit
{
public:
    void prepare (float sampleRate, int voiceIndex);
    void start (const VoiceParams& p, float velocity01, int gateSamples, std::uint32_t seed);
    void release() noexcept;            // note-off: only matters with Sustain > 0
    void fadeOut (float ms) noexcept;   // choke / retrigger
    bool isActive() const noexcept { return active; }
    void updateBlock (const VoiceParams& p);
    float process (const VoiceParams& p, float wheel) noexcept;

private:
    float oscillator (float freq, float tone, int wave) noexcept;
    float polyBlep (float t, float dt) const noexcept;

    int voice = 0;
    float sampleRate = 48000, invSampleRate = 1.0f / 48000;
    bool active = false, gated = false, fading = false;
    int gateRemaining = -1;
    long long age = 0;

    float velocityGain = 1;
    float ampLevel = 0, ampCoeff = 0, ampAttackStep = 0, sustain = 0;
    int ampStage = 0;   // 0 attack, 1 decay/sustain, 2 release
    float fadeGain = 1, fadeCoeff = 1;
    float oscEnv = 1, oscCoeff = 0;
    float pitchEnv = 0, pitchAttackStep = 0, pitchCoeff = 0;
    bool pitchRising = true;

    float phase = 0, modPhase = 0, sawLp = 0;
    std::array<float, 6> clusterPhase {};
    float noiseGain = 0;
    OnePole noiseLp, noiseHp, sawFilter;
    float colour = 0.5f;   // noise colour 0..1
    Svf filter;
    Rng rng;
};

class Instrument
{
public:
    void prepare (float sampleRate, int voiceIndex);
    void reset();
    // gateSamples < 0: held until noteOff (MIDI); otherwise released after that many samples.
    void trigger (const VoiceParams& p, int velocity, int gateSamples);
    void noteOff();
    void choke();   // fast fade of everything including comb/resonator tails (hat choke)
    void beginBlock (const VoiceParams& p);
    // Adds one sample of post-fader stereo output and FX send.
    void process (const VoiceParams& p, float wheel, float& outL, float& outR, float& sendL, float& sendR,
                  bool audible) noexcept;
    bool isIdle() const noexcept { return idle; }

private:
    int voice = 0;
    float sampleRate = 48000;
    std::array<Hit, 2> hits;
    int current = 0;
    Comb comb;
    Resonator resonator;
    bool combWasOn = false, resWasOn = false;
    float chokeGain = 1, chokeTarget = 1, chokeStep = 0;
    float gainL = 0, gainR = 0, send = 0;
    float smoothL = 0, smoothR = 0, smoothSend = 0;
    int quietSamples = 0;
    bool idle = true;
    std::uint32_t hitCounter = 1;
};
} // namespace percusyn::dsp
