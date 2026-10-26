#include "ui/Pages.h"

namespace percusyn::ui
{
namespace
{
const juce::String middleDot = juce::String::charToString (0x00b7);

juce::String voiceTitle (int v)
{
    const juce::String label (voiceLabels[(size_t) v]);
    return label.substring (0, 1) + label.substring (1).toLowerCase();
}

juce::Path playIcon()
{
    juce::Path p;
    p.addTriangle (0, 0, 0, 20, 17, 10);
    return p;
}

juce::Path stopIcon()
{
    juce::Path p;
    p.addRectangle (0, 0, 16, 16);
    return p;
}

juce::Path patternIcon()
{
    juce::Path p;
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x)
            p.addRectangle ((float) x * 7.0f, (float) y * 7.0f, 5.5f, 5.5f);
    return p;
}

juce::Path dieIcon()
{
    juce::Path p;
    p.addRoundedRectangle (0, 0, 20, 20, 3);
    p.setUsingNonZeroWinding (false);
    const float dots[5][2] { { 5, 5 }, { 15, 5 }, { 10, 10 }, { 5, 15 }, { 15, 15 } };
    for (const auto& d : dots)
        p.addEllipse (d[0] - 1.8f, d[1] - 1.8f, 3.6f, 3.6f);
    return p;
}

constexpr std::array<int, 16> synthKnobParams { oscTune, oscDecay, oscTone, pitchAmount, pitchAttack, pitchDecay,
                                                ampAttack, ampDecay, ampSustain, filterCutoff, filterResonance,
                                                noiseColor, noiseLevel, driveAmount, pitchBend, velocitySens };
constexpr std::array<int, 8> metalParams { combTune, combFeedback, combDamping, combMix,
                                           resTune, resDecay, resDamping, resMix };

// Module rectangles of the SYNTH page, in page coordinates (1456 x 620).
struct SynthLayout
{
    juce::Rectangle<int> osc { 0, 0, 560, 256 }, pitchEnv { 568, 0, 424, 256 }, ampEnv { 1000, 0, 456, 256 };
    juce::Rectangle<int> filter { 0, 264, 520, 206 }, noise { 528, 264, 320, 206 }, drive { 856, 264, 184, 206 };
    juce::Rectangle<int> pitch { 1048, 264, 192, 206 }, velocity { 1248, 264, 208, 206 };
    juce::Rectangle<int> comb { 0, 478, 720, 142 }, resonator { 728, 478, 728, 142 };
};
const SynthLayout synthLayout;

void layoutKnobs (std::initializer_list<juce::Component*> knobs, juce::Rectangle<int> area, int knobWidth)
{
    const int count = (int) knobs.size();
    const int gap = (area.getWidth() - count * knobWidth) / (count + 1);
    int x = area.getX() + gap;
    for (auto* k : knobs)
    {
        k->setBounds (x, area.getY(), knobWidth, area.getHeight());
        x += knobWidth + gap;
    }
}
} // namespace

void bindVoice (ParamControl& c, juce::AudioProcessorValueTreeState& s, int voice, int param)
{
    const auto& def = voiceParamDefs[(size_t) param];
    c.bind (s, voiceParamId (voice, param), def, helpText (def.key));
}

void bindGlobal (ParamControl& c, juce::AudioProcessorValueTreeState& s, int param)
{
    const auto& def = globalParamDefs[(size_t) param];
    c.bind (s, globalParamId (param), def, helpText (def.key));
}

// =============================================================================================
SynthPage::SynthPage (PercusynProcessor& p) : proc (p)
{
    for (auto& k : knobs)
        addAndMakeVisible (k);
    for (auto& k : metal)
    {
        k.setShowLimits (false);
        addAndMakeVisible (k);
    }
    for (auto* c : std::initializer_list<juce::Component*> { &waves, &filterTypes, &combPower, &resPower, &model })
        addAndMakeVisible (c);
    knobs[14].setLabel ("BEND");
    setVoice (proc.selectedVoice);
    startTimerHz (15);
}

void SynthPage::setVoice (int voice)
{
    auto& s = proc.state;
    for (size_t i = 0; i < knobs.size(); ++i)
        bindVoice (knobs[i], s, voice, synthKnobParams[i]);
    for (size_t i = 0; i < metal.size(); ++i)
        bindVoice (metal[i], s, voice, metalParams[i]);
    bindVoice (waves, s, voice, oscWave);
    bindVoice (filterTypes, s, voice, filterType);
    bindVoice (combPower, s, voice, combEnabled);
    bindVoice (resPower, s, voice, resEnabled);
    model.bind (s, voiceParamId (voice, resModel), voiceParamDefs[resModel], helpText ("resonator_model"));
    timerCallback();
}

void SynthPage::timerCallback()
{
    const bool c = combPower.value() > 0.5f, r = resPower.value() > 0.5f;
    if (c != combOn || r != resOn)
    {
        combOn = c;
        resOn = r;
        for (int i = 0; i < 4; ++i)
        {
            metal[(size_t) i].setBypassed (! combOn);
            metal[(size_t) i + 4].setBypassed (! resOn);
        }
        repaint();
    }
}

