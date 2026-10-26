#include "PluginEditor.h"

using namespace percusyn;
using namespace percusyn::ui;

namespace
{
juce::Path chevron (bool left)
{
    juce::Path p;
    p.startNewSubPath (left ? 8.0f : 0.0f, 0.0f);
    p.lineTo (left ? 0.0f : 8.0f, 8.0f);
    p.lineTo (left ? 8.0f : 0.0f, 16.0f);
    return p;
}

const juce::String bullets = juce::String::charToString (0x2022) + juce::String::charToString (0x2022)
                             + juce::String::charToString (0x2022);
} // namespace

Rack::Rack (PercusynProcessor& p)
    : proc (p), synth (p), overview (p, [this] (int v) { selectVoice (v); }), fx (p), mix (p),
      bar (p, [this] (int v) { selectVoice (v); }, [this] { showTab (1); }), exporter (p), lane (p)
{
    prev.setIcon (chevron (true), false);
    next.setIcon (chevron (false), false);
    more.setButtonText (bullets);
    prev.setTooltip ("Loads the previous preset. Save any edits you want to keep first.");
    next.setTooltip ("Loads the next preset. Save any edits you want to keep first.");
    init.setTooltip ("Resets every instrument to default settings and clears all steps.");
    save.setTooltip ("Saves the current sounds, patterns, FX and mixer as a user preset.");
    more.setTooltip ("Opens preset import, export, delete and help options.");

    prev.onClick = [this] { loadPreset ((proc.presetIndex + proc.presets.entries().size() - 1) % juce::jmax (1, proc.presets.entries().size())); };
    next.onClick = [this] { loadPreset ((proc.presetIndex + 1) % juce::jmax (1, proc.presets.entries().size())); };
    init.onClick = [this]
    {
        proc.presets.init();
        proc.presetName = "Init";
        proc.presetIndex = -1;
        notify ("All instruments reset, all steps cleared");
    };
    save.onClick = [this] { savePreset(); };
    more.onClick = [this] { showOptionsMenu(); };

    for (int i = 0; i < 4; ++i)
    {
        tabs[(size_t) i].onClick = [this, i] { showTab (i); };
        addAndMakeVisible (tabs[(size_t) i]);
    }
    for (auto* c : std::initializer_list<juce::Component*> { &prev, &next, &init, &save, &more, &synth, &overview, &fx, &mix,
                                                              &bar, &exporter, &lane })
        addAndMakeVisible (c);

    lane.onSelectStep = [this] { exporter.voiceOrStepChanged(); };
    showTab (0);
    selectVoice (proc.selectedVoice);
}

void Rack::paint (juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    g.setColour (colours::rack);
    g.fillRoundedRectangle (area, 10.0f);

    // Brushed metal rails with screws.
    for (float x : { 0.0f, area.getRight() - 34.0f })
    {
        const auto rail = juce::Rectangle<float> (x, 0, 34, area.getHeight());
        drawBrushedMetal (g, rail.reduced (2, 2), 6.0f);
        for (float y : { 44.0f, area.getCentreY(), area.getBottom() - 44.0f })
            drawScrew (g, { rail.getCentreX(), y }, 8.0f);
    }

    // Brand.
    g.setFont (font (50.0f).withExtraKerningFactor (0.04f));
    g.setColour (colours::text);
    g.drawText ("PERCUSYN", juce::Rectangle<float> (60, 16, 420, 56), juce::Justification::centredLeft);
    drawLabel (g, "PERCUSSION SYNTHESIZER", { 62, 70, 420, 20 }, 14.0f, colours::muted, juce::Justification::centredLeft);

    // Preset display.
    const auto box = juce::Rectangle<float> (636, 18, 328, 74);
    g.setColour (colours::display);
    g.fillRoundedRectangle (box, 5.0f);
    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawRoundedRectangle (box.reduced (0.5f), 5.0f, 1.0f);
    drawLabel (g, "PRESET", box.withHeight (24).translated (0, 2), 11.5f, colours::muted);
    drawLabel (g, proc.presetName, box.withTrimmedTop (22).withHeight (30), 21.0f, colours::text);
    const int count = juce::jmax (1, proc.presets.entries().size());
    for (int i = 0; i < 5; ++i)
    {
        const bool on = proc.presetIndex >= 0 && proc.presetIndex % 5 == i && count > 0;
        g.setColour (on ? colours::blue : juce::Colour (0xff4a4a4a));
        g.fillEllipse (box.getCentreX() - 26 + (float) i * 12, box.getBottom() - 14, 5, 5);
    }

    g.setColour (colours::line);
    g.drawHorizontalLine (106, 40, area.getRight() - 40);

    if (noticeTicks > 0 && notice.isNotEmpty())
    {
        const auto n = juce::Rectangle<float> (area.getCentreX() - 330, 690, 660, 40);
        g.setColour (juce::Colour (0xf0202020));
        g.fillRoundedRectangle (n, 6.0f);
        g.setColour (colours::orange);
        g.drawRoundedRectangle (n, 6.0f, 1.2f);
        drawLabel (g, notice, n, 15.0f, colours::text, juce::Justification::centred, false);
    }
}

