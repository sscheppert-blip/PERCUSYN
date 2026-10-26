#include "ui/Widgets.h"

#include <map>

namespace percusyn::ui
{
juce::String helpText (const juce::String& key)
{
    static const std::map<juce::String, juce::String> texts {
        { "comb_enabled", "Enables or bypasses the comb stage for this instrument. Settings are retained when bypassed." },
        { "comb_tune", "Sets the comb resonance spacing in Hertz. Lower tuning gives a larger, hollow body; higher tuning gives tighter metallic color." },
        { "comb_feedback", "Feeds part of the comb output back into its input. Higher settings produce stronger, longer ringing." },
        { "comb_damping", "Controls high-frequency loss in the comb feedback loop. Higher settings give darker ringing." },
        { "comb_mix", "Blends the original sound with the comb-filtered sound. Zero keeps the original signal." },
        { "resonator_enabled", "Enables or bypasses the resonator for this instrument. Settings are retained when bypassed." },
        { "resonator_model", "Selects a resonance character: METAL for an inharmonic metallic body, PLATE for sheet-like ringing, or TUBE for a hollow pipe-like body." },
        { "resonator_tune", "Sets the resonator's reference pitch in Hertz. The selected model determines how its resonances are spaced." },
        { "resonator_decay", "Sets how long the resonating body rings after a hit (time to -60 dB of the lowest mode)." },
        { "resonator_damping", "Controls how quickly bright resonances fade. Higher settings give a softer, darker tail." },
        { "resonator_mix", "Blends the original sound with the resonating body. Zero keeps the original signal." },
        { "osc_tune", "Transposes this instrument in semitones. 12 semitones equals one octave." },
        { "osc_decay", "Sets how quickly the oscillator component fades after a hit. The noise layer follows the Amp envelope only." },
        { "osc_tone", "Shapes the oscillator timbre: wavefolding for SINE/TRI, brightness for SAW, metallic cluster amount for NOISE, modulation index for FM." },
        { "osc_wave", "Selects the oscillator source: sine, triangle, saw, noise or FM." },
        { "pitch_amount", "Sets the depth of the pitch envelope, up to four octaves above Tune at full amount." },
        { "pitch_attack", "Sets how quickly the pitch envelope reaches its maximum." },
        { "pitch_decay", "Sets how quickly the pitch envelope returns to the base tuning." },
        { "amp_attack", "Sets how quickly a hit reaches full level. Short settings give a sharper transient." },
        { "amp_decay", "Sets how quickly the hit falls from its peak to the sustain level, and the release time after the gate closes." },
        { "amp_sustain", "Sets the level held while the note (or sequencer step) is held. At 0% every hit is a pure one-shot." },
        { "filter_cutoff", "Sets the filter frequency. Its effect depends on the selected filter type." },
        { "filter_resonance", "Emphasizes the filter frequency. Higher settings give a more ringing character." },
        { "filter_type", "Selects low-pass, high-pass, band-pass or band-reject filtering." },
        { "noise_color", "Shapes the noise spectrum, from darker to brighter." },
        { "noise_level", "Sets the level of the noise component in decibels." },
        { "drive_amount", "Sets the amount of saturation for extra harmonics and edge." },
        { "pitch_bend", "Sets how far the MIDI pitch wheel bends this instrument, in semitones. Negative values invert the wheel; 0 ignores it." },
        { "velocity_sensitivity", "Sets how strongly the instrument responds to velocity (step or MIDI note)." },
        { "level", "Sets this instrument's output level in decibels." },
        { "send_fx", "Sets how much of this instrument is sent to the effects bus." },
        { "pan", "Places this instrument between the left and right channels." },
        { "mute", "Mutes this instrument." },
        { "solo", "Plays only soloed instruments. Multiple instruments can be soloed together." },
        { "tempo", "Sets the internal sequencer tempo in beats per minute. When following the host, the host tempo is used." },
        { "swing", "Offsets alternating sixteenth notes to create a swung groove. Zero gives straight timing." },
        { "length", "Sets the number of steps before the pattern repeats. Steps beyond this length are retained." },
        { "fx_delay_time", "Sets the time between delay repeats in milliseconds." },
        { "fx_delay_feedback", "Sets how much delay output feeds back into the delay, controlling repeat length." },
        { "fx_delay_mix", "Sets the level of the delay return. Each instrument's Send FX feeds the delay." },
        { "fx_reverb_size", "Sets the apparent size of the reverberant space." },
        { "fx_reverb_decay", "Sets how long the reverb tail lasts." },
        { "fx_reverb_mix", "Sets the level of the reverb return. Each instrument's Send FX feeds the reverb." },
        { "master_level", "Sets the overall level of the main output after the mixer." },
        { "host_sync", "HOST lit: the internal sequencer follows the host transport. HOST off: while the host plays, the internal sequencer is off and MIDI notes from C1 upward play the instruments." },
    };
    const auto it = texts.find (key);
    return it != texts.end() ? it->second : juce::String();
}

// ---------------------------------------------------------------------------------------------
PercusynLook::PercusynLook()
{
    setColour (juce::ComboBox::backgroundColourId, colours::display);
    setColour (juce::ComboBox::textColourId, colours::text);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff3c3c3c));
    setColour (juce::ComboBox::arrowColourId, colours::muted);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1e1e1e));
    setColour (juce::PopupMenu::textColourId, colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2a14));
    setColour (juce::PopupMenu::highlightedTextColourId, colours::orange);
    setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff1e1e1e));
    setColour (juce::AlertWindow::textColourId, colours::text);
    setColour (juce::AlertWindow::outlineColourId, juce::Colour (0xff3c3c3c));
    setColour (juce::TextEditor::backgroundColourId, colours::display);
    setColour (juce::TextEditor::textColourId, colours::orange);
    setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff3c3c3c));
    setColour (juce::TextEditor::focusedOutlineColourId, colours::orange);
    setColour (juce::TextButton::buttonColourId, colours::button);
    setColour (juce::TextButton::textColourOffId, colours::text);
    setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xf0202020));
    setColour (juce::TooltipWindow::textColourId, colours::text);
    setColour (juce::TooltipWindow::outlineColourId, juce::Colour (0xff4a4a4a));
}