void SynthPage::paint (juce::Graphics& g)
{
    const auto& L = synthLayout;
    drawModule (g, L.osc.toFloat(), "OSCILLATOR");
    drawModule (g, L.pitchEnv.toFloat(), "PITCH ENVELOPE");
    drawModule (g, L.ampEnv.toFloat(), "AMP ENVELOPE");
    drawModule (g, L.filter.toFloat(), "FILTER");
    drawModule (g, L.noise.toFloat(), "NOISE");
    drawModule (g, L.drive.toFloat(), "DRIVE");
    drawModule (g, L.pitch.toFloat(), "PITCH");
    drawModule (g, L.velocity.toFloat(), "VELOCITY");
    drawModule (g, L.comb.toFloat(), "COMB", ! combOn);
    drawModule (g, L.resonator.toFloat(), "RESONATOR", ! resOn);
    drawLabel (g, "MODEL", { (float) L.resonator.getRight() - 290, (float) L.resonator.getY() + 8, 60, 26 }, 13.0f,
               colours::muted, juce::Justification::centredRight);
}

void SynthPage::resized()
{
    const auto& L = synthLayout;
    auto row = [] (juce::Rectangle<int> m, int top, int height) { return m.withTrimmedTop (top).withHeight (height); };

    layoutKnobs ({ &knobs[0], &knobs[1], &knobs[2] }, row (L.osc, 36, 172), 150);
    waves.setBounds (L.osc.getX() + 16, L.osc.getY() + 210, L.osc.getWidth() - 32, 38);
    layoutKnobs ({ &knobs[3], &knobs[4], &knobs[5] }, row (L.pitchEnv, 36, 172), 128);
    layoutKnobs ({ &knobs[6], &knobs[7], &knobs[8] }, row (L.ampEnv, 36, 172), 132);

    layoutKnobs ({ &knobs[9], &knobs[10] }, row (L.filter.withTrimmedRight (150), 34, 166), 150);
    filterTypes.setBounds (L.filter.getRight() - 136, L.filter.getY() + 44, 112, 140);
    layoutKnobs ({ &knobs[11], &knobs[12] }, row (L.noise, 34, 166), 140);
    layoutKnobs ({ &knobs[13] }, row (L.drive, 34, 166), 150);
    layoutKnobs ({ &knobs[14] }, row (L.pitch, 34, 166), 150);
    layoutKnobs ({ &knobs[15] }, row (L.velocity, 34, 166), 150);

    layoutKnobs ({ &metal[0], &metal[1], &metal[2], &metal[3] }, row (L.comb, 30, 110), 130);
    layoutKnobs ({ &metal[4], &metal[5], &metal[6], &metal[7] }, row (L.resonator, 30, 110), 130);
    combPower.setBounds (L.comb.getRight() - 72, L.comb.getY() + 8, 58, 26);
    resPower.setBounds (L.resonator.getRight() - 72, L.resonator.getY() + 8, 58, 26);
    model.setBounds (L.resonator.getRight() - 224, L.resonator.getY() + 8, 140, 26);
}

// =============================================================================================
OverviewPage::OverviewPage (PercusynProcessor& p, std::function<void (int)> s) : proc (p), select (std::move (s)) {}

juce::Rectangle<float> OverviewPage::rowLabel (int voice) const
{
    return { 0, 104.0f + (float) voice * 56.0f, 150, 48 };
}

juce::Rectangle<float> OverviewPage::cell (int voice, int step) const
{
    const float x0 = 164, w = ((float) getWidth() - x0) / (float) numSteps;
    return { x0 + w * (float) step + 3, 104.0f + (float) voice * 56.0f, w - 6, 48 };
}

void OverviewPage::paint (juce::Graphics& g)
{
    drawModule (g, getLocalBounds().toFloat(), {});
    drawLabel (g, "SEQUENCER / ALL INSTRUMENTS", { 18, 10, 600, 28 }, 19.0f, colours::text, juce::Justification::centredLeft);
    drawLabel (g, "Left-click or drag to draw steps, right-click or drag to erase. Click an instrument name to select its detail lane.", { 18, 40, 900, 22 },
               14.0f, colours::muted, juce::Justification::centredLeft, false);

    const int length = (int) proc.state.getRawParameterValue (globalParamId (GlobalParam::length))->load();
    for (int s = 0; s < numSteps; ++s)
    {
        auto c = cell (0, s);
        drawLabel (g, juce::String (s + 1), { c.getX(), 74, c.getWidth(), 24 }, 15.0f,
                   s == playing ? colours::orange : colours::muted);
    }
    for (int v = 0; v < numVoices; ++v)
    {
        const bool selected = v == proc.selectedVoice;
        auto label = rowLabel (v).withTrimmedLeft (12);
        drawButtonFace (g, label, selected, false, colours::orange);
        drawLabel (g, voiceLabels[(size_t) v], label, 14.0f, selected ? colours::orange : colours::text);
        for (int s = 0; s < numSteps; ++s)
        {
            const auto st = proc.pattern.get (v, s);
            auto c = cell (v, s);
            const auto accent = s % 4 == 0 ? colours::orange : colours::blue;
            g.setColour (st.active ? accent.withAlpha (0.25f + 0.75f * (float) st.velocity / 127.0f) : juce::Colour (0xff262626));
            g.fillRoundedRectangle (c, 4.0f);
            if (s >= length)
            {
                g.setColour (juce::Colours::black.withAlpha (0.55f));
                g.fillRoundedRectangle (c, 4.0f);
            }
            if (s == playing)
            {
                g.setColour (juce::Colours::white.withAlpha (0.8f));
                g.drawRoundedRectangle (c.reduced (1), 4.0f, 2.0f);
            }
        }
    }
}

