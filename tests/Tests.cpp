// Offline checks for the engine, sequencer, state and presets.
//   PercusynTests                         run all checks
//   PercusynTests --render out.wav <voice> [preset]   one-shot of a voice (0-8)
//   PercusynTests --kit out.wav [preset] [bars]       the internal sequencer playing a preset
//   PercusynTests --snapshot out.png <tab 0-3> [voice] [scale]
//   PercusynTests --load <path to PERCUSYN.vst3>      load the built plug-in like a host

#include "OneShot.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "dsp/Engine.h"
#include "dsp/Sequencer.h"

#include <iostream>

using namespace percusyn;

namespace
{
int failures = 0;

void check (bool ok, const juce::String& what)
{
    std::cout << (ok ? "  ok   " : "  FAIL ") << what << "\n";
    if (! ok)
        ++failures;
}

VoiceParams defaults (int voice)
{
    VoiceParams p;
    for (int i = 0; i < numVoiceParams; ++i)
        p[i] = voiceDefault (voice, i);
    return p;
}

bool finite (const juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (! std::isfinite (b.getSample (c, i)))
                return false;
    return true;
}

juce::AudioBuffer<float> runProcessor (PercusynProcessor& proc, double sr, int blocks, int block,
                                       std::function<void (int, juce::MidiBuffer&)> midiFor = {})
{
    juce::AudioBuffer<float> out (2, blocks * block), buffer (proc.getTotalNumOutputChannels(), block);
    juce::MidiBuffer midi;
    for (int b = 0; b < blocks; ++b)
    {
        midi.clear();
        if (midiFor)
            midiFor (b, midi);
        proc.processBlock (buffer, midi);
        out.copyFrom (0, b * block, buffer, 0, 0, block);
        out.copyFrom (1, b * block, buffer, 1, 0, block);
    }
    juce::ignoreUnused (sr);
    return out;
}

void testVoices()
{
    std::cout << "Voices\n";
    for (int v = 0; v < numVoices; ++v)
    {
        const auto audio = renderOneShot (v, defaults (v), 115, 48000);
        const float peak = audio.getMagnitude (0, audio.getNumSamples());
        check (finite (audio) && peak > 0.01f && peak < 2.0f && audio.getNumSamples() > 100,
               juce::String (voiceLabels[(size_t) v]) + " default: peak " + juce::String (peak, 3) + ", "
                   + juce::String (audio.getNumSamples() / 48.0, 1) + " ms");
    }

    // Extreme settings must stay bounded.
    for (int v = 0; v < numVoices; ++v)
    {
        auto p = defaults (v);
        p[driveAmount] = 100;
        p[filterResonance] = 100;
        p[noiseLevel] = 12;
        p[combEnabled] = 1;
        p[combFeedback] = 95;
        p[combMix] = 100;
        p[combDamping] = 0;
        p[resEnabled] = 1;
        p[resDecay] = 8000;
        p[resMix] = 100;
        p[resDamping] = 0;
        p[level] = 12;
        p[oscWave] = (float) (v % 5);
        const auto audio = renderOneShot (v, p, 127, 96000);
        const float peak = audio.getMagnitude (0, audio.getNumSamples());
        check (finite (audio) && peak < 8.0f && audio.getNumSamples() <= 96000 * 10,
               juce::String (voiceLabels[(size_t) v]) + " extreme: peak " + juce::String (peak, 2) + ", "
                   + juce::String (audio.getNumSamples() / 96000.0, 2) + " s");
    }

    // Velocity changes the level when the sensitivity is up.
    auto p = defaults (2);
    p[velocitySens] = 100;
    const float loud = renderOneShot (2, p, 127, 48000).getMagnitude (0, 2000);
    const float soft = renderOneShot (2, p, 30, 48000).getMagnitude (0, 2000);
    check (soft < loud * 0.5f, "velocity sensitivity: 30 -> " + juce::String (soft, 3) + ", 127 -> " + juce::String (loud, 3));

    // Sustain holds the gate; at 0% the gate is ignored.
    p = defaults (3);
    p[ampSustain] = 60;
    p[ampDecay] = 40;
    p[oscDecay] = 10000;   // keep the tonal layer alive; Osc Decay shapes it independently
    dsp::Instrument inst;
    inst.prepare (48000, 3);
    inst.beginBlock (p);
    inst.trigger (p, 100, -1);
    float l = 0, r = 0, sl = 0, sr = 0;
    for (int i = 0; i < 48000; ++i)
        inst.process (p, 0, l, r, sl, sr, true);
    float level = 0;
    for (int i = 0; i < 480; ++i)
    {
        float a = 0, b = 0;
        inst.process (p, 0, a, b, sl, sr, true);
        level = std::max (level, std::abs (a));
    }
    check (level > 0.01f, "sustain 60% holds while the note is held (" + juce::String (level, 3) + ")");
    inst.noteOff();
    for (int i = 0; i < 48000; ++i)
    {
        float a = 0, b = 0;
        inst.process (p, 0, a, b, sl, sr, true);
    }
    check (inst.isIdle(), "note-off releases the held voice");
}

void testSequencer()
{
    std::cout << "Sequencer\n";
    dsp::Sequencer seq;
    seq.prepare (48000);
    PatternData pattern {};
    for (int s = 0; s < numSteps; ++s)
        pattern[0][(size_t) s].active = true;
    pattern[1][3].active = true;
    pattern[1][3].condition = 1;   // 1:2 -> every second cycle
    dsp::Sequencer::Events events {};

    seq.startInternal();
    std::vector<std::pair<long long, int>> hits;
    long long pos = 0;
    const int block = 256;
    for (int b = 0; b < 48000 * 4 / block; ++b)   // 4 s at 120 BPM = 2 bars
    {
        const int n = seq.process (block, false, 0, 120, 120, pattern, 0, 16, events);
        for (int i = 0; i < n; ++i)
            hits.push_back ({ pos + events[(size_t) i].offset, events[(size_t) i].voice });
        pos += block;
    }
    int ch = 0, oh = 0;
    for (auto& h : hits)
        (h.second == 0 ? ch : oh)++;
    check (ch == 32, "internal clock: 32 closed-hat steps in two bars (" + juce::String (ch) + ")");
    check (oh == 1, "condition 1:2 fires once in two cycles (" + juce::String (oh) + ")");
    check (hits.size() > 2 && std::abs ((hits[1].first - hits[0].first) - 6000) <= 1, "straight sixteenths are 6000 samples apart");

    // Swing 50%: the second step comes half a sixteenth late.
    seq.stopInternal();
    seq.startInternal();
    hits.clear();
    pos = 0;
    for (int b = 0; b < 100; ++b)
    {
        const int n = seq.process (block, false, 0, 120, 120, pattern, 50, 16, events);
        for (int i = 0; i < n; ++i)
            if (events[(size_t) i].voice == 0)
                hits.push_back ({ pos + events[(size_t) i].offset, 0 });
        pos += block;
    }
    check (hits.size() >= 3 && std::abs ((hits[1].first - hits[0].first) - 9000) <= 1
               && std::abs ((hits[2].first - hits[1].first) - 3000) <= 1,
           "swing 50% delays every second step by half a step");

    // Host clock: steps come from the ppq position, block size does not matter.
    for (int blockSize : { 32, 512, 1999 })
    {
        int count = 0;
        double ppq = 0;
        for (long long done = 0; done < 48000 * 2; done += blockSize)
        {
            const int n = seq.process (blockSize, true, ppq, 120, 120, pattern, 0, 16, events);
            for (int i = 0; i < n; ++i)
                count += done + events[(size_t) i].offset < 48000 * 2 ? 1 : 0;
            ppq += blockSize * 120.0 / 60.0 / 48000.0;
        }
        check (count == 16 + 1, "host clock, block " + juce::String (blockSize) + ": 16 steps + 1 conditional hit per bar ("
                                    + juce::String (count) + ")");
    }
}

void testProcessor()
{
    std::cout << "Processor\n";
    PercusynProcessor proc;
    proc.setRateAndBufferSizeDetails (48000, 512);
    proc.prepareToPlay (48000, 512);

    // MIDI C1 plays the closed hat on the main output.
    auto audio = runProcessor (proc, 48000, 40, 512, [] (int b, juce::MidiBuffer& m)
    {
        if (b == 1)
            m.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 110), 100);
    });
    check (finite (audio) && audio.getMagnitude (0, audio.getNumSamples()) > 0.01f, "MIDI note 36 plays Closed Hat");

    // Note 45 (above Click) is ignored.
    proc.prepareToPlay (48000, 512);
    audio = runProcessor (proc, 48000, 40, 512, [] (int b, juce::MidiBuffer& m)
    {
        if (b == 1)
            m.addEvent (juce::MidiMessage::noteOn (1, 45, (juce::uint8) 110), 0);
    });
    check (audio.getMagnitude (0, audio.getNumSamples()) < 1.0e-6f, "notes outside C1..G#1 are ignored");

    // Internal sequencer plays the preset pattern.
    proc.prepareToPlay (48000, 512);
    proc.requestPlay (true);
    audio = runProcessor (proc, 48000, 200, 512);
    check (proc.isSequencerRunning() && audio.getMagnitude (0, audio.getNumSamples()) > 0.01f, "internal sequencer runs and plays");
    proc.requestPlay (false);
    runProcessor (proc, 48000, 2, 512);
    check (! proc.isSequencerRunning(), "STOP stops the internal sequencer");

    // Multi-out: an enabled voice bus takes that voice off the main output.
    {
        auto layout = proc.getBusesLayout();
        layout.outputBuses.getReference (3) = juce::AudioChannelSet::stereo();   // Snare
        check (proc.setBusesLayout (layout), "snare output bus can be enabled");
        proc.prepareToPlay (48000, 512);
        juce::AudioBuffer<float> buffer (proc.getTotalNumOutputChannels(), 512);
        juce::MidiBuffer midi;
        float mainPeak = 0, auxPeak = 0;
        for (int b = 0; b < 30; ++b)
        {
            midi.clear();
            if (b == 0)
                midi.addEvent (juce::MidiMessage::noteOn (1, 38, (juce::uint8) 120), 0);
            proc.processBlock (buffer, midi);
            mainPeak = std::max (mainPeak, buffer.getMagnitude (0, 0, 512));
            auto aux = proc.getBusBuffer (buffer, false, 3);
            auxPeak = std::max (auxPeak, aux.getMagnitude (0, 0, 512));
        }
        check (auxPeak > 0.01f, "snare plays on its own output (" + juce::String (auxPeak, 3) + ")");
        check (mainPeak < auxPeak * 0.5f, "snare dry signal leaves the main output (main " + juce::String (mainPeak, 3) + ")");
        layout.outputBuses.getReference (3) = juce::AudioChannelSet::disabled();
        proc.setBusesLayout (layout);
    }

    // State round trip: parameters and pattern.
    proc.pattern.setField (4, 5, StepField::Velocity, 77);
    proc.pattern.setField (4, 5, StepField::Active, 1);
    proc.state.getParameter ("tom_comb_feedback")->setValueNotifyingHost (0.25f);
    const float expected = proc.state.getParameter ("tom_comb_feedback")->getValue();
    juce::MemoryBlock block;
    proc.getStateInformation (block);
    PercusynProcessor other;
    other.setStateInformation (block.getData(), (int) block.getSize());
    check (other.pattern.get (4, 5).velocity == 77 && other.pattern.get (4, 5).active, "pattern survives the host state");
    check (std::abs (other.state.getParameter ("tom_comb_feedback")->getValue() - expected) < 1.0e-4f,
           "parameters survive the host state");

    // Presets: all factory presets load; JSON round trip.
    for (int i = 0; i < proc.presets.entries().size(); ++i)
    {
        juce::String error;
        check (proc.presets.load (i, error), "factory preset " + proc.presets.entries()[i].name + (error.isEmpty() ? "" : ": " + error));
    }
    juce::String name, error;
    const auto json = proc.presets.toVar ("Round Trip");
    proc.presets.init();
    check (! proc.pattern.get (0, 0).active, "INIT clears the steps");
    check (proc.presets.fromVar (json, name, error) && name == "Round Trip", "preset JSON round trip");
}

