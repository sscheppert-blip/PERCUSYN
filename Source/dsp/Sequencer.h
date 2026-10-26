#pragma once

// Sample-accurate 16-step clock. Runs from the host position (follow-host mode) or from its own
// position at the internal tempo. Steps are sixteenth notes; Swing delays every second step.

#include "dsp/Types.h"

#include <array>

namespace percusyn::dsp
{
struct SeqEvent
{
    int offset = 0;   // sample offset in the block
    int voice = 0;
    int velocity = 100;
};

class Sequencer
{
public:
    static constexpr int maxEvents = 128;
    using Events = std::array<SeqEvent, maxEvents>;

    void prepare (double sampleRate);
    void startInternal() noexcept;
    void stopInternal() noexcept;
    bool isInternalRunning() const noexcept { return internalRunning; }

    // useHost: follow hostPpq/hostBpm; otherwise advance the internal position (if running).
    // Returns the number of events written.
    int process (int numSamples, bool useHost, double hostPpq, double hostBpm, double internalBpm,
                 const PatternData& pattern, float swingPercent, int length, Events& out) noexcept;

    int currentStep() const noexcept { return step; }   // -1 when stopped
    double sixteenthSamples (double bpm) const noexcept { return sampleRate * 15.0 / bpm; }
    void resetStep() noexcept { step = -1; }

private:
    double sampleRate = 48000;
    double internalPpq = 0;
    bool internalRunning = false;
    int step = -1;
    // Last step index played and where the previous block ended: a step is never played twice
    // when the host position of the next block lands a hair before the previous block's end.
    long long lastK = -(1LL << 62);
    double lastEnd = -1.0e9;
    std::uint32_t rng = 0x2545f491u;
    float random01() noexcept;
};
} // namespace percusyn::dsp