void OverviewPage::mouseDown (const juce::MouseEvent& e)
{
    for (int v = 0; v < numVoices; ++v)
    {
        if (rowLabel (v).contains (e.position))
        {
            select (v);
            return;
        }
    }
    paintValue = e.mods.isRightButtonDown() ? 0.0f : 1.0f;
    painting = paintAt (e.position);
}

void OverviewPage::mouseDrag (const juce::MouseEvent& e)
{
    if (painting)
        paintAt (e.position);
}

bool OverviewPage::paintAt (juce::Point<float> pos)
{
    for (int v = 0; v < numVoices; ++v)
        for (int s = 0; s < numSteps; ++s)
            if (cell (v, s).expanded (3, 4).contains (pos))
            {
                if (proc.pattern.getField (v, s, StepField::Active) != paintValue)
                {
                    proc.pattern.setField (v, s, StepField::Active, paintValue);
                    repaint();
                }
                return true;
            }
    return false;
}

void OverviewPage::refresh (int step)
{
    if (step != playing || proc.pattern.version() != lastVersion)
    {
        playing = step;
        lastVersion = proc.pattern.version();
        repaint();
    }
}

// =============================================================================================
FxPage::FxPage (PercusynProcessor& p)
{
    const int params[] { delayTime, delayFeedback, delayMix, reverbSize, reverbDecay, reverbMix };
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        bindGlobal (knobs[i], p.state, params[i]);
        addAndMakeVisible (knobs[i]);
    }
}

void FxPage::paint (juce::Graphics& g)
{
    drawLabel (g, "EFFECTS / GLOBAL", { 4, 0, 600, 30 }, 19.0f, colours::text, juce::Justification::centredLeft);
    drawModule (g, { 0, 40, 720, 318 }, "DELAY");
    drawModule (g, { 736, 40, 720, 318 }, "REVERB");
    drawLabel (g, "Stereo delay on the FX bus. Adjust each instrument's FX send in the mixer.", { 18, 318, 690, 28 }, 14.0f,
               colours::muted, juce::Justification::centredLeft, false);
    drawLabel (g, "Room size, decay and return level are stored with the preset.", { 754, 318, 690, 28 }, 14.0f,
               colours::muted, juce::Justification::centredLeft, false);

    drawModule (g, { 0, 372, 1456, 248 }, "ROUTING / MIDI");
    const juce::String arrow = " " + juce::String::charToString (0x2192) + " ";
    const juce::String lines[] {
        "Signal path: Instrument" + arrow + "Drive" + arrow + "Filter" + arrow + "Comb" + arrow + "Resonator" + arrow
            + "Level / Pan" + arrow + "Main or voice output.  Send FX" + arrow + "Delay + Reverb" + arrow + "Main.",
        "MIDI notes (any channel): C1 Closed Hat, C#1 Open Hat, D1 Snare, D#1 Tom, E1 Shaker, F1 Clap, F#1 Perc, G1 Rim, G#1 Click.",
        "Closed Hat chokes Open Hat. Every instrument retriggers monophonically. The pitch wheel bends each instrument by its BEND range.",
        "Outputs: Main stereo plus one stereo output per instrument. Enable an instrument output in the host to route it there.",
        "Sequencer: HOST off - while the host plays, the internal sequencer is off and MIDI plays the instruments. HOST lit - it follows the host transport.",
    };
    float y = 412;
    for (const auto& l : lines)
    {
        drawLabel (g, l, { 18, y, 1420, 26 }, 15.0f, colours::text, juce::Justification::centredLeft, false);
        y += 38;
    }
}

void FxPage::resized()
{
    layoutKnobs ({ &knobs[0], &knobs[1], &knobs[2] }, { 0, 84, 720, 220 }, 190);
    layoutKnobs ({ &knobs[3], &knobs[4], &knobs[5] }, { 736, 84, 720, 220 }, 190);
}

// =============================================================================================
MixPage::MixPage (PercusynProcessor& p)
{
    for (int v = 0; v < numVoices; ++v)
    {
        auto& s = strips[(size_t) v];
        bindVoice (s.pan, p.state, v, pan);
        bindVoice (s.level, p.state, v, level);
        bindVoice (s.send, p.state, v, sendFx);
        bindVoice (s.mute, p.state, v, mute);
        bindVoice (s.solo, p.state, v, solo);
        s.pan.setLabel ("PAN");
        s.send.setLabel ("SEND FX");
        s.level.setLabel ("LEVEL");
        s.pan.setShowLimits (false);
        s.send.setShowLimits (false);
        for (auto* c : std::initializer_list<juce::Component*> { &s.pan, &s.level, &s.send, &s.mute, &s.solo })
            addAndMakeVisible (c);
    }
    bindGlobal (master, p.state, masterLevel);
    master.setLabel ("MASTER");
    master.setShowLimits (false);
    addAndMakeVisible (master);
}