void testMockupPreset()
{
    std::cout << "Mockup preset compatibility\n";
    PercusynProcessor proc;
    const auto file = juce::File::getCurrentWorkingDirectory().getChildFile ("examples/presets/06-metal-controls-demo.json");
    if (! file.existsAsFile())
    {
        std::cout << "  skip (run from the project folder)\n";
        return;
    }
    juce::String name, error;
    check (proc.presets.importFrom (file, name, error), "imports " + file.getFileName() + " " + error);
    const auto data = juce::JSON::parse (file);
    const auto* params = data.getProperty ("parameters", {}).getDynamicObject();
    int mismatches = 0;
    for (const auto& nv : params->getProperties())
    {
        auto* param = proc.state.getParameter (nv.name.toString());
        if (param == nullptr)
            continue;
        if (std::abs (param->convertFrom0to1 (param->getValue()) - (float) (double) nv.value) > 0.051f)
            ++mismatches;
    }
    check (mismatches == 0, "every mockup parameter value arrives unchanged (" + juce::String (mismatches) + " mismatches)");
}

void testStepPainting()
{
    std::cout << "Step painting (simulated mouse)" << std::endl;
    PercusynProcessor proc;
    proc.pattern.clearAll();
    proc.selectedVoice = 2;
    ui::LanePanel lane (proc);
    lane.setBounds (0, 0, 744, 260);

    auto source = juce::Desktop::getInstance().getMainMouseSource();
    // STEP row cells: x0 = 74, width (744 - 86) / 16, row centre y = 96.
    const float w = (744.0f - 86.0f) / 16.0f;
    auto at = [&] (int step) { return juce::Point<float> (74.0f + w * ((float) step + 0.5f), 96.0f); };
    auto event = [&] (juce::Point<float> pos, juce::ModifierKeys mods, juce::Point<float> down)
    {
        return juce::MouseEvent (source, pos, mods, 0, 0, 0, 0, 0, &lane, &lane, juce::Time::getCurrentTime(), down,
                                 juce::Time::getCurrentTime(), 1, pos != down);
    };
    const juce::ModifierKeys left (juce::ModifierKeys::leftButtonModifier), right (juce::ModifierKeys::rightButtonModifier);

    lane.mouseDown (event (at (2), left, at (2)));
    for (int s = 3; s <= 9; ++s)
        lane.mouseDrag (event (at (s), left, at (2)));
    lane.mouseUp (event (at (9), {}, at (2)));
    int on = 0;
    for (int s = 0; s < numSteps; ++s)
        on += proc.pattern.get (2, s).active ? 1 : 0;
    check (on == 8 && proc.pattern.get (2, 2).active && proc.pattern.get (2, 9).active, "left drag draws steps 3-10 in one stroke ("
                                                                                          + juce::String (on) + " on)");

    lane.mouseDown (event (at (8), right, at (8)));
    for (int s = 7; s >= 4; --s)
        lane.mouseDrag (event (at (s), right, at (8)));
    lane.mouseUp (event (at (4), {}, at (8)));
    on = 0;
    for (int s = 0; s < numSteps; ++s)
        on += proc.pattern.get (2, s).active ? 1 : 0;
    check (on == 3 && ! proc.pattern.get (2, 4).active && proc.pattern.get (2, 9).active,
           "right drag erases steps 5-9 in one stroke (" + juce::String (on) + " left)");
    check (proc.pattern.get (1, 4).active == false && proc.selectedStep == 2, "other lanes untouched, clicked step selected");
}

