#include "Pattern.h"

namespace percusyn
{
Pattern::Pattern() = default;

Step Pattern::get (int voice, int step) const noexcept
{
    const auto& s = steps[(size_t) voice][(size_t) step];
    return { s.active.load(), s.velocity.load(), s.probability.load(), s.condition.load() };
}

float Pattern::getField (int voice, int step, StepField field) const noexcept
{
    const auto s = get (voice, step);
    switch (field)
    {
        case StepField::Active: return s.active ? 1.0f : 0.0f;
        case StepField::Velocity: return (float) s.velocity;
        case StepField::Probability: return s.probability;
        case StepField::Condition: return (float) s.condition;
    }
    return 0;
}

void Pattern::setField (int voice, int step, StepField field, float value) noexcept
{
    if (voice < 0 || voice >= numVoices || step < 0 || step >= numSteps || ! std::isfinite (value))
        return;
    auto& s = steps[(size_t) voice][(size_t) step];
    switch (field)
    {
        case StepField::Active: s.active = value > 0.5f; break;
        case StepField::Velocity: s.velocity = juce::jlimit (1, 127, (int) std::lround (value)); break;
        case StepField::Probability: s.probability = juce::jlimit (0.0f, 100.0f, std::round (value)); break;
        case StepField::Condition: s.condition = juce::jlimit (0, 3, (int) std::lround (value)); break;
    }
    ++changes;
}

void Pattern::clearLane (int voice) noexcept
{
    for (int i = 0; i < numSteps; ++i)
        setField (voice, i, StepField::Active, 0);
}

void Pattern::clearAll() noexcept
{
    for (int v = 0; v < numVoices; ++v)
        for (int i = 0; i < numSteps; ++i)
        {
            setField (v, i, StepField::Active, 0);
            setField (v, i, StepField::Velocity, 100);
            setField (v, i, StepField::Probability, 100);
            setField (v, i, StepField::Condition, 0);
        }
}

void Pattern::snapshot (PatternData& out) const noexcept
{
    for (int v = 0; v < numVoices; ++v)
        for (int i = 0; i < numSteps; ++i)
            out[(size_t) v][(size_t) i] = get (v, i);
}

juce::String Pattern::fieldId (int voice, int step, StepField field)
{
    static const char* names[] { "active", "velocity", "probability", "condition" };
    return juce::String (voiceKeys[(size_t) voice]) + "_step_" + juce::String (step + 1).paddedLeft ('0', 2) + "_"
           + names[(int) field];
}

juce::String Pattern::octaveId (int voice, int step)
{
    return juce::String (voiceKeys[(size_t) voice]) + "_step_" + juce::String (step + 1).paddedLeft ('0', 2) + "_octave";
}
} // namespace percusyn
