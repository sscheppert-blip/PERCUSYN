#pragma once

// Parameter-bound controls drawn in the hardware style. Every control can be rebound to another
// voice's parameter, so one page serves all nine voices.

#include "Params.h"
#include "ui/Look.h"

namespace percusyn::ui
{
// English help text (mockup tooltips.js), keyed by parameter key without the voice prefix.
juce::String helpText (const juce::String& key);

class PercusynLook : public juce::LookAndFeel_V4
{
public:
    PercusynLook();
    void drawComboBox (juce::Graphics&, int width, int height, bool down, int bx, int by, int bw, int bh,
                       juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
};

// Base for controls bound to one host parameter.
class ParamControl : public juce::Component, public juce::SettableTooltipClient
{
public:
    void bind (juce::AudioProcessorValueTreeState& state, const juce::String& id, const ParamDef& def,
               const juce::String& tooltip);
    float value() const;
    float normalised() const;
    const ParamDef* definition() const noexcept { return def; }

protected:
    void beginDrag();
    void dragTo (float normalisedValue);
    void endDrag();
    void setValue (float plainValue);
    void resetToDefault();
    void enterValue();

    juce::RangedAudioParameter* param = nullptr;
    const ParamDef* def = nullptr;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};

class Knob : public ParamControl
{
public:
    explicit Knob (bool compact = false) : compact (compact) {}
    void setLabel (const juce::String& text) { label = text; repaint(); }
    void setBypassed (bool b) { if (b != bypassed) { bypassed = b; repaint(); } }
    void setShowLimits (bool b) { showLimits = b; }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    juce::Rectangle<float> knobArea() const;
    juce::Rectangle<float> readoutArea() const;
    bool compact, bypassed = false, showLimits = true, dragging = false;
    juce::String label;
    float dragStart = 0;
    juce::Point<float> mouseStart;
};

class Fader : public ParamControl
{
public:
    void setLabel (const juce::String& text) { label = text; }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    juce::Rectangle<float> trackArea() const;
    juce::String label;
    bool onReadout = false;
};

// ON/OFF power buttons and M/S mixer buttons.
class Toggle : public ParamControl
{
public:
    enum class Style { Power, Mute, Solo, Plain };
    explicit Toggle (Style s, juce::String text = {}) : style (s), text (std::move (text)) {}
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }

private:
    Style style;
    juce::String text;
};

// Oscillator wave buttons with icons.
class WaveButtons : public ParamControl
{
public:
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
};

// LP / HP / BP / BR radio list.
class FilterTypes : public ParamControl
{
public:
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
};

// Resonator model selector.
class ChoiceBox : public juce::ComboBox
{
public:
    void bind (juce::AudioProcessorValueTreeState& state, const juce::String& id, const ParamDef& def,
               const juce::String& tooltip);

private:
    std::unique_ptr<juce::ComboBoxParameterAttachment> attachment;
};

// Plain panel button: text or icon, optional lit state.
class PanelButton : public juce::Button
{
public:
    explicit PanelButton (const juce::String& name, juce::Colour accent = colours::orange);
    void setIcon (juce::Path p, bool filled) { icon = std::move (p); iconFilled = filled; repaint(); }
    void setSubText (const juce::String& s) { sub = s; }
    void setLit (bool b) { if (b != lit) { lit = b; repaint(); } }
    void setTextSize (float s) { textSize = s; }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;

private:
    juce::Colour accent;
    juce::Path icon;
    bool iconFilled = false, lit = false;
    juce::String sub;
    float textSize = 15.0f;
};

// Small numeric field with a seven-segment display (LENGTH, SWING, BPM): drag or type.
class NumberField : public ParamControl
{
public:
    explicit NumberField (juce::String title) : title (std::move (title)) {}
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override { enterValue(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    juce::String title;
    float dragStart = 0;
    bool moved = false;
};

void showValueEntry (juce::Component& owner, const juce::String& title, const juce::String& prompt, float current,
                     std::function<void (float)> apply);
} // namespace percusyn::ui