int renderVoice (const juce::String& path, int voice, const juce::String& preset)
{
    PercusynProcessor proc;
    juce::String error;
    proc.presets.load (juce::jmax (0, proc.presets.indexOf (preset)), error);
    const auto audio = renderOneShot (voice, proc.voiceSnapshot (voice), 115, 48000);
    const bool ok = writeWav (audio, { 48000, 24 }, juce::File::getCurrentWorkingDirectory().getChildFile (path), error);
    std::cout << (ok ? "wrote " + path : error) << "\n";
    return ok ? 0 : 1;
}

int renderKit (const juce::String& path, const juce::String& preset, int bars)
{
    PercusynProcessor proc;
    juce::String error;
    proc.presets.load (juce::jmax (0, proc.presets.indexOf (preset)), error);
    proc.setRateAndBufferSizeDetails (48000, 512);
    proc.prepareToPlay (48000, 512);
    proc.requestPlay (true);
    const double bpm = proc.state.getRawParameterValue ("global_tempo")->load();
    const int blocks = (int) (bars * 4 * 60.0 / bpm * 48000 / 512) + 90;
    const auto audio = runProcessor (proc, 48000, blocks, 512);
    const bool ok = writeWav (audio, { 48000, 24 }, juce::File::getCurrentWorkingDirectory().getChildFile (path), error);
    std::cout << (ok ? "wrote " + path + ", peak " + juce::String (audio.getMagnitude (0, audio.getNumSamples()), 3) : error) << "\n";
    return ok ? 0 : 1;
}

