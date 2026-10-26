#pragma once

// Rack sections: the four tab pages (fixed 620 px main area), the instrument bar and the two
// bottom panels (one-shot export, selected-voice step lane).

#include "PluginProcessor.h"
#include "ui/Widgets.h"

namespace percusyn::ui
{
void bindVoice (ParamControl& c, juce::AudioProcessorValueTreeState& s, int voice, int param);
void bindGlobal (ParamControl& c, juce::AudioProcessorValueTreeState& s, int param);

class SynthPage : public juce::Component, private juce::Timer
{
public:
    explicit SynthPage (PercusynProcessor&);
    void setVoice (int voice);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    PercusynProcessor& proc;
    std::array<Knob, 16> knobs;   // VoiceParam order up to velocitySens, without wave/type
    std::array<Knob, 8> metal { Knob (true), Knob (true), Knob (true), Knob (true),
                                Knob (true), Knob (true), Knob (true), Knob (true) };
    WaveButtons waves;
    FilterTypes filterTypes;
    Toggle combPower { Toggle::Style::Power }, resPower { Toggle::Style::Power };
    ChoiceBox model;
    bool combOn = false, resOn = false;
};

class OverviewPage : public juce::Component
{
public:
    OverviewPage (PercusynProcessor&, std::function<void (int)> selectVoice);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { painting = false; }
    void refresh (int playingStep);

private:
    juce::Rectangle<float> cell (int voice, int step) const;
    juce::Rectangle<float> rowLabel (int voice) const;
    bool paintAt (juce::Point<float>);
    PercusynProcessor& proc;
    std::function<void (int)> select;
    int playing = -1;
    bool painting = false;
    float paintValue = 1;   // left button draws, right button erases
    std::uint32_t lastVersion = 0;
};

class FxPage : public juce::Component
{
public:
    explicit FxPage (PercusynProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::array<Knob, 6> knobs;
};

class MixPage : public juce::Component
{
public:
    explicit MixPage (PercusynProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Strip
    {
        Knob pan { true }, send { true };
        Fader level;
        Toggle mute { Toggle::Style::Mute, "M" }, solo { Toggle::Style::Solo, "S" };
    };
    std::array<Strip, numVoices> strips;
    Knob master;
};

class InstrumentBar : public juce::Component
{
public:
    InstrumentBar (PercusynProcessor&, std::function<void (int)> selectVoice, std::function<void()> showPattern);
    void resized() override;
    void refresh (int selectedVoice, bool patternView);

private:
    PercusynProcessor& proc;
    juce::OwnedArray<PanelButton> voices;
    juce::OwnedArray<Toggle> mutes, solos;
    PanelButton play { "play" }, stop { "stop" }, host { "HOST", colours::blue }, pattern { "pattern", colours::blue },
        random { "random" };
    NumberField lengthField { "LENGTH" }, swingField { "SWING" };
    std::array<std::uint32_t, numVoices> lastTriggers {};
    std::array<int, numVoices> flash {};
};

class LanePanel : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit LanePanel (PercusynProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void refresh (int playingStep);
    std::function<void()> onSelectStep;

private:
    bool hit (juce::Point<float>, int& row, int& step) const;
    juce::Rectangle<float> cell (int row, int step) const;
    PercusynProcessor& proc;
    NumberField tempoField { "BPM" };
    PanelButton clear { "CLEAR" };
    int playing = -1, dragRow = -1, dragStep = -1;
    float dragStartValue = 0;
    bool painting = false;
    float paintValue = 1;   // left button draws, right button erases
    std::uint32_t lastVersion = 0;
};

class ExportPanel : public juce::Component, public juce::DragAndDropContainer
{
public:
    explicit ExportPanel (PercusynProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void voiceOrStepChanged();
    void soundChanged();

private:
    void render();
    bool ensureFile();
    void saveAs();
    juce::Rectangle<float> dragArea() const;
    juce::Rectangle<float> viewArea() const;

    PercusynProcessor& proc;
    juce::ComboBox rate, depth;
    PanelButton renderButton { "RENDER" }, previewButton { "PREVIEW" }, wavButton { "WAV" };
    std::shared_ptr<juce::AudioBuffer<float>> audio;
    double audioRate = 48000;
    int audioDepth = 24;
    juce::File file;   // written render of `audio`, kept on disk for drags
    bool stale = true, dragStarted = false;
    juce::String status;
    std::unique_ptr<juce::FileChooser> chooser;
};
} // namespace percusyn::ui
