#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace percusyn;

juce::AudioProcessor::BusesProperties PercusynProcessor::makeBuses()
{
    auto buses = BusesProperties().withOutput ("Main", juce::AudioChannelSet::stereo(), true);
    for (int v = 0; v < numVoices; ++v)
    {
        const auto label = juce::String (voiceLabels[(size_t) v]);
        buses = buses.withOutput (label.substring (0, 1) + label.substring (1).toLowerCase(), juce::AudioChannelSet::stereo(), false);
    }
    return buses;
}

PercusynProcessor::PercusynProcessor()
    : AudioProcessor (makeBuses()), state (*this, nullptr, "PERCUSYN", createParameterLayout())
{
    juce::String error;
    presets.load (0, error);   // start with the first factory kit
}

bool PercusynProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    for (int i = 1; i < layouts.outputBuses.size(); ++i)
    {
        const auto& set = layouts.outputBuses.getReference (i);
        if (! set.isDisabled() && set != juce::AudioChannelSet::stereo())
            return false;
    }
    return layouts.inputBuses.isEmpty();
}

void PercusynProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    engine.prepare (sampleRate);
    sequencer.prepare (sampleRate);
    sequencer.stopInternal();
    seqRunning = false;
    seqStep = -1;
}

VoiceParams PercusynProcessor::voiceSnapshot (int voice) const
{
    VoiceParams p;
    for (int i = 0; i < numVoiceParams; ++i)
        p[i] = refs.voice (voice, i);
    return p;
}

void PercusynProcessor::audition (int voice, int velocity) noexcept
{
    auditionRequest.velocity = velocity;
    auditionRequest.voice = voice;
}

void PercusynProcessor::startPreview (std::shared_ptr<const juce::AudioBuffer<float>> audio, double audioSampleRate)
{
    const juce::SpinLock::ScopedLockType lock (previewLock);
    preview = std::move (audio);
    previewStep = audioSampleRate / currentSampleRate;
    previewPos = 0;
}

void PercusynProcessor::stopPreview()
{
    const juce::SpinLock::ScopedLockType lock (previewLock);
    preview.reset();
}

void PercusynProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    refs.read (voiceParams, globalParams);
    engine.setParameters (voiceParams, globalParams);
    pattern.snapshot (patternData);

    // --- transport --------------------------------------------------------------------------
    bool playing = false;
    double ppq = 0, bpm = globalParams[tempo];
    if (auto* ph = getPlayHead())
        if (const auto pos = ph->getPosition())
        {
            playing = pos->getIsPlaying();
            if (const auto b = pos->getBpm())
                bpm = *b;
            if (const auto p = pos->getPpqPosition())
                ppq = *p;
        }
    hostPlaying = playing;
    const bool followHost = globalParams[hostSync] > 0.5f;

    if (const int request = playRequest.exchange (0); request == 1 && ! playing)
        sequencer.startInternal();
    else if (request == 2)
        sequencer.stopInternal();
    if (playing)
        sequencer.stopInternal();   // host transport wins: internal clock off while the host plays

    const bool useHost = playing && followHost;
    const int numEvents = sequencer.process (numSamples, useHost, ppq, bpm, globalParams[tempo], patternData,
                                             globalParams[swing], (int) globalParams[length], seqEvents);
    const bool running = useHost || sequencer.isInternalRunning();
    seqRunning = running;
    if (! running)
        sequencer.resetStep();
    seqStep = sequencer.currentStep();
    const int gate = (int) sequencer.sixteenthSamples (useHost ? bpm : globalParams[tempo]);

    // --- output pointers --------------------------------------------------------------------
    auto mainBuffer = getBusBuffer (buffer, false, 0);
    float* mainL = mainBuffer.getWritePointer (0);
    float* mainR = mainBuffer.getWritePointer (1);
    std::array<float*, numVoices> auxL {}, auxR {};
    for (int v = 0; v < numVoices; ++v)
    {
        auto* bus = getBus (false, v + 1);
        if (bus != nullptr && bus->isEnabled() && bus->getNumberOfChannels() == 2)
        {
            auto aux = getBusBuffer (buffer, false, v + 1);
            auxL[(size_t) v] = aux.getWritePointer (0);
            auxR[(size_t) v] = aux.getWritePointer (1);
        }
    }

    // --- events, rendered sample-accurately between them -------------------------------------
    int pos = 0, seqIndex = 0;
    auto renderTo = [&] (int until)
    {
        if (until > pos)
        {
            engine.render (mainL, mainR, auxL, auxR, pos, until - pos);
            pos = until;
        }
    };
    auto trigger = [&] (int voice, int velocity, int gateSamples)
    {
        engine.noteOn (voice, velocity, gateSamples);
        ++triggers[(size_t) voice];
    };

    if (const int v = auditionRequest.voice.exchange (-1); v >= 0)
        trigger (v, auditionRequest.velocity.load(), gate);

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int offset = juce::jlimit (0, numSamples - 1, metadata.samplePosition);
        while (seqIndex < numEvents && seqEvents[(size_t) seqIndex].offset <= offset)
        {
            renderTo (seqEvents[(size_t) seqIndex].offset);
            trigger (seqEvents[(size_t) seqIndex].voice, seqEvents[(size_t) seqIndex].velocity, gate);
            ++seqIndex;
        }
        renderTo (offset);
        const int voice = msg.getNoteNumber() - firstMidiNote;
        if (msg.isNoteOn() && juce::isPositiveAndBelow (voice, numVoices))
            trigger (voice, msg.getVelocity(), -1);
        else if (msg.isNoteOff() && juce::isPositiveAndBelow (voice, numVoices))
            engine.noteOff (voice);
        else if (msg.isPitchWheel())
            engine.setPitchWheel ((float) (msg.getPitchWheelValue() - 8192) / 8192.0f);
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            for (int i = 0; i < numVoices; ++i)
                engine.noteOff (i);
    }
    for (; seqIndex < numEvents; ++seqIndex)
    {
        renderTo (seqEvents[(size_t) seqIndex].offset);
        trigger (seqEvents[(size_t) seqIndex].voice, seqEvents[(size_t) seqIndex].velocity, gate);
    }
    renderTo (numSamples);

    // --- one-shot preview on the main output -------------------------------------------------
    const juce::SpinLock::ScopedTryLockType lock (previewLock);
    if (lock.isLocked() && preview != nullptr)
    {
        const int length = preview->getNumSamples();
        const float* pl = preview->getReadPointer (0);
        const float* pr = preview->getReadPointer (1);
        for (int i = 0; i < numSamples && previewPos < length - 1; ++i)
        {
            const int idx = (int) previewPos;
            const float f = (float) (previewPos - idx);
            mainL[i] += pl[idx] + f * (pl[idx + 1] - pl[idx]);
            mainR[i] += pr[idx] + f * (pr[idx + 1] - pr[idx]);
            previewPos += previewStep;
        }
        if (previewPos >= length - 1)
            preview.reset();   // released on the audio thread only if the editor dropped its copy
    }
}

