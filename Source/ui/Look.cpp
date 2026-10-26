#include "ui/Look.h"

namespace percusyn::ui
{
juce::Font font (float size, bool bold)
{
    return juce::Font (juce::FontOptions (size, bold ? juce::Font::bold : juce::Font::plain));
}

void drawLabel (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, float size, juce::Colour colour,
                juce::Justification justification, bool bold)
{
    auto f = font (size, bold).withExtraKerningFactor (0.06f);
    g.setFont (f);
    g.setColour (colour);
    g.drawText (text, area, justification, false);
}

void drawModule (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& title, bool bypassed)
{
    g.setColour (colours::panel);
    g.fillRoundedRectangle (area, 6.0f);
    g.setColour (colours::lineSoft);
    g.drawRoundedRectangle (area.reduced (0.5f), 6.0f, 1.0f);
    if (title.isNotEmpty())
        drawLabel (g, title, area.reduced (16, 10).withHeight (24), 19.0f, bypassed ? colours::muted : colours::text,
                   juce::Justification::centredLeft);
}

void drawBrushedMetal (juce::Graphics& g, juce::Rectangle<float> area, float corner)
{
    juce::ColourGradient grad (juce::Colour (0xff9a9a9a), area.getX(), area.getY(), juce::Colour (0xff4e4e4e), area.getRight(),
                               area.getBottom(), false);
    grad.addColour (0.45, juce::Colour (0xffc8c8c8));
    grad.addColour (0.7, juce::Colour (0xff7b7b7b));
    g.setGradientFill (grad);
    g.fillRoundedRectangle (area, corner);

    juce::Graphics::ScopedSaveState save (g);
    juce::Path clip;
    clip.addRoundedRectangle (area, corner);
    g.reduceClipRegion (clip);
    juce::Random rng (7);
    for (float y = area.getY(); y < area.getBottom(); y += 1.5f)
    {
        g.setColour (juce::Colours::white.withAlpha (rng.nextFloat() * 0.07f));
        g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
    }
}

void drawScrew (juce::Graphics& g, juce::Point<float> c, float r)
{
    juce::ColourGradient grad (juce::Colour (0xffd8d8d8), c.x - r, c.y - r, juce::Colour (0xff404040), c.x + r, c.y + r, false);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
    g.setColour (juce::Colour (0xff202020));
    g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, 1.0f);
    g.drawLine (c.x - r * 0.6f, c.y - r * 0.6f, c.x + r * 0.6f, c.y + r * 0.6f, 1.6f);
}

void drawMetalKnob (juce::Graphics& g, juce::Rectangle<float> area, float angle, bool bypassed)
{
    const auto c = area.getCentre();
    const float r = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;

    // Dark collar with drop shadow.
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (c.x - r * 1.04f, c.y - r * 0.98f, r * 2.08f, r * 2.12f);
    juce::ColourGradient collar (juce::Colour (0xff3c3c3c), c.x, c.y - r, juce::Colour (0xff0e0e0e), c.x, c.y + r, false);
    g.setGradientFill (collar);
    g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);

    // Brushed metal cap: conic-like shading from several gradients.
    const float cr = r * 0.8f;
    juce::ColourGradient cap (juce::Colour (0xfff2f2f2), c.x - cr * 0.6f, c.y - cr * 0.8f, juce::Colour (0xff5d5d5d),
                              c.x + cr * 0.7f, c.y + cr * 0.9f, false);
    cap.addColour (0.35, juce::Colour (0xffbdbdbd));
    cap.addColour (0.55, juce::Colour (0xffe0e0e0));
    cap.addColour (0.8, juce::Colour (0xff8a8a8a));
    g.setGradientFill (cap);
    g.fillEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2);

    {
        juce::Graphics::ScopedSaveState save (g);
        juce::Path clip;
        clip.addEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2);
        g.reduceClipRegion (clip);
        juce::Random rng (11);
        for (int i = 0; i < 90; ++i)
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            g.setColour ((rng.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha (0.05f + rng.nextFloat() * 0.06f));
            g.drawLine (c.x, c.y, c.x + std::cos (a) * cr, c.y + std::sin (a) * cr, 0.8f);
        }
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.35f), c.x - cr * 0.3f, c.y - cr * 0.9f,
                                    juce::Colours::transparentWhite, c.x, c.y, true);
        g.setGradientFill (sheen);
        g.fillEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2);
    }
    g.setColour (juce::Colour (0xff2a2a2a));
    g.drawEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2, 1.2f);

    // Pointer.
    const float s = std::sin (angle), co = -std::cos (angle);
    g.setColour (bypassed ? juce::Colour (0xff3a3a3a) : juce::Colour (0xff111111));
    g.drawLine (c.x + s * cr * 0.25f, c.y + co * cr * 0.25f, c.x + s * cr * 0.92f, c.y + co * cr * 0.92f, juce::jmax (2.0f, r * 0.07f));
}