void PercusynLook::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    const auto area = juce::Rectangle<float> (0, 0, (float) width, (float) height);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (area, 3.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (area.reduced (0.5f), 3.0f, 1.0f);
    juce::Path arrow;
    const float cx = (float) width - 12.0f, cy = (float) height * 0.5f;
    arrow.addTriangle (cx - 4, cy - 2, cx + 4, cy - 2, cx, cy + 3);
    g.setColour (colours::muted);
    g.fillPath (arrow);
}

juce::Font PercusynLook::getComboBoxFont (juce::ComboBox& box)
{
    return font (juce::jmin (16.0f, (float) box.getHeight() * 0.55f));
}

juce::Font PercusynLook::getPopupMenuFont()
{
    return font (16.0f, false);
}

void PercusynLook::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (4, 0, box.getWidth() - 20, box.getHeight());
    label.setFont (getComboBoxFont (box));
}

// ---------------------------------------------------------------------------------------------
void showValueEntry (juce::Component& owner, const juce::String& title, const juce::String& prompt, float current,
                     std::function<void (float)> apply)
{
    auto* window = new juce::AlertWindow (title, prompt, juce::MessageBoxIconType::NoIcon, &owner);
    window->addTextEditor ("value", juce::String (current), {});
    window->addButton ("Apply", 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    if (auto* editor = window->getTextEditor ("value"))
        editor->selectAll();
    juce::Component::SafePointer<juce::Component> safe (&owner);
    window->enterModalState (true, juce::ModalCallbackFunction::create ([window, safe, apply] (int result)
    {
        if (result == 1 && safe != nullptr)
        {
            const auto text = window->getTextEditorContents ("value").trim();
            if (text.containsAnyOf ("0123456789"))
                apply (text.getFloatValue());
        }
    }), true);
}

void ParamControl::bind (juce::AudioProcessorValueTreeState& state, const juce::String& id, const ParamDef& d,
                         const juce::String& tooltip)
{
    attachment.reset();
    def = &d;
    param = state.getParameter (id);
    jassert (param != nullptr);
    attachment = std::make_unique<juce::ParameterAttachment> (*param, [this] (float) { repaint(); }, state.undoManager);
    attachment->sendInitialUpdate();
    setTooltip (tooltip);
    repaint();
}

float ParamControl::value() const
{
    return param != nullptr ? param->convertFrom0to1 (param->getValue()) : 0.0f;
}

float ParamControl::normalised() const
{
    return param != nullptr ? param->getValue() : 0.0f;
}

void ParamControl::beginDrag()
{
    if (attachment != nullptr)
        attachment->beginGesture();
}

void ParamControl::dragTo (float n)
{
    if (attachment != nullptr && param != nullptr)
        attachment->setValueAsPartOfGesture (param->convertFrom0to1 (juce::jlimit (0.0f, 1.0f, n)));
}

void ParamControl::endDrag()
{
    if (attachment != nullptr)
        attachment->endGesture();
}

void ParamControl::setValue (float plain)
{
    if (attachment != nullptr && def != nullptr)
        attachment->setValueAsCompleteGesture (juce::jlimit (def->min, def->max, plain));
}

void ParamControl::resetToDefault()
{
    if (param != nullptr)
        setValue (param->convertFrom0to1 (param->getDefaultValue()));
}

void ParamControl::enterValue()
{
    if (def == nullptr || param == nullptr)
        return;
    const auto prompt = "Value (" + juce::String (*def->unit != 0 ? def->unit : "units") + ") from "
                        + juce::String (def->min) + " to " + juce::String (def->max);
    juce::Component::SafePointer<ParamControl> safe (this);
    showValueEntry (*this, param->getName (64), prompt, value(), [safe] (float v)
    {
        if (safe != nullptr)
            safe->setValue (v);
    });
}

// ---------------------------------------------------------------------------------------------
juce::Rectangle<float> Knob::knobArea() const
{
    auto r = getLocalBounds().toFloat();
    const float labelH = compact ? 20.0f : 24.0f;
    const float bottom = (compact ? 34.0f : 42.0f) + (showLimits && ! compact ? 16.0f : 0.0f);
    r.removeFromTop (labelH);
    r.removeFromBottom (bottom);
    const float s = juce::jmin (r.getWidth(), r.getHeight());
    return r.withSizeKeepingCentre (s, s);
}

juce::Rectangle<float> Knob::readoutArea() const
{
    auto r = getLocalBounds().toFloat();
    const float h = compact ? 30.0f : 36.0f;
    const float w = juce::jmin (r.getWidth() - 8.0f, compact ? 96.0f : 108.0f);
    return { r.getCentreX() - w * 0.5f, r.getBottom() - h - 2.0f, w, h };
}

void Knob::paint (juce::Graphics& g)
{
    if (def == nullptr)
        return;
    const auto area = getLocalBounds().toFloat();
    drawLabel (g, label.isNotEmpty() ? label : juce::String (def->name).toUpperCase(),
               area.withHeight (compact ? 20.0f : 24.0f), compact ? 14.0f : 15.5f,
               bypassed ? colours::dim : colours::text);

    const auto ka = knobArea();
    const float n = normalised();
    drawLedRing (g, ka, n, isDurationParam (*def) ? colours::blue : colours::orange, bypassed);
    drawMetalKnob (g, ka.reduced (ka.getWidth() * 0.15f), juce::degreesToRadians (-135.0f + n * 270.0f), bypassed);

    if (showLimits && ! compact)
    {
        const auto lo = formatText (*def, def->min), hi = formatText (*def, def->max);
        const auto row = juce::Rectangle<float> (ka.getX() - 18, ka.getBottom() - 2, ka.getWidth() + 36, 16);
        drawLabel (g, lo, row, 11.5f, colours::muted, juce::Justification::centredLeft, false);
        drawLabel (g, hi, row, 11.5f, colours::muted, juce::Justification::centredRight, false);
    }

    const auto d = formatValue (*def, value());
    drawReadout (g, readoutArea(), d.number, d.unit, compact ? 17.0f : 21.0f);
}

void Knob::mouseDown (const juce::MouseEvent& e)
{
    dragging = ! readoutArea().contains (e.position);
    if (dragging)
    {
        dragStart = normalised();
        mouseStart = e.position;
        beginDrag();
    }
}

void Knob::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging)
        return;
    const float sensitivity = e.mods.isShiftDown() ? 1600.0f : 260.0f;
    const auto delta = (mouseStart.y - e.position.y) + (e.position.x - mouseStart.x);
    dragTo (dragStart + delta / sensitivity);
}