juce::AudioProcessorEditor* PercusynProcessor::createEditor()
{
    return new PercusynEditor (*this);
}

void PercusynProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto tree = state.copyState();
    juce::ValueTree steps ("PATTERN");
    for (int v = 0; v < numVoices; ++v)
        for (int i = 0; i < numSteps; ++i)
            for (auto f : { StepField::Active, StepField::Velocity, StepField::Probability, StepField::Condition })
                steps.setProperty (Pattern::fieldId (v, i, f), pattern.getField (v, i, f), nullptr);
    tree.removeChild (tree.getChildWithName ("PATTERN"), nullptr);
    tree.appendChild (steps, nullptr);

    tree.setProperty ("selectedVoice", selectedVoice, nullptr);
    tree.setProperty ("selectedStep", selectedStep, nullptr);
    tree.setProperty ("exportRate", exportSettings.sampleRate, nullptr);
    tree.setProperty ("exportDepth", exportSettings.bitDepth, nullptr);
    tree.setProperty ("presetName", presetName, nullptr);
    tree.setProperty ("presetIndex", presetIndex, nullptr);

    if (auto xml = tree.createXml())
        copyXmlToBinary (*xml, destData);
    tree.removeChild (tree.getChildWithName ("PATTERN"), nullptr);
}

void PercusynProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (state.state.getType()))
        return;
    auto tree = juce::ValueTree::fromXml (*xml);

    const auto steps = tree.getChildWithName ("PATTERN");
    for (int v = 0; v < numVoices; ++v)
        for (int i = 0; i < numSteps; ++i)
            for (auto f : { StepField::Active, StepField::Velocity, StepField::Probability, StepField::Condition })
            {
                const auto id = Pattern::fieldId (v, i, f);
                if (steps.hasProperty (id))
                    pattern.setField (v, i, f, (float) steps.getProperty (id));
            }

    selectedVoice = juce::jlimit (0, numVoices - 1, (int) tree.getProperty ("selectedVoice", 0));
    selectedStep = juce::jlimit (0, numSteps - 1, (int) tree.getProperty ("selectedStep", 0));
    exportSettings.sampleRate = (double) tree.getProperty ("exportRate", 48000.0);
    exportSettings.bitDepth = (int) tree.getProperty ("exportDepth", 24);
    presetName = tree.getProperty ("presetName", "Acid Hats").toString();
    presetIndex = (int) tree.getProperty ("presetIndex", 0);

    tree.removeChild (steps, nullptr);
    state.replaceState (tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PercusynProcessor();
}