void MixPage::paint (juce::Graphics& g)
{
    drawLabel (g, "MIXER / NINE VOICES", { 4, 14, 600, 30 }, 19.0f, colours::text, juce::Justification::centredLeft);
    const float w = 1296.0f / numVoices;
    for (int v = 0; v < numVoices; ++v)
    {
        auto area = juce::Rectangle<float> (w * (float) v, 66, w, 554).reduced (4, 0);
        drawModule (g, area, {});
        drawLabel (g, voiceLabels[(size_t) v], area.withHeight (30).translated (0, 4), 14.0f, colours::text);
    }
    drawModule (g, { 1304, 66, 152, 554 }, {});
    drawLabel (g, "MAIN", { 1304, 70, 152, 30 }, 14.0f, colours::text);
    g.setFont (font (13.0f, false));
    g.setColour (colours::muted);
    g.drawFittedText ("Instruments with their own output enabled in the host bypass the main mix.", { 1318, 330, 124, 120 },
                      juce::Justification::centredTop, 6);
}

void MixPage::resized()
{
    master.setBounds (1310, 110, 140, 190);
    const int w = 1296 / numVoices;
    for (int v = 0; v < numVoices; ++v)
    {
        auto& s = strips[(size_t) v];
        const int x = w * v + 4, sw = w - 8;
        s.pan.setBounds (x, 100, sw, 106);
        s.level.setBounds (x, 210, sw, 240);
        s.send.setBounds (x, 450, sw, 106);
        s.mute.setBounds (x + sw / 2 - 52, 568, 48, 36);
        s.solo.setBounds (x + sw / 2 + 4, 568, 48, 36);
    }
}

// =============================================================================================
InstrumentBar::InstrumentBar (PercusynProcessor& p, std::function<void (int)> selectVoice, std::function<void()> showPattern)
    : proc (p)
{
    for (int v = 0; v < numVoices; ++v)
    {
        auto* b = voices.add (new PanelButton (voiceLabels[(size_t) v]));
        b->setIcon (instrumentIcon (v), false);
        b->setSubText (voiceLabels[(size_t) v]);
        b->setTooltip ("Selects " + voiceTitle (v) + " for editing and plays it (MIDI note "
                       + juce::MidiMessage::getMidiNoteName (firstMidiNote + v, true, true, 3) + ").");
        b->onClick = [this, v, selectVoice]
        {
            selectVoice (v);
            proc.audition (v, proc.pattern.get (v, proc.selectedStep).velocity);
        };
        addAndMakeVisible (b);

        auto* m = mutes.add (new Toggle (Toggle::Style::Mute, "M"));
        auto* s = solos.add (new Toggle (Toggle::Style::Solo, "S"));
        bindVoice (*m, proc.state, v, mute);
        bindVoice (*s, proc.state, v, solo);
        addAndMakeVisible (m);
        addAndMakeVisible (s);
    }
    play.setIcon (playIcon(), true);
    stop.setIcon (stopIcon(), true);
    pattern.setIcon (patternIcon(), true);
    pattern.setSubText ("PATTERN");
    random.setIcon (dieIcon(), true);
    random.setSubText ("RANDOM");
    host.setTextSize (14.0f);

    play.setTooltip ("Starts the internal sequencer. While the host plays, the sequencer is controlled by the HOST setting.");
    stop.setTooltip ("Stops the internal sequencer and returns the play position to step 1.");
    pattern.setTooltip ("Opens the full pattern view. Lights only while the SEQ view is selected.");
    random.setTooltip ("Randomizes step on/off states and velocities for the selected instrument only.");
    host.setTooltip (helpText ("host_sync"));

    play.onClick = [this] { proc.requestPlay (true); };
    stop.onClick = [this] { proc.requestPlay (false); };
    pattern.onClick = std::move (showPattern);
    random.onClick = [this]
    {
        juce::Random rng;
        const int v = proc.selectedVoice;
        for (int i = 0; i < numSteps; ++i)
        {
            proc.pattern.setField (v, i, StepField::Active, rng.nextFloat() < 0.45f ? 1.0f : 0.0f);
            proc.pattern.setField (v, i, StepField::Velocity, (float) (60 + rng.nextInt (68)));
        }
    };
    host.onClick = [this]
    {
        if (auto* param = proc.state.getParameter (globalParamId (hostSync)))
            param->setValueNotifyingHost (param->getValue() > 0.5f ? 0.0f : 1.0f);
    };

    bindGlobal (lengthField, proc.state, GlobalParam::length);
    bindGlobal (swingField, proc.state, swing);
    for (auto* c : std::initializer_list<juce::Component*> { &play, &stop, &host, &pattern, &random, &lengthField, &swingField })
        addAndMakeVisible (c);
}