void Knob::mouseUp (const juce::MouseEvent& e)
{
    if (dragging)
        endDrag();
    else if (readoutArea().contains (e.position) && e.mouseWasClicked())
        enterValue();
    dragging = false;
}

void Knob::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (! readoutArea().contains (e.position))
        resetToDefault();
}

void Knob::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    if (param == nullptr)
        return;
    const float step = (e.mods.isShiftDown() ? 0.002f : 0.02f) * (w.deltaY > 0 ? 1.0f : -1.0f) * (w.isReversed ? -1.0f : 1.0f);
    beginDrag();
    dragTo (normalised() + step);
    endDrag();
}

// ---------------------------------------------------------------------------------------------
juce::Rectangle<float> Fader::trackArea() const
{
    auto r = getLocalBounds().toFloat();
    r.removeFromBottom (68);
    r.removeFromTop (10);
    return r.withSizeKeepingCentre (44, r.getHeight()).translated (10, 0);
}

void Fader::paint (juce::Graphics& g)
{
    if (def == nullptr)
        return;
    const auto track = trackArea();
    const float n = normalised();

    // Scale ticks at their real positions.
    const float marks[] { 12, 0, -12, -24, -36, -60 };
    for (auto m : marks)
    {
        if (m < def->min || m > def->max)
            continue;
        const float y = track.getBottom() - (m - def->min) / (def->max - def->min) * track.getHeight();
        drawLabel (g, (m > 0 ? "+" : "") + juce::String ((int) m), { track.getX() - 44, y - 7, 34, 14 }, 11.0f,
                   colours::muted, juce::Justification::centredRight, false);
        g.setColour (juce::Colour (0xff3a3a3a));
        g.drawHorizontalLine ((int) y, track.getX() - 8, track.getX() - 2);
    }

    const auto slot = track.withSizeKeepingCentre (6, track.getHeight());
    g.setColour (juce::Colour (0xff050505));
    g.fillRoundedRectangle (slot, 3.0f);
    const auto filled = slot.withTop (track.getBottom() - n * track.getHeight());
    g.setColour (colours::orange.withAlpha (0.65f));
    g.fillRoundedRectangle (filled, 3.0f);

    const float y = track.getBottom() - n * track.getHeight();
    auto thumb = juce::Rectangle<float> (track.getCentreX() - 20, y - 13, 40, 26);
    drawBrushedMetal (g, thumb, 3.0f);
    g.setColour (juce::Colour (0xff222222));
    g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    g.drawHorizontalLine ((int) y, thumb.getX() + 4, thumb.getRight() - 4);

    const auto area = getLocalBounds().toFloat();
    drawLabel (g, label, { area.getX(), area.getBottom() - 66, area.getWidth(), 20 }, 14.0f, colours::text);
    const auto d = formatValue (*def, value());
    drawReadout (g, { area.getCentreX() - 46, area.getBottom() - 42, 92, 32 }, d.number, d.unit, 18.0f);
}

