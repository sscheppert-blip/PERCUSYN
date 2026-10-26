#include "OneShot.h"

#include "dsp/Voice.h"

namespace percusyn
{
juce::AudioBuffer<float> renderOneShot (int voice, const VoiceParams& params, int velocity, double sampleRate)
{
    dsp::Instrument instrument;
    instrument.prepare ((float) sampleRate, voice);
    instrument.beginBlock (params);
    instrument.trigger (params, juce::jlimit (1, 127, velocity), (int) (sampleRate * 0.125));   // one 16th at 120 BPM

    const int maxSamples = (int) (sampleRate * 10.0);
    juce::AudioBuffer<float> out (2, maxSamples);
    out.clear();
    auto* l = out.getWritePointer (0);
    auto* r = out.getWritePointer (1);

    constexpr float threshold = 3.1623e-5f;   // -90 dBFS
    int lastLoud = 0, n = 0;
    for (; n < maxSamples; ++n)
    {
        if (n % 64 == 0)
            instrument.beginBlock (params);
        float sl = 0, sr = 0;
        instrument.process (params, 0.0f, l[n], r[n], sl, sr, true);
        if (std::abs (l[n]) > threshold || std::abs (r[n]) > threshold)
            lastLoud = n;
        if (instrument.isIdle() && n - lastLoud > (int) (sampleRate * 0.05))
            break;
    }

    const int length = juce::jmax (1, lastLoud + 1);
    // Short fade over the last 2 ms so the file ends at zero.
    const int fade = juce::jmin (length, (int) (sampleRate * 0.002));
    for (int ch = 0; ch < 2; ++ch)
        out.applyGainRamp (ch, length - fade, fade, 1.0f, 0.0f);
    out.setSize (2, length, true, false, false);
    return out;
}

bool writeWav (const juce::AudioBuffer<float>& audio, const OneShotSettings& settings, const juce::File& file,
               juce::String& error)
{
    file.deleteFile();
    std::unique_ptr<juce::OutputStream> stream (file.createOutputStream());
    if (stream == nullptr)
    {
        error = "Could not create " + file.getFullPathName();
        return false;
    }
    juce::WavAudioFormat wav;
    const auto options = juce::AudioFormatWriterOptions()
                             .withSampleRate (settings.sampleRate)
                             .withNumChannels (audio.getNumChannels())
                             .withBitsPerSample (settings.bitDepth)
                             .withSampleFormat (settings.bitDepth == 32 ? juce::AudioFormatWriterOptions::SampleFormat::floatingPoint
                                                                        : juce::AudioFormatWriterOptions::SampleFormat::integral);
    auto writer = wav.createWriterFor (stream, options);
    if (writer == nullptr)
    {
        error = "This sample rate / bit depth cannot be written as WAV.";
        return false;
    }
    if (! writer->writeFromAudioSampleBuffer (audio, 0, audio.getNumSamples()))
    {
        error = "Writing the WAV file failed.";
        return false;
    }
    return true;
}

juce::File renderFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("PERCUSYN").getChildFile ("Renders");
}

juce::File newRenderFile (int voice, const juce::String& presetName)
{
    const auto label = juce::String (voiceLabels[(size_t) voice]);
    const auto title = label.substring (0, 1) + label.substring (1).toLowerCase();
    const auto stamp = juce::Time::getCurrentTime().formatted ("%Y-%m-%d %H%M%S");
    const auto name = juce::File::createLegalFileName ("PERCUSYN " + title + " - " + presetName + " " + stamp);
    return renderFolder().getChildFile (name + ".wav").getNonexistentSibling (false);
}
} // namespace percusyn