void InstrumentBar::resized()
{
    for (int v = 0; v < numVoices; ++v)
    {
        voices[v]->setBounds (v * 102, 0, 98, 62);
        mutes[v]->setBounds (v * 102, 64, 48, 26);
        solos[v]->setBounds (v * 102 + 50, 64, 48, 26);
    }
    play.setBounds (930, 14, 62, 62);
    stop.setBounds (996, 14, 62, 62);
    host.setBounds (1062, 14, 66, 62);
    pattern.setBounds (1134, 6, 76, 78);
    random.setBounds (1214, 6, 76, 78);
    lengthField.setBounds (1296, 8, 76, 70);
    swingField.setBounds (1378, 8, 78, 70);
}

void InstrumentBar::refresh (int selectedVoice, bool patternView)
{
    for (int v = 0; v < numVoices; ++v)
    {
        const auto t = proc.triggerCount (v);
        if (t != lastTriggers[(size_t) v])
        {
            lastTriggers[(size_t) v] = t;
            flash[(size_t) v] = 3;
        }
        voices[v]->setLit (v == selectedVoice || flash[(size_t) v] > 0);
        if (flash[(size_t) v] > 0)
            --flash[(size_t) v];
    }
    const bool running = proc.isSequencerRunning();
    play.setLit (running);
    pattern.setLit (patternView);
    host.setLit (proc.state.getRawParameterValue (globalParamId (hostSync))->load() > 0.5f);
    const bool hostBlocks = proc.isHostPlaying();
    play.setEnabled (! hostBlocks);
    stop.setEnabled (! hostBlocks);
}

// =============================================================================================
namespace
{
const char* laneRows[] { "STEP", "VEL", "PROB", "COND" };
const StepField laneFields[] { StepField::Active, StepField::Velocity, StepField::Probability, StepField::Condition };
const char* conditionNames[] { ".", "1:2", "2:2", "1:4" };
} // namespace

LanePanel::LanePanel (PercusynProcessor& p) : proc (p)
{
    bindGlobal (tempoField, proc.state, tempo);
    clear.setTooltip ("Disables all steps in the selected lane. Their detail values are retained.");
    clear.setTextSize (13.0f);
    clear.onClick = [this]
    {
        proc.pattern.clearLane (proc.selectedVoice);
        repaint();
    };
    addAndMakeVisible (tempoField);
    addAndMakeVisible (clear);
    setTooltip ("STEP: left-click or drag to draw steps (the clicked step is used for one-shot renders), right-click or "
                "drag to erase. VEL/PROB: drag or scroll, "
                "double-click to type. COND: click to cycle ALL, 1:2, 2:2, 1:4.");
}

juce::Rectangle<float> LanePanel::cell (int row, int step) const
{
    const float x0 = 74, w = ((float) getWidth() - x0 - 12) / (float) numSteps;
    const float tops[] { 76, 124, 162, 200 };
    const float heights[] { 40, 32, 32, 32 };
    return { x0 + w * (float) step + 2, tops[row], w - 4, heights[row] };
}

bool LanePanel::hit (juce::Point<float> p, int& row, int& step) const
{
    for (int r = 0; r < 4; ++r)
        for (int s = 0; s < numSteps; ++s)
            if (cell (r, s).contains (p))
            {
                row = r;
                step = s;
                return true;
            }
    return false;
}

void LanePanel::paint (juce::Graphics& g)
{
    drawModule (g, getLocalBounds().toFloat(), {});
    const int v = proc.selectedVoice;
    drawLabel (g, juce::String (voiceLabels[(size_t) v]) + " / PATTERN 01", { 16, 10, 400, 30 }, 16.0f, colours::text,
               juce::Justification::centredLeft);

    const int length = (int) proc.state.getRawParameterValue (globalParamId (GlobalParam::length))->load();
    for (int s = 0; s < numSteps; ++s)
    {
        auto c = cell (0, s);
        drawLabel (g, juce::String (s + 1), { c.getX(), 50, c.getWidth(), 22 }, 14.0f,
                   s == playing ? colours::orange : (s % 4 == 0 ? colours::text : colours::muted));
    }
    for (int r = 0; r < 4; ++r)
    {
        auto c = cell (r, 0);
        drawLabel (g, laneRows[r], { 10, c.getY(), 60, c.getHeight() }, 13.0f, colours::muted, juce::Justification::centredLeft);
        for (int s = 0; s < numSteps; ++s)
        {
            const auto st = proc.pattern.get (v, s);
            auto box = cell (r, s);
            const auto accent = s % 4 == 0 ? colours::orange : colours::blue;
            if (r == 0)
            {
                g.setColour (st.active ? accent : juce::Colour (0xff2a2a2a));
                g.fillRoundedRectangle (box, 3.0f);
                if (s == playing)
                {
                    g.setColour (juce::Colours::white.withAlpha (0.85f));
                    g.drawRoundedRectangle (box.reduced (1), 3.0f, 2.0f);
                }
                if (s == proc.selectedStep)
                {
                    g.setColour (colours::text);
                    g.fillRect (box.getX() + 4, box.getBottom() + 2, box.getWidth() - 8, 3.0f);
                }
            }
            else
            {
                g.setColour (juce::Colour (0xff202020));
                g.fillRoundedRectangle (box, 3.0f);
                float fill = 0;
                juce::String text;
                if (r == 1) { fill = (float) st.velocity / 127.0f; text = juce::String (st.velocity); }
                else if (r == 2) { fill = st.probability / 100.0f; text = juce::String ((int) st.probability); }
                else text = st.condition == 0 ? middleDot : juce::String (conditionNames[st.condition]);
                if (fill > 0)
                {
                    g.setColour (juce::Colour (0xff3a3a3a));
                    g.fillRoundedRectangle (box.withTop (box.getBottom() - box.getHeight() * fill), 3.0f);
                }
                drawLabel (g, text, box, 12.5f, st.active ? colours::text : colours::dim, juce::Justification::centred, false);
            }
            if (s >= length)
            {
                g.setColour (juce::Colours::black.withAlpha (0.55f));
                g.fillRoundedRectangle (box, 3.0f);
            }
        }
    }
}