int loadPlugin (const juce::String& path)
{
    juce::AudioPluginFormatManager formats;
    formats.addFormat (std::make_unique<juce::VST3PluginFormat>());
    juce::OwnedArray<juce::PluginDescription> found;
    juce::KnownPluginList list;
    juce::VST3PluginFormat vst3;
    list.scanAndAddFile (path, true, found, vst3);
    check (found.size() == 1, "scan finds one plug-in in " + path);
    if (found.isEmpty())
        return 1;
    const auto& desc = *found[0];
    check (desc.isInstrument && desc.name == "PERCUSYN", "described as instrument '" + desc.name + "' by " + desc.manufacturerName);

    juce::String error;
    auto instance = formats.createPluginInstance (desc, 48000, 512, error);
    check (instance != nullptr, "instantiates " + error);
    if (instance == nullptr)
        return 1;
    check (instance->getBusCount (false) == 1 + numVoices, "output buses: " + juce::String (instance->getBusCount (false)));
    check (instance->getParameters().size() >= numGlobalParams + numVoices * numVoiceParams,
           "host parameters: " + juce::String (instance->getParameters().size()));
    instance->enableAllBuses();
    instance->prepareToPlay (48000, 512);

    juce::AudioBuffer<float> buffer (instance->getTotalNumOutputChannels(), 512);
    juce::MidiBuffer midi;
    float peak = 0;
    bool ok = true;
    for (int b = 0; b < 200; ++b)
    {
        midi.clear();
        if (b % 20 == 0)
            for (int n = 0; n < numVoices; ++n)
                midi.addEvent (juce::MidiMessage::noteOn (1, firstMidiNote + n, (juce::uint8) 100), n * 7);
        buffer.clear();
        instance->processBlock (buffer, midi);
        for (int c = 0; c < buffer.getNumChannels(); ++c)
            for (int i = 0; i < 512; ++i)
                ok = ok && std::isfinite (buffer.getSample (c, i));
        peak = std::max (peak, buffer.getMagnitude (0, 512));
    }
    check (ok && peak > 0.01f, "MIDI C1..G#1 through the VST3 produce audio on " + juce::String (buffer.getNumChannels())
                                   + " channels, peak " + juce::String (peak, 3));

    juce::MemoryBlock state;
    instance->getStateInformation (state);
    instance->setStateInformation (state.getData(), (int) state.getSize());
    check (state.getSize() > 1000, "state save/restore through the VST3 (" + juce::String ((int) state.getSize()) + " bytes)");

    std::unique_ptr<juce::AudioProcessorEditor> editor (instance->createEditorAndMakeActive());
    check (editor != nullptr && editor->getWidth() > 0, "editor opens at " + juce::String (editor ? editor->getWidth() : 0)
                                                           + " x " + juce::String (editor ? editor->getHeight() : 0));
    editor.reset();
    instance->releaseResources();
    std::cout << (failures == 0 ? "VST3 load test passed" : "VST3 load test FAILED") << std::endl;
    return failures == 0 ? 0 : 1;
}

