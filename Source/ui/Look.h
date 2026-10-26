#pragma once

// Visual language of the approved mockup: neutral graphite/black, brushed metal, orange/blue
// LED rings and orange seven-segment values (docs/02_UI_DESIGN.md).

#include <juce_gui_basics/juce_gui_basics.h>

namespace percusyn::ui
{
namespace colours
{
const juce::Colour window { 0xff0d0d0d };
const juce::Colour rack { 0xff1b1b1b };
const juce::Colour panel { 0xff161616 };
const juce::Colour line { 0xff3a3a3a };
const juce::Colour lineSoft { 0xff2a2a2a };
const juce::Colour text { 0xffe4e4e4 };
const juce::Colour muted { 0xffababab };
const juce::Colour dim { 0xff6c6c6c };
const juce::Colour orange { 0xffff9d25 };
const juce::Colour blue { 0xff78caff };
const juce::Colour button { 0xff2a2a2a };
const juce::Colour buttonHi { 0xff353535 };
const juce::Colour display { 0xff0b0b0b };
} // namespace colours

// Logical design size; the editor scales this whole rack with the window.
constexpr int rackWidth = 1536;
constexpr int rackHeight = 1120;

juce::Font font (float size, bool bold = true);

// Text with letter spacing, as in the mockup's small caps labels.
void drawLabel (juce::Graphics&, const juce::String&, juce::Rectangle<float>, float size, juce::Colour,
                juce::Justification = juce::Justification::centred, bool bold = true);

void drawModule (juce::Graphics&, juce::Rectangle<float> area, const juce::String& title, bool bypassed = false);
void drawMetalKnob (juce::Graphics&, juce::Rectangle<float> area, float angleRadians, bool bypassed);
void drawLedRing (juce::Graphics&, juce::Rectangle<float> area, float normalised, juce::Colour, bool bypassed);
void drawButtonFace (juce::Graphics&, juce::Rectangle<float> area, bool active, bool hover, juce::Colour accent);
void drawBrushedMetal (juce::Graphics&, juce::Rectangle<float> area, float corner);
void drawScrew (juce::Graphics&, juce::Point<float> centre, float radius);

// Seven-segment readout: digits, '-', '.' in the display colour; unit text after the digits.
float sevenSegmentWidth (const juce::String& digits, float digitHeight);
void drawSevenSegment (juce::Graphics&, const juce::String& digits, juce::Point<float> topLeft, float digitHeight,
                       juce::Colour);
void drawReadout (juce::Graphics&, juce::Rectangle<float> area, const juce::String& number, const juce::String& unit,
                  float digitHeight, juce::Colour = colours::orange);

// Icon paths from the mockup's SVG (instrument and waveform icons).
juce::Path instrumentIcon (int voice);
juce::Path waveIcon (int wave);
} // namespace percusyn::ui