void LanePanel::resized()
{
    tempoField.setBounds (getWidth() - 200, 4, 86, 46);
    clear.setBounds (getWidth() - 104, 14, 88, 30);
}

void LanePanel::mouseDown (const juce::MouseEvent& e)
{
    dragRow = -1;
    painting = false;
    int row, step;
    if (! hit (e.position, row, step))
        return;
    const int v = proc.selectedVoice;
    if (row == 0)
    {
        paintValue = e.mods.isRightButtonDown() ? 0.0f : 1.0f;
        painting = true;
        proc.pattern.setField (v, step, StepField::Active, paintValue);
        if (paintValue > 0.5f)
        {
            proc.selectedStep = step;
            if (onSelectStep)
                onSelectStep();
        }
    }
    else if (row == 3)
        proc.pattern.setField (v, step, StepField::Condition,
                               (float) ((proc.pattern.get (v, step).condition + (e.mods.isShiftDown() ? 3 : 1)) % 4));
    else
    {
        dragRow = row;
        dragStep = step;
        dragStartValue = proc.pattern.getField (v, step, laneFields[row]);
    }
    repaint();
}

void LanePanel::mouseDrag (const juce::MouseEvent& e)
{
    if (painting)
    {
        // Follow the column under the mouse, even if the drag leaves the STEP row a little.
        for (int s = 0; s < numSteps; ++s)
        {
            const auto c = cell (0, s);
            if (e.position.x >= c.getX() - 2 && e.position.x < c.getRight() + 2 && std::abs (e.position.y - c.getCentreY()) < 60)
            {
                if (proc.pattern.getField (proc.selectedVoice, s, StepField::Active) != paintValue)
                {
                    proc.pattern.setField (proc.selectedVoice, s, StepField::Active, paintValue);
                    repaint();
                }
                break;
            }
        }
        return;
    }
    if (dragRow < 0)
        return;
    const float range = dragRow == 1 ? 127.0f : 100.0f;
    proc.pattern.setField (proc.selectedVoice, dragStep, laneFields[dragRow],
                           dragStartValue - (float) e.getDistanceFromDragStartY() * range / 150.0f);
    if (dragRow == 1 && onSelectStep)
        onSelectStep();
    repaint();
}

void LanePanel::mouseUp (const juce::MouseEvent&)
{
    dragRow = -1;
    painting = false;
}

void LanePanel::mouseDoubleClick (const juce::MouseEvent& e)
{
    int row, step;
    if (! hit (e.position, row, step) || row == 0 || row == 3)
        return;
    const int v = proc.selectedVoice;
    juce::Component::SafePointer<LanePanel> safe (this);
    showValueEntry (*this, juce::String (laneRows[row]) + " - step " + juce::String (step + 1),
                    row == 1 ? "Velocity from 1 to 127" : "Probability (%) from 0 to 100",
                    proc.pattern.getField (v, step, laneFields[row]), [safe, v, step, row] (float value)
                    {
                        if (safe != nullptr)
                        {
                            safe->proc.pattern.setField (v, step, laneFields[row], value);
                            safe->repaint();
                        }
                    });
}

void LanePanel::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    int row, step;
    if (! hit (e.position, row, step) || row == 0 || row == 3)
        return;
    const int v = proc.selectedVoice;
    const float delta = (w.deltaY > 0 ? 1.0f : -1.0f) * (e.mods.isShiftDown() ? 1.0f : 5.0f);
    proc.pattern.setField (v, step, laneFields[row], proc.pattern.getField (v, step, laneFields[row]) + delta);
    repaint();
}

void LanePanel::refresh (int step)
{
    if (step != playing || proc.pattern.version() != lastVersion)
    {
        playing = step;
        lastVersion = proc.pattern.version();
        repaint();
    }
}