void Fader::mouseDown (const juce::MouseEvent& e)
{
    onReadout = e.position.y > (float) getHeight() - 44.0f;
    if (! onReadout)
    {
        beginDrag();
        mouseDrag (e);
    }
}

void Fader::mouseDrag (const juce::MouseEvent& e)
{
    if (onReadout)
        return;
    const auto track = trackArea();
    dragTo ((track.getBottom() - e.position.y) / track.getHeight());
}

void Fader::mouseUp (const juce::MouseEvent& e)
{
    if (onReadout)
    {
        if (e.mouseWasClicked())
            enterValue();
    }
    else
        endDrag();
}

void Fader::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.position.y <= (float) getHeight() - 44.0f)
        resetToDefault();
}

void Fader::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    beginDrag();
    dragTo (normalised() + (w.deltaY > 0 ? 0.01f : -0.01f));
    endDrag();
}

// ---------------------------------------------------------------------------------------------
void Toggle::paint (juce::Graphics& g)
{
    const bool on = value() > 0.5f;
    const auto area = getLocalBounds().toFloat().reduced (1);
    const auto accent = style == Style::Solo ? colours::blue : colours::orange;
    drawButtonFace (g, area, on, isMouseOver(), accent);
    juce::String t = text;
    if (style == Style::Power)
        t = on ? "ON" : "OFF";
    drawLabel (g, t, area, juce::jmin (15.0f, area.getHeight() * 0.55f), on ? accent : colours::muted);
}

