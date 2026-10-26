#include "Params.h"

namespace percusyn
{
juce::String voiceParamId (int voice, int param)
{
    return juce::String (voiceKeys[(size_t) voice]) + "_" + voiceParamDefs[(size_t) param].key;
}

juce::String globalParamId (int param)
{
    return juce::String ("global_") + globalParamDefs[(size_t) param].key;
}

juce::NormalisableRange<float> rangeFor (const ParamDef& def)
{
    if (def.log)
    {
        const float lo = def.min, hi = def.max, step = def.step;
        return { lo, hi,
                 [lo, hi] (float, float, float n) { return lo * std::pow (hi / lo, n); },
                 [lo, hi] (float, float, float v) { return std::log (v / lo) / std::log (hi / lo); },
                 [lo, hi, step] (float, float, float v)
                 { return juce::jlimit (lo, hi, std::round ((v - lo) / step) * step + lo); } };
    }
    return { def.min, def.max, def.step };
}

Display formatValue (const ParamDef& def, float value)
{
    const juce::String unit (def.unit);
    if (unit == "Hz" && value >= 1000.0f)
        return { juce::String (value / 1000.0f, 1), "kHz" };
    if (unit == "ms" && value >= 1000.0f)
    {
        auto s = juce::String (value / 1000.0f, 2);
        if (s.endsWithChar ('0'))
            s = s.dropLastCharacters (1);
        return { s, "s" };
    }
    if (def.step < 1.0f)
    {
        auto s = juce::String (std::round (value * 10.0f) / 10.0f, 1);
        if (s.endsWith (".0"))
            s = s.dropLastCharacters (2);
        if (s == "-0")
            s = "0";
        return { s, unit };
    }
    const int rounded = (int) std::lround (value);
    return { juce::String (rounded), unit };
}

juce::String formatText (const ParamDef& def, float value)
{
    if (def.kind == Kind::Choice)
        return juce::StringArray::fromTokens (def.choices, "|", "")[(int) value];
    if (def.kind == Kind::Bool)
        return value > 0.5f ? "On" : "Off";
    const auto d = formatValue (def, value);
    return d.unit.isEmpty() ? d.number : d.number + " " + d.unit;
}

namespace
{
std::unique_ptr<juce::RangedAudioParameter> makeParam (const juce::String& id, const juce::String& name,
                                                       const ParamDef& def, float defaultValue)
{
    const juce::ParameterID pid { id, 1 };
    if (def.kind == Kind::Bool)
        return std::make_unique<juce::AudioParameterBool> (pid, name, defaultValue > 0.5f);
    if (def.kind == Kind::Choice)
        return std::make_unique<juce::AudioParameterChoice> (pid, name, juce::StringArray::fromTokens (def.choices, "|", ""),
                                                             (int) defaultValue);
    if (def.kind == Kind::Int)
        return std::make_unique<juce::AudioParameterInt> (pid, name, (int) def.min, (int) def.max, (int) defaultValue);

    const auto attributes = juce::AudioParameterFloatAttributes()
                                .withLabel (def.unit)
                                .withStringFromValueFunction ([def] (float v, int) { return formatText (def, v); });
    return std::make_unique<juce::AudioParameterFloat> (pid, name, rangeFor (def), defaultValue, attributes);
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto globals = std::make_unique<juce::AudioProcessorParameterGroup> ("global", "Global", "|");
    for (int p = 0; p < numGlobalParams; ++p)
    {
        const auto& def = globalParamDefs[(size_t) p];
        globals->addChild (makeParam (globalParamId (p), def.name, def, def.def));
    }
    layout.add (std::move (globals));

    for (int v = 0; v < numVoices; ++v)
    {
        const auto label = juce::String (voiceLabels[(size_t) v]);
        const auto title = label.substring (0, 1) + label.substring (1).toLowerCase();
        auto group = std::make_unique<juce::AudioProcessorParameterGroup> (voiceKeys[(size_t) v], title, "|");
        for (int p = 0; p < numVoiceParams; ++p)
        {
            const auto& def = voiceParamDefs[(size_t) p];
            group->addChild (makeParam (voiceParamId (v, p), title + " " + def.name, def, voiceDefault (v, p)));
        }
        layout.add (std::move (group));
    }
    return layout;
}

ParamRefs::ParamRefs (juce::AudioProcessorValueTreeState& state)
{
    for (int v = 0; v < numVoices; ++v)
        for (int p = 0; p < numVoiceParams; ++p)
            voiceRefs[(size_t) v][(size_t) p] = state.getRawParameterValue (voiceParamId (v, p));
    for (int p = 0; p < numGlobalParams; ++p)
        globalRefs[(size_t) p] = state.getRawParameterValue (globalParamId (p));
}

void ParamRefs::read (std::array<VoiceParams, numVoices>& voices, GlobalParams& globals) const noexcept
{
    for (size_t v = 0; v < voices.size(); ++v)
        for (size_t p = 0; p < (size_t) numVoiceParams; ++p)
            voices[v].v[p] = voiceRefs[v][p]->load();
    for (size_t p = 0; p < (size_t) numGlobalParams; ++p)
        globals.v[p] = globalRefs[p]->load();
}
} // namespace percusyn