int snapshot (const juce::String& path, int tab, int voice, float scale)
{
    PercusynProcessor proc;
    proc.selectedVoice = voice;
    std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());
    editor->setSize ((int) (ui::rackWidth * scale), (int) (ui::rackHeight * scale));
    if (auto* rack = dynamic_cast<Rack*> (editor->getChildComponent (0)))
        rack->showTab (tab);
    const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    juce::File out = juce::File::getCurrentWorkingDirectory().getChildFile (path);
    out.deleteFile();
    juce::FileOutputStream stream (out);
    juce::PNGImageFormat png;
    const bool ok = stream.openedOk() && png.writeImageToStream (image, stream);
    std::cout << (ok ? "wrote " + path : "could not write " + path) << "\n";
    return ok ? 0 : 1;
}
} // namespace

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
    juce::StringArray args;
    for (int i = 1; i < argc; ++i)
        args.add (argv[i]);

    if (args[0] == "--render")
        return renderVoice (args[1], args[2].getIntValue(), args[3]);
    if (args[0] == "--kit")
        return renderKit (args[1], args[2], args.size() > 3 ? args[3].getIntValue() : 2);
    if (args[0] == "--load")
        return loadPlugin (args[1]);
    if (args[0] == "--snapshot")
        return snapshot (args[1], args[2].getIntValue(), args[3].getIntValue(), args.size() > 4 ? args[4].getFloatValue() : 1.0f);

    testVoices();
    testSequencer();
    testProcessor();
    testMockupPreset();
    testStepPainting();
    std::cout << (failures == 0 ? "\nAll checks passed\n" : "\n" + std::to_string (failures) + " check(s) failed\n");
    return failures == 0 ? 0 : 1;
}
