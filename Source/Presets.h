#pragma once

// Presets contain everything: all voices, comb/resonator, patterns, sequencer settings, FX and
// mixer. File format is the mockup's portable JSON ("percusyn-preset", schema 1 or 2).

#include "Pattern.h"
#include "Params.h"

namespace percusyn
{
struct PresetEntry
{
    juce::String name;
    bool factory = true;
    juce::File file;           // user presets
    juce::String json;         // factory presets
};

class Presets
{
public:
    Presets (juce::AudioProcessorValueTreeState& state, Pattern& pattern);

    void refresh();
    const juce::Array<PresetEntry>& entries() const noexcept { return list; }
    int indexOf (const juce::String& name) const;

    bool load (int index, juce::String& error);
    bool saveUser (const juce::String& name, juce::String& error);
    bool deleteUser (int index, juce::String& error);
    bool exportTo (const juce::File& file, const juce::String& name, juce::String& error) const;
    bool importFrom (const juce::File& file, juce::String& name, juce::String& error);
    void init();   // per-voice defaults, all steps cleared

    juce::var toVar (const juce::String& name) const;
    bool fromVar (const juce::var& data, juce::String& name, juce::String& error);

    static juce::File userFolder();

private:
    void setParam (const juce::String& id, float value);

    juce::AudioProcessorValueTreeState& state;
    Pattern& pattern;
    juce::Array<PresetEntry> list;
};
} // namespace percusyn