void Toggle::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && contains (e.getPosition()))
        setValue (value() > 0.5f ? 0.0f : 1.0f);
}

// ---------------------------------------------------------------------------------------------
void WaveButtons::paint (juce::Graphics& g)
{
    if (def == nullptr)
        return;
    const auto names = juce::StringArray::fromTokens (def->choices, "|", "");
    const int current = (int) std::lround (value());
    const float w = (float) getWidth() / (float) names.size();
    for (int i = 0; i < names.size(); ++i)
    {
        auto cell = juce::Rectangle<float> (w * (float) i, 0, w, (float) getHeight()).reduced (3, 0);
        const bool on = i == current;
        drawButtonFace (g, cell, on, false, colours::orange);
        auto icon = waveIcon (i);
        auto iconArea = cell.withTrimmedBottom (cell.getHeight() * 0.42f).reduced (cell.getWidth() * 0.32f, 4);
        icon.applyTransform (icon.getTransformToScaleToFit (iconArea, true));
        g.setColour (on ? colours::orange : colours::muted);
        g.strokePath (icon, juce::PathStrokeType (1.6f));
        drawLabel (g, names[i], cell.withTrimmedTop (cell.getHeight() * 0.58f).translated (0, -3), 13.0f,
                   on ? colours::orange : colours::text);
    }
}

void WaveButtons::mouseUp (const juce::MouseEvent& e)
{
    if (def == nullptr || ! e.mouseWasClicked())
        return;
    const int count = juce::StringArray::fromTokens (def->choices, "|", "").size();
    setValue ((float) juce::jlimit (0, count - 1, (int) (e.position.x / ((float) getWidth() / (float) count))));
}

// ---------------------------------------------------------------------------------------------
void FilterTypes::paint (juce::Graphics& g)
{
    if (def == nullptr)
        return;
    const auto area = getLocalBounds().toFloat();
    g.setColour (colours::display);
    g.fillRoundedRectangle (area, 4.0f);
    g.setColour (juce::Colour (0xff333333));
    g.drawRoundedRectangle (area.reduced (0.5f), 4.0f, 1.0f);
    drawLabel (g, "TYPE", area.withHeight (26), 13.5f, colours::text);

    const auto names = juce::StringArray::fromTokens (def->choices, "|", "");
    const int current = (int) std::lround (value());
    const float rowH = (area.getHeight() - 30) / (float) names.size();
    for (int i = 0; i < names.size(); ++i)
    {
        auto row = juce::Rectangle<float> (area.getX() + 12, 28 + rowH * (float) i, area.getWidth() - 16, rowH);
        const bool on = i == current;
        const auto dot = juce::Rectangle<float> (row.getX(), row.getCentreY() - 5, 10, 10);
        g.setColour (on ? colours::orange : juce::Colour (0xff3a3a3a));
        g.fillEllipse (dot);
        if (on)
        {
            g.setColour (colours::orange.withAlpha (0.3f));
            g.fillEllipse (dot.expanded (3));
        }
        drawLabel (g, names[i], row.withTrimmedLeft (18), 14.0f, on ? colours::text : colours::muted,
                   juce::Justification::centredLeft);
    }
}

