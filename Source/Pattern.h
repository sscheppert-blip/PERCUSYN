#pragma once

// The 9 x 16 step pattern: plug-in state, not host parameters. Lock-free for the audio thread.

#include "dsp/Types.h"

#include <juce_core/juce_core.h>

#include <atomic>

namespace percusyn
{
enum class StepField { Active, Velocity, Probability, Condition };

class Pattern
{
public:
    Pattern();

    Step get (int voice, int step) const noexcept;
    float getField (int voice, int step, StepField field) const noexcept;
    void setField (int voice, int step, StepField field, float value) noexcept;
    void clearLane (int voice) noexcept;
    void clearAll() noexcept;

    void snapshot (PatternData& out) const noexcept;
    std::uint32_t version() const noexcept { return changes.load(); }

    // IDs as in the mockup: closedhat_step_01_active, ..._velocity, ..._probability, ..._condition.
    static juce::String fieldId (int voice, int step, StepField field);
    static juce::String octaveId (int voice, int step);   // legacy field, always written as 0

private:
    struct AtomicStep
    {
        std::atomic<bool> active { false };
        std::atomic<int> velocity { 100 };
        std::atomic<float> probability { 100.0f };
        std::atomic<int> condition { 0 };
    };
    std::array<std::array<AtomicStep, numSteps>, numVoices> steps;
    std::atomic<std::uint32_t> changes { 0 };
};
} // namespace percusyn
