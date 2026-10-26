#pragma once

// Host parameters: every sound, comb/resonator, FX, mixer and sequencer-global control.
// Per-step pattern data is plug-in state only (Pattern.h). IDs follow mockup/src/model.js.

#include "dsp/Types.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace percusyn
{
juce::String voiceParamId (int voice, int param);
juce::String globalParamId (int param);

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

// Display text as on the hardware readouts: "6.5 kHz", "0.5 ms", "1.8 s", "-9 dB".
struct Display
{
    juce::String number, unit;
};
Display formatValue (const ParamDef& def, float value);
juce::String formatText (const ParamDef& def, float value);

// LED ring colour rule (control-colors.js): durations in ms/s are blue, all else orange.
inline bool isDurationParam (const ParamDef& def) { return juce::String (def.unit) == "ms"; }

juce::NormalisableRange<float> rangeFor (const ParamDef& def);

// Fast audio-thread access to every parameter value.
class ParamRefs
{
public:
    explicit ParamRefs (juce::AudioProcessorValueTreeState& state);
    void read (std::array<VoiceParams, numVoices>& voices, GlobalParams& globals) const noexcept;
    float voice (int v, int p) const noexcept { return voiceRefs[(size_t) v][(size_t) p]->load(); }
    float global (int p) const noexcept { return globalRefs[(size_t) p]->load(); }

private:
    std::array<std::array<std::atomic<float>*, numVoiceParams>, numVoices> voiceRefs {};
    std::array<std::atomic<float>*, numGlobalParams> globalRefs {};
};
} // namespace percusyn
