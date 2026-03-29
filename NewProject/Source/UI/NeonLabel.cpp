#include "NeonLabel.h"
#include <cmath>

namespace tamura
{

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

void NeonLabel::setDrawFrame (bool shouldDraw)
{
    if (drawFrame != shouldDraw)
    {
        drawFrame = shouldDraw;
        repaint();
    }
}

void NeonLabel::setFramePadding (float padding)
{
    framePadding = juce::jmax (0.0f, padding);
    repaint();
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

    // ── Neon rectangular frame ────────────────────────────────────────
    if (drawFrame)
    {
        auto frameRect = getLocalBounds().toFloat().reduced (framePadding);

        // Glow passes (same multi-layer technique as text glow)
        struct FrameGlow { float expand; float alpha; };
        static constexpr FrameGlow frameGlows[] = {
            { 6.0f, 0.04f }, { 4.0f, 0.07f }, { 2.5f, 0.12f }, { 1.2f, 0.22f }
        };

        for (const auto& fg : frameGlows)
        {
            g.setColour (neonColour.withAlpha (fg.alpha));
            g.drawRoundedRectangle (frameRect.expanded (fg.expand), 3.0f, 2.0f);
        }

        // Bright core border
        g.setColour (neonColour.brighter (0.3f));
        g.drawRoundedRectangle (frameRect, 3.0f, 2.0f);

        // White-hot highlight
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.drawRoundedRectangle (frameRect, 3.0f, 1.0f);
    }
}

void NeonLabel::drawTextLayer (juce::Graphics& g, float offsetX, float offsetY,
                               juce::Colour colour) const
{
    auto area = getLocalBounds().toFloat().translated (offsetX, offsetY);

    g.setColour (colour);
    g.setFont (juce::FontOptions (fontSize).withStyle ("Bold"));
    g.drawText (displayText.toUpperCase(), area, justification, true);
}

} // namespace tamura
