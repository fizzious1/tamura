#include "WaveformDisplay.h"

namespace tamura
{

// ═════════════════════════════════════════════════════════════════════════════
//  Construction
// ═════════════════════════════════════════════════════════════════════════════

WaveformDisplay::WaveformDisplay()
{
    setOpaque (true);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Public API
// ═════════════════════════════════════════════════════════════════════════════

void WaveformDisplay::setAudioBuffer (const juce::AudioBuffer<float>* buffer)
{
    audioBuffer = buffer;
    repaint();
}

void WaveformDisplay::setSamplesPerPixel (int spp)
{
    samplesPerPixel = juce::jmax (1, spp);
    repaint();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Component overrides
// ═════════════════════════════════════════════════════════════════════════════

void WaveformDisplay::resized()
{
    // nothing extra needed — all layout is computed on-the-fly in paint()
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    paintBezel (g);

    auto screen = getScreenArea();

    // Clip all inner drawing to the screen rectangle
    g.saveState();
    g.reduceClipRegion (screen.toNearestInt());

    paintBackground (g, screen);
    paintGrid       (g, screen);

    if (audioBuffer != nullptr && audioBuffer->getNumSamples() > 0)
        paintWaveform (g, screen);
    else
        paintIdleLine (g, screen);

    paintScanlines (g, screen);

    g.restoreState();
}

// ═════════════════════════════════════════════════════════════════════════════
//  Geometry
// ═════════════════════════════════════════════════════════════════════════════

juce::Rectangle<float> WaveformDisplay::getScreenArea() const
{
    return getLocalBounds().toFloat().reduced (bezelThickness);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Painting helpers
// ═════════════════════════════════════════════════════════════════════════════

void WaveformDisplay::paintBezel (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Outer chrome bezel — a linear gradient from dark to light gives a
    // brushed-metal look.
    juce::ColourGradient bezelGrad (juce::Colour (colBezelDark), bounds.getX(), bounds.getY(),
                                    juce::Colour (colBezelLight), bounds.getRight(), bounds.getBottom(),
                                    false);
    g.setGradientFill (bezelGrad);
    g.fillRoundedRectangle (bounds, 3.0f);

    // Thin bright specular highlight on top edge
    g.setColour (juce::Colour (0x30FFFFFF));
    g.drawHorizontalLine (static_cast<int> (bounds.getY() + 1.0f),
                          bounds.getX() + 4.0f, bounds.getRight() - 4.0f);

    // Inner shadow just inside the bezel to make the screen look inset
    auto innerRect = bounds.reduced (bezelThickness);
    g.setColour (juce::Colour (0x60000000));
    g.drawRect (innerRect, 1.0f);

    // Decorative LEDs on the left side of the bezel
    paintLEDs (g, bounds);

    // Decorative knobs and labels
    paintDecorativeKnobs (g, bounds);
    paintBezelLabels (g, bounds);
}

void WaveformDisplay::paintLEDs (juce::Graphics& g, juce::Rectangle<float> bezelRect)
{
    // Three small LED dots vertically centred in the left bezel strip
    const float ledRadius  = 2.0f;
    const float ledCentreX = bezelRect.getX() + bezelThickness * 0.5f;
    const float spacing    = 10.0f;
    const float startY     = bezelRect.getCentreY() - spacing;

    const juce::Colour ledColours[] = {
        juce::Colour (0xFF00FF80),   // green
        juce::Colour (0xFFFFCC00),   // amber
        juce::Colour (0xFFFF3040)    // red
    };

    for (int i = 0; i < 3; ++i)
    {
        float cy = startY + static_cast<float> (i) * spacing;

        // Soft glow behind the LED
        g.setColour (ledColours[i].withAlpha (0.25f));
        g.fillEllipse (ledCentreX - ledRadius * 2.0f, cy - ledRadius * 2.0f,
                        ledRadius * 4.0f, ledRadius * 4.0f);

        // The LED dot itself
        g.setColour (ledColours[i]);
        g.fillEllipse (ledCentreX - ledRadius, cy - ledRadius,
                        ledRadius * 2.0f, ledRadius * 2.0f);

        // Bright specular highlight on the LED
        g.setColour (ledColours[i].brighter (0.6f).withAlpha (0.8f));
        g.fillEllipse (ledCentreX - ledRadius * 0.5f, cy - ledRadius * 0.7f,
                        ledRadius, ledRadius);
    }
}

void WaveformDisplay::paintBackground (juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour (juce::Colour (colBackground));
    g.fillRect (area);
}

void WaveformDisplay::paintGrid (juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour (juce::Colour (colGrid));

    // Vertical grid lines (8 divisions)
    const int numVertical = 8;
    for (int i = 1; i < numVertical; ++i)
    {
        float x = area.getX() + area.getWidth() * static_cast<float> (i) / static_cast<float> (numVertical);
        g.drawVerticalLine (static_cast<int> (x), area.getY(), area.getBottom());
    }

    // Horizontal grid lines (6 divisions)
    const int numHorizontal = 6;
    for (int i = 1; i < numHorizontal; ++i)
    {
        float y = area.getY() + area.getHeight() * static_cast<float> (i) / static_cast<float> (numHorizontal);
        g.drawHorizontalLine (static_cast<int> (y), area.getX(), area.getRight());
    }

    // Centre line (slightly brighter)
    g.setColour (juce::Colour (colGrid).brighter (0.4f));
    float centreY = area.getCentreY();
    g.drawHorizontalLine (static_cast<int> (centreY), area.getX(), area.getRight());
}

void WaveformDisplay::paintWaveform (juce::Graphics& g, juce::Rectangle<float> area)
{
    const int numSamples  = audioBuffer->getNumSamples();
    const int numChannels = audioBuffer->getNumChannels();
    const int width       = static_cast<int> (area.getWidth());

    if (width <= 0 || numSamples == 0)
        return;

    const float centreY   = area.getCentreY();
    const float halfH     = area.getHeight() * 0.5f;
    const float startX    = area.getX();

    // Build a path from the audio data (mono-mix if stereo)
    juce::Path waveformPath;
    bool pathStarted = false;

    for (int px = 0; px < width; ++px)
    {
        const int sampleStart = px * samplesPerPixel;
        const int sampleEnd   = juce::jmin (sampleStart + samplesPerPixel, numSamples);

        if (sampleStart >= numSamples)
            break;

        // Find the min and max sample values for this pixel column
        float minVal =  1.0f;
        float maxVal = -1.0f;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = audioBuffer->getReadPointer (ch);
            for (int s = sampleStart; s < sampleEnd; ++s)
            {
                const float v = data[s];
                if (v < minVal) minVal = v;
                if (v > maxVal) maxVal = v;
            }
        }

        // Normalise across channels
        if (numChannels > 1)
        {
            minVal /= static_cast<float> (numChannels);
            maxVal /= static_cast<float> (numChannels);
        }

        float x  = startX + static_cast<float> (px);
        float y1 = centreY - maxVal * halfH;
        float y2 = centreY - minVal * halfH;

        // Clamp
        y1 = juce::jlimit (area.getY(), area.getBottom(), y1);
        y2 = juce::jlimit (area.getY(), area.getBottom(), y2);

        if (! pathStarted)
        {
            waveformPath.startNewSubPath (x, y1);
            pathStarted = true;
        }

        waveformPath.lineTo (x, y1);
        waveformPath.lineTo (x, y2);
    }

    // ── Glow layers (bloom effect) ──────────────────────────────────────

    const juce::Colour waveCol (colWaveform);

    // Outer glow — wide, very transparent
    {
        juce::Path glowPath (waveformPath);
        g.setColour (waveCol.withAlpha (0.08f));
        g.strokePath (glowPath, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }
    // Mid glow
    {
        juce::Path glowPath (waveformPath);
        g.setColour (waveCol.withAlpha (0.15f));
        g.strokePath (glowPath, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
    }
    // Inner bright stroke
    {
        g.setColour (waveCol.withAlpha (0.85f));
        g.strokePath (waveformPath, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
    }
    // Core white-hot centre
    {
        g.setColour (waveCol.brighter (0.5f).withAlpha (0.6f));
        g.strokePath (waveformPath, juce::PathStrokeType (0.6f, juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
    }
}

void WaveformDisplay::paintIdleLine (juce::Graphics& g, juce::Rectangle<float> area)
{
    // Draw a flat centre line with subtle deterministic "noise" bumps so the
    // display looks alive even when no buffer is loaded.

    const juce::Colour waveCol (colWaveform);
    const float centreY = area.getCentreY();
    const float startX  = area.getX();
    const int   width   = static_cast<int> (area.getWidth());

    juce::Path noisePath;
    noisePath.startNewSubPath (startX, centreY);

    // Simple deterministic pseudo-noise (no randomness so it is stable per repaint)
    for (int px = 0; px < width; ++px)
    {
        float noise = std::sin (static_cast<float> (px) * 0.7f) * 0.3f
                    + std::sin (static_cast<float> (px) * 1.9f) * 0.15f
                    + std::sin (static_cast<float> (px) * 4.3f) * 0.08f;
        float y = centreY + noise;
        noisePath.lineTo (startX + static_cast<float> (px), y);
    }

    // Glow
    g.setColour (waveCol.withAlpha (0.06f));
    g.strokePath (noisePath, juce::PathStrokeType (5.0f));

    g.setColour (waveCol.withAlpha (0.12f));
    g.strokePath (noisePath, juce::PathStrokeType (2.5f));

    // Main line
    g.setColour (waveCol.withAlpha (0.5f));
    g.strokePath (noisePath, juce::PathStrokeType (1.0f));
}

void WaveformDisplay::paintDecorativeKnobs (juce::Graphics& g, juce::Rectangle<float> bezelRect)
{
    // Two small decorative chrome knobs at bottom-left and bottom-right of the bezel
    auto drawKnob = [&] (float cx, float cy, float radius)
    {
        // Outer ring
        g.setColour (juce::Colour (colBezelLight));
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        // Inner face (dark)
        float inner = radius * 0.7f;
        juce::ColourGradient knobGrad (juce::Colour (0xFF5A5A64), cx, cy - inner,
                                        juce::Colour (0xFF28282E), cx, cy + inner, false);
        g.setGradientFill (knobGrad);
        g.fillEllipse (cx - inner, cy - inner, inner * 2.0f, inner * 2.0f);

        // Indicator line
        g.setColour (juce::Colour (colWaveform).withAlpha (0.6f));
        g.drawLine (cx, cy - inner * 0.8f, cx, cy - inner * 0.3f, 1.5f);

        // Specular dot
        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.fillEllipse (cx - 1.0f, cy - inner * 0.5f, 2.0f, 2.0f);
    };

    float knobY = bezelRect.getBottom() - bezelThickness * 0.5f;
    drawKnob (bezelRect.getX() + 30.0f, knobY, 5.0f);
    drawKnob (bezelRect.getRight() - 30.0f, knobY, 5.0f);

    // Right-side LEDs (mirror of left-side)
    const float ledRadius  = 2.0f;
    const float ledCentreX = bezelRect.getRight() - bezelThickness * 0.5f;
    const float spacing    = 10.0f;
    const float startY     = bezelRect.getCentreY() - spacing;

    const juce::Colour ledColours[] = {
        juce::Colour (0xFF00FF80),
        juce::Colour (0xFFFFCC00),
        juce::Colour (0xFFFF3040)
    };

    for (int i = 0; i < 3; ++i)
    {
        float cy = startY + static_cast<float> (i) * spacing;

        g.setColour (ledColours[i].withAlpha (0.25f));
        g.fillEllipse (ledCentreX - ledRadius * 2.0f, cy - ledRadius * 2.0f,
                        ledRadius * 4.0f, ledRadius * 4.0f);

        g.setColour (ledColours[i]);
        g.fillEllipse (ledCentreX - ledRadius, cy - ledRadius,
                        ledRadius * 2.0f, ledRadius * 2.0f);

        g.setColour (ledColours[i].brighter (0.6f).withAlpha (0.8f));
        g.fillEllipse (ledCentreX - ledRadius * 0.5f, cy - ledRadius * 0.7f,
                        ledRadius, ledRadius);
    }
}

void WaveformDisplay::paintBezelLabels (juce::Graphics& g, juce::Rectangle<float> bezelRect)
{
    auto font = juce::Font (juce::FontOptions().withHeight (8.0f));
    g.setFont (font);
    g.setColour (juce::Colour (0xFF9E9E8E));

    // "TIME" label at bottom-left
    g.drawText ("TIME", juce::Rectangle<float> (bezelRect.getX() + 42.0f,
        bezelRect.getBottom() - bezelThickness - 1.0f, 30.0f, bezelThickness),
        juce::Justification::centredLeft, false);

    // "FREQ" label at bottom-right
    g.drawText ("FREQ", juce::Rectangle<float> (bezelRect.getRight() - 72.0f,
        bezelRect.getBottom() - bezelThickness - 1.0f, 30.0f, bezelThickness),
        juce::Justification::centredRight, false);
}

void WaveformDisplay::paintScanlines (juce::Graphics& g, juce::Rectangle<float> area)
{
    // Very subtle alternating dark horizontal lines to mimic a CRT scanline
    // effect.  Every other pixel-row gets a faint dark overlay.
    g.setColour (juce::Colour (colScanline));

    const int top    = static_cast<int> (area.getY());
    const int bottom = static_cast<int> (area.getBottom());

    for (int y = top; y < bottom; y += 2)
    {
        g.drawHorizontalLine (y, area.getX(), area.getRight());
    }
}

} // namespace tamura