void Rack::resized()
{
    prev.setBounds (644, 36, 40, 40);
    next.setBounds (916, 36, 40, 40);
    init.setBounds (984, 26, 66, 58);
    save.setBounds (1054, 26, 66, 58);
    more.setBounds (1124, 26, 58, 58);
    for (int i = 0; i < 4; ++i)
        tabs[(size_t) i].setBounds (1200 + i * 74, 26, 70, 58);

    const juce::Rectangle<int> main (40, 112, 1456, 620);
    for (auto* page : std::initializer_list<juce::Component*> { &synth, &overview, &fx, &mix })
        page->setBounds (main);
    bar.setBounds (40, 742, 1456, 90);
    exporter.setBounds (40, 842, 700, 260);
    lane.setBounds (752, 842, 744, 260);
}

void Rack::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && presetNameArea().contains (e.getPosition()))
        showPresetMenu();
}

void Rack::showTab (int t)
{
    tab = t;
    synth.setVisible (t == 0);
    overview.setVisible (t == 1);
    fx.setVisible (t == 2);
    mix.setVisible (t == 3);
    for (int i = 0; i < 4; ++i)
        tabs[(size_t) i].setLit (i == t);
}

void Rack::selectVoice (int v)
{
    proc.selectedVoice = juce::jlimit (0, numVoices - 1, v);
    synth.setVoice (proc.selectedVoice);
    exporter.voiceOrStepChanged();
    lane.repaint();
    overview.repaint();
}

void Rack::notify (const juce::String& message)
{
    notice = message;
    noticeTicks = 120;
    repaint();
}

void Rack::refresh()
{
    const int step = proc.isSequencerRunning() ? proc.currentStep() : -1;
    bar.refresh (proc.selectedVoice, tab == 1);
    lane.refresh (step);
    if (tab == 1)
        overview.refresh (step);

    // A changed sound or render velocity invalidates the rendered one-shot.
    const auto sound = proc.voiceSnapshot (proc.selectedVoice);
    const int velocity = proc.pattern.get (proc.selectedVoice, proc.selectedStep).velocity;
    if (sound.v != lastSound.v || velocity != lastVelocity)
    {
        lastSound = sound;
        lastVelocity = velocity;
        exporter.soundChanged();
    }

    if (noticeTicks > 0 && --noticeTicks == 0)
        repaint();
}

void Rack::loadPreset (int index)
{
    juce::String error;
    if (! proc.presets.load (index, error))
    {
        notify (error);
        return;
    }
    proc.presetIndex = index;
    proc.presetName = proc.presets.entries()[index].name;
    selectVoice (proc.selectedVoice);
    repaint();
}

void Rack::showPresetMenu()
{
    juce::PopupMenu menu;
    const auto& list = proc.presets.entries();
    menu.addSectionHeader ("FACTORY");
    for (int i = 0; i < list.size(); ++i)
        if (list[i].factory)
            menu.addItem (i + 1, list[i].name, true, i == proc.presetIndex);
    bool header = false;
    for (int i = 0; i < list.size(); ++i)
        if (! list[i].factory)
        {
            if (! header)
                menu.addSectionHeader ("USER");
            header = true;
            menu.addItem (i + 1, list[i].name, true, i == proc.presetIndex);
        }
    juce::Component::SafePointer<Rack> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withTargetScreenArea (
                            localAreaToGlobal (presetNameArea())),
                        [safe] (int result)
                        {
                            if (safe != nullptr && result > 0)
                                safe->loadPreset (result - 1);
                        });
}

void Rack::savePreset()
{
    juce::Component::SafePointer<Rack> safe (this);
    auto* window = new juce::AlertWindow ("Save preset", "Name for the user preset:", juce::MessageBoxIconType::NoIcon, this);
    window->addTextEditor ("name", proc.presetIndex >= 0 && proc.presets.entries()[proc.presetIndex].factory
                                       ? proc.presetName + " copy" : proc.presetName);
    window->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    window->enterModalState (true, juce::ModalCallbackFunction::create ([safe, window] (int result)
    {
        if (safe == nullptr || result != 1)
            return;
        const auto name = window->getTextEditorContents ("name").trim().substring (0, 64);
        juce::String error;
        if (! safe->proc.presets.saveUser (name, error))
        {
            safe->notify (error);
            return;
        }
        safe->proc.presetName = name.isEmpty() ? juce::String ("Untitled") : name;
        safe->proc.presetIndex = safe->proc.presets.indexOf (safe->proc.presetName);
        safe->notify ("Preset saved: " + safe->proc.presetName);
    }), true);
}