void drawLedRing (juce::Graphics& g, juce::Rectangle<float> area, float n, juce::Colour colour, bool bypassed)
{
    constexpr int segments = 29;
    const auto c = area.getCentre();
    const float size = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
    const float r1 = size * 0.80f, r2 = size * 0.97f;
    const int lit = (int) std::round (juce::jlimit (0.0f, 1.0f, n) * (segments - 1));
    for (int i = 0; i < segments; ++i)
    {
        const float a = juce::degreesToRadians (135.0f + (float) i * 270.0f / (segments - 1));
        const bool on = i <= lit;
        auto col = on ? colour : juce::Colour (0xff2b2b2b);
        if (on && bypassed)
            col = col.withMultipliedSaturation (0.55f).withMultipliedBrightness (0.75f);
        g.setColour (col);
        g.drawLine (c.x + r1 * std::cos (a), c.y + r1 * std::sin (a), c.x + r2 * std::cos (a), c.y + r2 * std::sin (a),
                    juce::jmax (2.0f, size * 0.07f));
    }
}

void drawButtonFace (juce::Graphics& g, juce::Rectangle<float> area, bool active, bool hover, juce::Colour accent)
{
    juce::ColourGradient grad (hover ? colours::buttonHi : colours::button, area.getX(), area.getY(),
                               juce::Colour (0xff1a1a1a), area.getX(), area.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (area, 4.0f);
    if (active)
    {
        g.setColour (accent.withAlpha (0.16f));
        g.fillRoundedRectangle (area, 4.0f);
        g.setColour (accent);
        g.drawRoundedRectangle (area.reduced (0.8f), 4.0f, 1.6f);
    }
    else
    {
        g.setColour (juce::Colour (0xff3c3c3c));
        g.drawRoundedRectangle (area.reduced (0.5f), 4.0f, 1.0f);
    }
}

// ---------------------------------------------------------------------------------------------
namespace
{
// Segment polygons in a 15 x 27 cell (mockup app.js).
const char* segmentPoints[7] {
    "3,1 10,1 12,3 10,5 3,5 1,3",          // a
    "11,4 13,3 13,11 11,13 9,11 9,6",      // b
    "11,14 13,12 13,20 11,22 9,20 9,16",   // c
    "3,21 10,21 12,23 10,25 3,25 1,23",    // d
    "1,14 3,16 3,20 1,22 0,20 0,12",       // e
    "1,4 3,6 3,11 1,13 0,11 0,3",          // f
    "3,11 9,11 11,13 9,15 3,15 1,13",      // g
};

const char* segmentsFor (juce::juce_wchar c)
{
    switch (c)
    {
        case '0': return "abcdef";
        case '1': return "bc";
        case '2': return "abdeg";
        case '3': return "abcdg";
        case '4': return "bcfg";
        case '5': return "acdfg";
        case '6': return "acdefg";
        case '7': return "abc";
        case '8': return "abcdefg";
        case '9': return "abcdfg";
        case '-': return "g";
        default: return "";
    }
}

const std::array<juce::Path, 7>& segmentPaths()
{
    static const auto paths = []
    {
        std::array<juce::Path, 7> out;
        for (int s = 0; s < 7; ++s)
        {
            auto tokens = juce::StringArray::fromTokens (segmentPoints[s], " ", "");
            for (int i = 0; i < tokens.size(); ++i)
            {
                const auto xy = juce::StringArray::fromTokens (tokens[i], ",", "");
                const juce::Point<float> p (xy[0].getFloatValue(), xy[1].getFloatValue());
                if (i == 0)
                    out[(size_t) s].startNewSubPath (p);
                else
                    out[(size_t) s].lineTo (p);
            }
            out[(size_t) s].closeSubPath();
        }
        return out;
    }();
    return paths;
}
} // namespace

float sevenSegmentWidth (const juce::String& digits, float h)
{
    const float scale = h / 27.0f;
    float w = 0;
    for (auto c : digits)
        w += (c == '.' ? 5.0f : 17.0f) * scale;
    return w;
}

void drawSevenSegment (juce::Graphics& g, const juce::String& digits, juce::Point<float> topLeft, float h, juce::Colour colour)
{
    const float scale = h / 27.0f;
    float x = topLeft.x;
    const auto& paths = segmentPaths();
    for (auto c : digits)
    {
        if (c == '.')
        {
            g.setColour (colour);
            g.fillEllipse (x + 0.5f * scale, topLeft.y + 21.5f * scale, 3.0f * scale, 3.0f * scale);
            x += 5.0f * scale;
            continue;
        }
        const juce::String lit (segmentsFor (c));
        const auto transform = juce::AffineTransform::scale (scale).translated (x, topLeft.y);
        for (int s = 0; s < 7; ++s)
        {
            const bool on = lit.containsChar ((juce::juce_wchar) ('a' + s));
            g.setColour (on ? colour : colour.withAlpha (0.065f));
            g.fillPath (paths[(size_t) s], transform);
        }
        x += 17.0f * scale;
    }
}

void drawReadout (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& number, const juce::String& unit,
                  float h, juce::Colour colour)
{
    g.setColour (colours::display);
    g.fillRoundedRectangle (area, 3.0f);
    g.setColour (juce::Colour (0xff2e2e2e));
    g.drawRoundedRectangle (area.reduced (0.5f), 3.0f, 1.0f);

    const float unitSize = h * 0.5f;
    const auto unitFont = font (unitSize, true);
    const float unitW = unit.isEmpty() ? 0.0f : juce::GlyphArrangement::getStringWidth (unitFont, unit) + h * 0.25f;
    const float digitsW = sevenSegmentWidth (number, h);
    const float total = digitsW + unitW;
    const float x = area.getCentreX() - total * 0.5f;
    const float y = area.getCentreY() - h * 0.5f;
    drawSevenSegment (g, number, { x, y }, h, colour);
    if (unit.isNotEmpty())
    {
        g.setFont (unitFont);
        g.setColour (colour);
        g.drawText (unit, juce::Rectangle<float> (x + digitsW + h * 0.2f, y, unitW + 4, h), juce::Justification::centredLeft);
    }
}

// ---------------------------------------------------------------------------------------------
namespace
{
juce::Path svg (const char* d)
{
    return juce::Drawable::parseSVGPath (d);
}

void addEllipse (juce::Path& p, float cx, float cy, float rx, float ry)
{
    p.addEllipse (cx - rx, cy - ry, rx * 2, ry * 2);
}
} // namespace

juce::Path instrumentIcon (int voice)
{
    juce::Path p;
    switch (voice)
    {
        case 0:
        case 1:
            addEllipse (p, 16, 18, 13, 4);
            p.addPath (svg ("M16 4v22m-6 0h12"));
            if (voice == 1)
                addEllipse (p, 16, 13, 10, 3);
            break;
        case 2:
            addEllipse (p, 16, 7, 11, 4);
            p.addPath (svg ("M5 7v16m22-16v16M5 23q11 7 22 0M10 10v13m12-13v13"));
            break;
        case 3:
            addEllipse (p, 16, 8, 12, 4);
            p.addPath (svg ("M4 8l3 17q9 5 18 0l3-17M10 12v13m12-13v13"));
            break;
        case 4: p = svg ("m6 23 16-18q5-3 7 3L12 27q-5 3-6-4Zm12-13 6 6"); break;
        case 5: p = svg ("m9 22-3-8q-1-4 2-5l5 7-3-11q1-4 4-2l4 11 1-10q3-3 4 1l-1 10 5-5q4-1 3 3l-9 13q-8 4-12-4Z"); break;
        case 6:
            addEllipse (p, 16, 14, 5, 5);
            p.addPath (svg ("M13 8q-8-10-9 0l6 6q-13 5-4 10l7-5q1 14 8 7l-1-9q14-1 9-8l-9 1q-1-12-7-2"));
            break;
        case 7:
            addEllipse (p, 16, 9, 11, 5);
            p.addPath (svg ("m5 10 6 16h10l6-16"));
            break;
        default: p = svg ("M4 9v14m24-14v14M4 16q12-13 24 0-12 13-24 0Z"); break;
    }
    return p;
}

juce::Path waveIcon (int wave)
{
    static const char* paths[] { "M2 12Q8 0 14 12T26 12", "M2 18 14 5 26 18", "M2 18V5L14 18V5L26 18",
                                 "M2 12h3l2-7 3 15 3-17 3 20 3-15 2 8 3-4h3",
                                 "M3 12a7 7 0 1 0 14 0 7 7 0 1 0-14 0M12 12a7 7 0 1 0 14 0 7 7 0 1 0-14 0" };
    return svg (paths[juce::jlimit (0, 4, wave)]);
}
} // namespace percusyn::ui