// =============================================================================================
ExportPanel::ExportPanel (PercusynProcessor& p) : proc (p)
{
    rate.addItem ("44.1 kHz", 1);
    rate.addItem ("48 kHz", 2);
    rate.addItem ("96 kHz", 3);
    depth.addItem ("16 BIT", 1);
    depth.addItem ("24 BIT", 2);
    depth.addItem ("32 FLOAT", 3);
    const double r = proc.exportSettings.sampleRate;
    rate.setSelectedId (r < 46000 ? 1 : (r < 60000 ? 2 : 3), juce::dontSendNotification);
    depth.setSelectedId (proc.exportSettings.bitDepth == 16 ? 1 : (proc.exportSettings.bitDepth == 32 ? 3 : 2),
                         juce::dontSendNotification);
    rate.onChange = [this]
    {
        proc.exportSettings.sampleRate = rate.getSelectedId() == 1 ? 44100.0 : (rate.getSelectedId() == 2 ? 48000.0 : 96000.0);
        stale = true;
        repaint();
    };
    depth.onChange = [this]
    {
        proc.exportSettings.bitDepth = depth.getSelectedId() == 1 ? 16 : (depth.getSelectedId() == 3 ? 32 : 24);
        file = juce::File();
        repaint();
    };
    rate.setTooltip ("Sample rate of the rendered one-shot.");
    depth.setTooltip ("Bit depth of the WAV file: 16 or 24 bit PCM, or 32-bit float.");

    renderButton.setTooltip ("Renders the selected instrument as a one-shot at the velocity of the selected step. "
                             "Includes comb, resonator, level and pan; no global FX or master.");
    previewButton.setTooltip ("Plays the rendered one-shot on the main output.");
    wavButton.setTooltip ("Saves the rendered one-shot as a WAV file.");
    renderButton.onClick = [this] { render(); };
    previewButton.onClick = [this]
    {
        if (stale || audio == nullptr)
            render();
        if (audio != nullptr)
            proc.startPreview (audio, audioRate);
    };
    wavButton.onClick = [this] { saveAs(); };
    for (auto* c : std::initializer_list<juce::Component*> { &rate, &depth, &renderButton, &previewButton, &wavButton })
        addAndMakeVisible (c);
}

juce::Rectangle<float> ExportPanel::viewArea() const
{
    return { 16, 46, (float) getWidth() - 32, 92 };
}

juce::Rectangle<float> ExportPanel::dragArea() const
{
    return { 388, 194, (float) getWidth() - 404, 34 };
}

void ExportPanel::resized()
{
    rate.setBounds (172, 150, 110, 30);
    depth.setBounds (364, 150, 120, 30);
    renderButton.setBounds (16, 194, 116, 34);
    previewButton.setBounds (138, 194, 116, 34);
    wavButton.setBounds (260, 194, 116, 34);
}

void ExportPanel::paint (juce::Graphics& g)
{
    drawModule (g, getLocalBounds().toFloat(), "ONE-SHOT / EXPORT");
    const int v = proc.selectedVoice;
    const int velocity = proc.pattern.get (v, proc.selectedStep).velocity;
    const auto badge = juce::Rectangle<float> ((float) getWidth() - 236, 12, 220, 24);
    g.setColour (colours::display);
    g.fillRoundedRectangle (badge, 3.0f);
    drawLabel (g, "VEL " + juce::String (velocity) + " " + middleDot + " STEP " + juce::String (proc.selectedStep + 1), badge,
               12.5f, colours::orange);

    // View: rendered waveform, otherwise the amp envelope of the selected instrument.
    const auto view = viewArea();
    g.setColour (colours::display);
    g.fillRoundedRectangle (view, 4.0f);
    g.setColour (juce::Colour (0xff2e2e2e));
    g.drawRoundedRectangle (view, 4.0f, 1.0f);
    const auto inner = view.reduced (8, 10);

    if (audio != nullptr && ! stale)
    {
        drawLabel (g, "RENDERED ONE-SHOT", view.reduced (8, 4).withHeight (14), 11.0f, colours::muted,
                   juce::Justification::topLeft);
        const int n = audio->getNumSamples();
        const float peak = juce::jmax (1.0e-6f, audio->getMagnitude (0, n));
        juce::Path wave;
        const int columns = (int) inner.getWidth();
        for (int x = 0; x < columns; ++x)
        {
            const int a = (int) ((int64_t) n * x / columns), b = juce::jmax (a + 1, (int) ((int64_t) n * (x + 1) / columns));
            const auto range = audio->findMinMax (0, a, juce::jmin (b, n) - a);
            const float top = inner.getCentreY() - range.getEnd() / peak * inner.getHeight() * 0.48f;
            const float bottom = inner.getCentreY() - range.getStart() / peak * inner.getHeight() * 0.48f;
            wave.addRectangle (inner.getX() + (float) x, top, 1.0f, juce::jmax (1.0f, bottom - top));
        }
        g.setColour (colours::orange);
        g.fillPath (wave);
    }
    else
    {
        drawLabel (g, "AMP ENVELOPE", view.reduced (8, 4).withHeight (14), 11.0f, colours::muted, juce::Justification::topLeft);
        const auto p = proc.voiceSnapshot (v);
        const float a = p[ampAttack], d = p[ampDecay], s = p[ampSustain] * 0.01f;
        const float total = juce::jmax (10.0f, a + d * (s > 0 ? 1.6f : 1.2f));
        juce::Path env;
        env.startNewSubPath (inner.getX(), inner.getBottom());
        for (int x = 0; x <= (int) inner.getWidth(); x += 2)
        {
            const float t = total * (float) x / inner.getWidth();
            float level = t < a ? t / a : s + (1.0f - s) * std::exp (-6.9f * (t - a) / d);
            if (s > 0 && t > a + d * 0.8f)
                level = s * std::exp (-6.9f * (t - a - d * 0.8f) / d);
            env.lineTo (inner.getX() + (float) x, inner.getBottom() - level * inner.getHeight());
        }
        g.setColour (colours::orange);
        g.strokePath (env, juce::PathStrokeType (1.6f));
        const auto def = [] (int i) -> const ParamDef& { return voiceParamDefs[(size_t) i]; };
        drawLabel (g, "A " + formatText (def (ampAttack), a) + "  D " + formatText (def (ampDecay), d) + "  S "
                          + formatText (def (ampSustain), p[ampSustain]),
                   inner.withTrimmedTop (inner.getHeight() - 14), 11.0f, colours::orange, juce::Justification::bottomRight,
                   false);
    }

    drawLabel (g, "FORMAT", { 16, 150, 70, 30 }, 13.0f, colours::muted, juce::Justification::centredLeft);
    drawLabel (g, "WAV", { 76, 150, 50, 30 }, 13.0f, colours::text, juce::Justification::centredLeft);
    drawLabel (g, "RATE", { 124, 150, 46, 30 }, 13.0f, colours::muted, juce::Justification::centredLeft);
    drawLabel (g, "DEPTH", { 300, 150, 62, 30 }, 13.0f, colours::muted, juce::Justification::centredLeft);

    // Drag source.
    const auto drag = dragArea();
    drawButtonFace (g, drag, false, false, colours::orange);
    for (int i = 0; i < 6; ++i)
    {
        g.setColour (colours::muted);
        g.fillEllipse (drag.getX() + 14 + (float) (i % 2) * 6, drag.getY() + 9 + (float) (i / 2) * 6, 3, 3);
    }
    drawLabel (g, "DRAG TO DAW", drag.withTrimmedLeft (24), 13.5f, colours::text);

    const auto info = status.isNotEmpty() ? status
                                          : juce::String (voiceLabels[(size_t) v]) + " " + middleDot
                                                + " drag the one-shot into your arrangement";
    drawLabel (g, info, { 16, 232, (float) getWidth() - 32, 20 }, 12.0f, colours::muted, juce::Justification::centredLeft,
               false);
}