void Rack::showOptionsMenu()
{
    juce::PopupMenu menu;
    const bool isUser = juce::isPositiveAndBelow (proc.presetIndex, proc.presets.entries().size())
                        && ! proc.presets.entries()[proc.presetIndex].factory;
    menu.addItem (1, "Export preset JSON...");
    menu.addItem (2, "Import preset JSON...");
    menu.addItem (3, "Delete user preset", isUser);
    menu.addSeparator();
    menu.addItem (4, "Show preset folder");
    menu.addItem (5, "Show render folder");
    menu.addSeparator();
    menu.addItem (6, "Help");

    juce::Component::SafePointer<Rack> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&more), [safe] (int result)
    {
        if (safe == nullptr)
            return;
        auto& self = *safe;
        if (result == 1)
        {
            self.chooser = std::make_unique<juce::FileChooser> (
                "Export preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                                     .getChildFile (juce::File::createLegalFileName (self.proc.presetName) + ".json"),
                "*.json");
            self.chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
                                       [safe] (const juce::FileChooser& fc)
                                       {
                                           if (safe == nullptr || fc.getResult() == juce::File())
                                               return;
                                           juce::String error;
                                           if (safe->proc.presets.exportTo (fc.getResult().withFileExtension ("json"),
                                                                            safe->proc.presetName, error))
                                               safe->notify ("Preset exported");
                                           else
                                               safe->notify (error);
                                       });
        }
        else if (result == 2)
        {
            self.chooser = std::make_unique<juce::FileChooser> (
                "Import preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.json");
            self.chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                       [safe] (const juce::FileChooser& fc)
                                       {
                                           if (safe == nullptr || fc.getResult() == juce::File())
                                               return;
                                           juce::String name, error;
                                           if (safe->proc.presets.importFrom (fc.getResult(), name, error))
                                           {
                                               safe->proc.presetName = name;
                                               safe->proc.presetIndex = -1;
                                               safe->selectVoice (safe->proc.selectedVoice);
                                               safe->notify ("Imported " + name + " - use SAVE to keep it as a user preset");
                                           }
                                           else
                                               safe->notify (error);
                                       });
        }
        else if (result == 3)
        {
            juce::String error;
            if (self.proc.presets.deleteUser (self.proc.presetIndex, error))
            {
                self.notify ("User preset deleted");
                self.proc.presetIndex = -1;
            }
            else
                self.notify (error);
        }
        else if (result == 4)
        {
            Presets::userFolder().createDirectory();
            Presets::userFolder().startAsProcess();
        }
        else if (result == 5)
        {
            renderFolder().createDirectory();
            renderFolder().startAsProcess();
        }
        else if (result == 6)
            self.showHelp();
    });
}

void Rack::showHelp()
{
    const juce::String text =
        "Knobs: drag up/down (Shift for fine), scroll, double-click to reset. Click a value display to type an exact value.\n\n"
        "Instruments: click to select and play. MIDI notes from C1 (36) upward play Closed Hat, Open Hat, Snare, Tom, "
        "Shaker, Clap, Perc, Rim and Click on any channel. Closed Hat chokes Open Hat.\n\n"
        "Sequencer: left-click or drag over steps to draw them, right-click or drag to erase them in one stroke; VEL/PROB drag "
        "or scroll; COND cycles ALL, 1:2, 2:2, 1:4. M/S under each instrument button mute or solo it. "
        "HOST off: the internal sequencer runs only while the host is stopped. HOST lit: it follows the host transport.\n\n"
        "One-shot: RENDER uses the velocity of the selected step. PREVIEW plays it, WAV saves it, DRAG TO DAW drops the "
        "file into your arrangement. Renders are kept in Documents/PERCUSYN/Renders.\n\n"
        "Outputs: Main stereo plus one stereo output per instrument (enable them in the host).";
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::NoIcon)
                                      .withTitle ("PERCUSYN Help")
                                      .withMessage (text)
                                      .withButton ("Close")
                                      .withAssociatedComponent (this),
                                  nullptr);
}

// =============================================================================================
PercusynEditor::PercusynEditor (PercusynProcessor& p) : AudioProcessorEditor (p), rack (p)
{
    setLookAndFeel (&look);
    addAndMakeVisible (rack);
    rack.setBounds (0, 0, rackWidth, rackHeight);

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) rackWidth / rackHeight);
    setResizeLimits (rackWidth / 2, rackHeight / 2, rackWidth * 2, rackHeight * 2);
    setSize (rackWidth * 3 / 4, rackHeight * 3 / 4);
    startTimerHz (30);
}

PercusynEditor::~PercusynEditor()
{
    setLookAndFeel (nullptr);
}

void PercusynEditor::paint (juce::Graphics& g)
{
    g.fillAll (colours::window);
}

void PercusynEditor::resized()
{
    const float scale = juce::jmin ((float) getWidth() / rackWidth, (float) getHeight() / rackHeight);
    const float x = ((float) getWidth() - rackWidth * scale) * 0.5f;
    const float y = ((float) getHeight() - rackHeight * scale) * 0.5f;
    rack.setTransform (juce::AffineTransform::scale (scale).translated (x, y));
}

void PercusynEditor::timerCallback()
{
    rack.refresh();
}