void FilterTypes::mouseUp (const juce::MouseEvent& e)
{
    if (def == nullptr || ! e.mouseWasClicked())
        return;
    const int count = juce::StringArray::fromTokens (def->choices, "|", "").size();
    const float rowH = ((float) getHeight() - 30) / (float) count;
    setValue ((float) juce::jlimit (0, count - 1, (int) ((e.position.y - 28) / rowH)));
}

// ---------------------------------------------------------------------------------------------
void ChoiceBox::bind (juce::AudioProcessorValueTreeState& state, const juce::String& id, const ParamDef& d,
                      const juce::String& tooltip)
{
    attachment.reset();
    if (getNumItems() == 0)
        addItemList (juce::StringArray::fromTokens (d.choices, "|", ""), 1);
    if (auto* p = state.getParameter (id))
        attachment = std::make_unique<juce::ComboBoxParameterAttachment> (*p, *this, state.undoManager);
    setTooltip (tooltip);
}

// ---------------------------------------------------------------------------------------------
PanelButton::PanelButton (const juce::String& name, juce::Colour a) : juce::Button (name), accent (a) {}

void PanelButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto area = getLocalBounds().toFloat().reduced (1);
    drawButtonFace (g, area, lit || down, highlighted, accent);
    const auto colour = ! isEnabled() ? colours::dim : (lit ? accent : colours::text);

    if (! icon.isEmpty())
    {
        auto iconArea = sub.isNotEmpty() ? area.withTrimmedBottom (area.getHeight() * 0.38f) : area;
        iconArea = sub.isNotEmpty() ? iconArea.reduced (area.getWidth() * 0.3f, area.getHeight() * 0.14f)
                                    : iconArea.reduced (area.getWidth() * 0.34f, area.getHeight() * 0.3f);
        auto p = icon;
        p.applyTransform (p.getTransformToScaleToFit (iconArea, true));
        g.setColour (colour);
        if (iconFilled)
            g.fillPath (p);
        else
            g.strokePath (p, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        if (sub.isNotEmpty())
            drawLabel (g, sub, area.withTrimmedTop (area.getHeight() * 0.6f).translated (0, -4), 12.5f, colour);
        return;
    }
    drawLabel (g, getButtonText(), area, textSize, colour);
}

// ---------------------------------------------------------------------------------------------
void NumberField::paint (juce::Graphics& g)
{
    if (def == nullptr)
        return;
    auto area = getLocalBounds().toFloat();
    drawLabel (g, title, area.removeFromTop (20), 13.5f, colours::text);
    const auto d = formatValue (*def, value());
    drawReadout (g, area.reduced (2, 2), d.number, d.unit == "BPM" ? juce::String() : d.unit, 19.0f);
}

void NumberField::mouseDown (const juce::MouseEvent&)
{
    dragStart = normalised();
    moved = false;
    beginDrag();
}

void NumberField::mouseDrag (const juce::MouseEvent& e)
{
    moved = moved || e.getDistanceFromDragStart() > 2;
    dragTo (dragStart - (float) e.getDistanceFromDragStartY() / 300.0f);
}

void NumberField::mouseUp (const juce::MouseEvent&)
{
    endDrag();
}

void NumberField::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    if (def != nullptr)
        setValue (value() + (w.deltaY > 0 ? def->step : -def->step) * (def->step < 1 ? 10.0f : 1.0f));
}
} // namespace percusyn::ui
