#include "NeonLabel.h"
#include <cmath>

// ═════════════════════════════════════════════════════════════════════════════
//  Construction
// ═════════════════════════════════════════════════════════════════════════════

NeonLabel::NeonLabel()
{
    setInterceptsMouseClicks (false, false);
}

NeonLabel::NeonLabel (const juce::String& initialText)
    : displayText (initialText)
{
    setInterceptsMouseClicks (false, false);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Public API
// ═════════════════════════════════════════════════════════════════════════════

void NeonLabel::setText (const juce::String& text)
{
    if (displayText != text)
    {
        displayText = text;
        repaint();
    }
}

void NeonLabel::setNeonColour (juce::Colour colour)
{
    if (neonColour != colour)
    {
        neonColour = colour;
        repaint();
    }
}

void NeonLabel::setFontSize (float size)
{
    size = juce::jmax (1.0f, size);
    if (! juce::approximatelyEqual (fontSize, size))
    {
        fontSize = size;
        repaint();
    }
}

void NeonLabel::setJustification (juce::Justification j)
{
    if (justification != j)
    {
        justification = j;
        repaint();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Painting
// ═════════════════════════════════════════════════════════════════════════════

void NeonLabel::paint (juce::Graphics& g)
{
    if (displayText.isEmpty())
        return;

    // ── Glow layers (outermost to innermost) ────────────────────────────
    //
    // Each layer is drawn at multiple offsets around the centre to create a
    // smooth, omnidirectional bloom.  Larger radius == more transparent.

    struct GlowPass
    {
        float radius;
        float alpha;
    };

    static constexpr GlowPass passes[] = {
        { 6.0f, 0.04f },
        { 4.5f, 0.07f },
        { 3.0f, 0.12f },
        { 2.0f, 0.18f },
        { 1.2f, 0.28f },
        { 0.6f, 0.45f }
    };

    for (const auto& pass : passes)
    {
        juce::Colour glowCol = neonColour.withAlpha (pass.alpha);

        // Draw at 8 points around a circle of the given radius
        const int numOffsets = 8;
        for (int i = 0; i < numOffsets; ++i)
        {
            float angle = juce::MathConstants<float>::twoPi
                        * static_cast<float> (i) / static_cast<float> (numOffsets);
            float dx = std::cos (angle) * pass.radius;
            float dy = std::sin (angle) * pass.radius;
            drawTextLayer (g, dx, dy, glowCol);
        }
    }

    // ── Bright core text ────────────────────────────────────────────────
    // A saturated version of the neon colour
    drawTextLayer (g, 0.0f, 0.0f, neonColour.brighter (0.35f));

    // ── White-hot highlight down the centre ─────────────────────────────
    // Slightly transparent white gives the "hot filament" look
    drawTextLayer (g, 0.0f, 0.0f, juce::Colours::white.withAlpha (0.45f));
}

void NeonLabel::drawTextLayer (juce::Graphics& g, float offsetX, float offsetY,
                               juce::Colour colour) const
{
    auto area = getLocalBounds().toFloat().translated (offsetX, offsetY);

    g.setColour (colour);
    g.setFont (juce::FontOptions (fontSize).withStyle ("Bold"));
    g.drawText (displayText.toUpperCase(), area, justification, true);
}
