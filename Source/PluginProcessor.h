#pragma once

#include "OneShot.h"
#include "Params.h"
#include "Pattern.h"
#include "Presets.h"
#include "dsp/Engine.h"
#include "dsp/Sequencer.h"

#include <juce_audio_utils/juce_audio_utils.h>

class PercusynProcessor : public juce::AudioProcessor
{
public:
    PercusynProcessor();
    ~PercusynProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- editor API (message thread) ---
    juce::AudioProcessorValueTreeState state;
    percusyn::Pattern pattern;
    percusyn::Presets presets { state, pattern };

    void requestPlay (bool shouldPlay) noexcept { playRequest = shouldPlay ? 1 : 2; }
    bool isSequencerRunning() const noexcept { return seqRunning.load(); }
    bool isHostPlaying() const noexcept { return hostPlaying.load(); }
    int currentStep() const noexcept { return seqStep.load(); }
    // Bumped each time a voice is triggered; the editor flashes the instrument buttons.
    std::uint32_t triggerCount (int voice) const noexcept { return triggers[(size_t) voice].load(); }

    void audition (int voice, int velocity) noexcept;   // click an instrument with Shift
    void startPreview (std::shared_ptr<const juce::AudioBuffer<float>> audio, double audioSampleRate);
    void stopPreview();

    percusyn::VoiceParams voiceSnapshot (int voice) const;

    // UI/session state stored with the project.
    int selectedVoice = 0, selectedStep = 0;
    percusyn::OneShotSettings exportSettings;
    juce::String presetName { "Acid Hats" };
    int presetIndex = 0;

private:
    static BusesProperties makeBuses();

    percusyn::ParamRefs refs { state };
    percusyn::dsp::Engine engine;
    percusyn::dsp::Sequencer sequencer;

    std::array<percusyn::VoiceParams, percusyn::numVoices> voiceParams {};
    percusyn::GlobalParams globalParams {};
    percusyn::PatternData patternData {};
    percusyn::dsp::Sequencer::Events seqEvents {};

    std::atomic<int> playRequest { 0 };   // 1 start, 2 stop
    std::atomic<bool> seqRunning { false }, hostPlaying { false };
    std::atomic<int> seqStep { -1 };
    std::array<std::atomic<std::uint32_t>, percusyn::numVoices> triggers {};

    struct Audition { std::atomic<int> voice { -1 }, velocity { 100 }; } auditionRequest;

    juce::SpinLock previewLock;
    std::shared_ptr<const juce::AudioBuffer<float>> preview;
    double previewStep = 1, previewPos = 0;
    double currentSampleRate = 48000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PercusynProcessor)
};
