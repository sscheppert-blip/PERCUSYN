#pragma once

// Offline one-shot render of one voice from an independent parameter snapshot. Never touches
// the live engine, transport or automation (docs/05_EXPORT_WORKFLOW.md).

#include "dsp/Types.h"

#include <juce_audio_formats/juce_audio_formats.h>

namespace percusyn
{
struct OneShotSettings
{
    double sampleRate = 48000;
    int bitDepth = 24;   // 16, 24 or 32 (float)
};

// Stereo, voice chain incl. comb/resonator and voice level/pan, no global FX/master, no
// normalization, tail until -90 dBFS (max 10 s).
juce::AudioBuffer<float> renderOneShot (int voice, const VoiceParams& params, int velocity, double sampleRate);

bool writeWav (const juce::AudioBuffer<float>& audio, const OneShotSettings& settings, const juce::File& file,
               juce::String& error);

// Rendered files are kept here so a drag into the DAW never points at a deleted file.
juce::File renderFolder();
juce::File newRenderFile (int voice, const juce::String& presetName);
} // namespace percusyn