void ExportPanel::render()
{
    const int v = proc.selectedVoice;
    const int velocity = proc.pattern.get (v, proc.selectedStep).velocity;
    audioRate = proc.exportSettings.sampleRate;
    audio = std::make_shared<juce::AudioBuffer<float>> (renderOneShot (v, proc.voiceSnapshot (v), velocity, audioRate));
    stale = false;
    file = juce::File();
    status = juce::String (voiceLabels[(size_t) v]) + " " + middleDot + " " + juce::String (audio->getNumSamples() / audioRate, 2)
             + " s " + middleDot + " VEL " + juce::String (velocity) + " " + middleDot + " peak "
             + juce::String (juce::Decibels::gainToDecibels (audio->getMagnitude (0, audio->getNumSamples())), 1) + " dBFS";
    repaint();
}

bool ExportPanel::ensureFile()
{
    if (stale || audio == nullptr)
        render();
    if (file.existsAsFile() && audioDepth == proc.exportSettings.bitDepth)
        return true;
    juce::String error;
    if (! renderFolder().createDirectory())
    {
        status = "Could not create " + renderFolder().getFullPathName();
        repaint();
        return false;
    }
    auto target = newRenderFile (proc.selectedVoice, proc.presetName);
    if (! writeWav (*audio, proc.exportSettings, target, error))
    {
        status = error;
        repaint();
        return false;
    }
    file = target;
    audioDepth = proc.exportSettings.bitDepth;
    status = "Saved " + file.getFileName();
    repaint();
    return true;
}

void ExportPanel::saveAs()
{
    if (! ensureFile())
        return;
    chooser = std::make_unique<juce::FileChooser> ("Save one-shot WAV", renderFolder().getChildFile (file.getFileName()), "*.wav");
    juce::Component::SafePointer<ExportPanel> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe] (const juce::FileChooser& fc)
                          {
                              if (safe == nullptr)
                                  return;
                              auto target = fc.getResult();
                              if (target == juce::File())
                                  return;
                              if (! target.hasFileExtension ("wav"))
                                  target = target.withFileExtension ("wav");
                              if (target != safe->file)
                                  safe->file.copyFileTo (target);
                              safe->status = "Saved " + target.getFullPathName();
                              safe->repaint();
                          });
}

void ExportPanel::mouseDown (const juce::MouseEvent&)
{
    dragStarted = false;
}

void ExportPanel::mouseDrag (const juce::MouseEvent& e)
{
    if (dragStarted || ! dragArea().contains (e.mouseDownPosition) || e.getDistanceFromDragStart() < 6)
        return;
    dragStarted = true;
    if (ensureFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
}

void ExportPanel::voiceOrStepChanged()
{
    stale = true;
    status.clear();
    repaint();
}

void ExportPanel::soundChanged()
{
    if (! stale)
    {
        stale = true;
        status.clear();
    }
    repaint();
}
} // namespace percusyn::ui
