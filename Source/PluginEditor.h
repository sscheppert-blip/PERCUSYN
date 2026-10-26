#pragma once

#include "PluginProcessor.h"
#include "ui/Pages.h"

// The rack in fixed logical coordinates (1536 x 1120); the editor scales it with the window.
class Rack : public juce::Component
{
public:
    explicit Rack (PercusynProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
    void refresh();
    void showTab (int tab);
    void selectVoice (int voice);

private:
    void loadPreset (int index);
    void showPresetMenu();
    void showOptionsMenu();
    void savePreset();
    void showHelp();
    void notify (const juce::String& message);
    juce::Rectangle<int> presetNameArea() const { return { 690, 24, 220, 62 }; }

    PercusynProcessor& proc;
    percusyn::ui::PanelButton prev { "prev" }, next { "next" }, init { "INIT" }, save { "SAVE" }, more { "more" };
    std::array<percusyn::ui::PanelButton, 4> tabs { percusyn::ui::PanelButton ("SYNTH"), percusyn::ui::PanelButton ("SEQ"),
                                                    percusyn::ui::PanelButton ("FX"), percusyn::ui::PanelButton ("MIX") };
    percusyn::ui::SynthPage synth;
    percusyn::ui::OverviewPage overview;
    percusyn::ui::FxPage fx;
    percusyn::ui::MixPage mix;
    percusyn::ui::InstrumentBar bar;
    percusyn::ui::ExportPanel exporter;
    percusyn::ui::LanePanel lane;
    int tab = 0;
    juce::String notice;
    int noticeTicks = 0;
    percusyn::VoiceParams lastSound {};
    int lastVelocity = -1;
    std::unique_ptr<juce::FileChooser> chooser;
};

class PercusynEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PercusynEditor (PercusynProcessor&);
    ~PercusynEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    percusyn::ui::PercusynLook look;
    Rack rack;
    juce::TooltipWindow tooltips { this, 600 };
};
