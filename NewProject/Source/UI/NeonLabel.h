#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * NeonLabel — a text label rendered with a neon-sign glow effect.
 *
 * The text is drawn multiple times at slight offsets with decreasing opacity
 * to simulate a bloom/glow, then a bright, crisp version is drawn on top.
 * Best used with uppercase block letters for the classic neon sign aesthetic.
 *
 * Default neon colour is purple (#9370DB).
 */
class NeonLabel : public juce::Component
{
public:
    NeonLabel();
    explicit NeonLabel (const juce::String& initialText);
    ~NeonLabel() override = default;

    // ── Public API ──────────────────────────────────────────────────────

    /** Set the display text.  Triggers a repaint. */
    void setText (const juce::String& text);

    /** Get the current display text. */
    const juce::String& getText() const noexcept { return displayText; }

    /** Set the neon glow colour (default: #9370DB medium purple). */
    void setNeonColour (juce::Colour colour);

    /** Get the current neon colour. */
    juce::Colour getNeonColour() const noexcept { return neonColour; }

    /** Set the font size in points. */
    void setFontSize (float size);

    /** Get the current font size. */
    float getFontSize() const noexcept { return fontSize; }

    /** Set the text justification (default: centred). */
    void setJustification (juce::Justification j);

    // ── Component overrides ─────────────────────────────────────────────

    void paint (juce::Graphics& g) override;

private:
    // ── Painting helpers ────────────────────────────────────────────────

    /** Draw a single text pass at the given offset with the given colour. */
    void drawTextLayer (juce::Graphics& g, float offsetX, float offsetY,
                        juce::Colour colour) const;

    // ── Data ────────────────────────────────────────────────────────────

    juce::String        displayText;
    juce::Colour        neonColour   { 0xFF9370DB };   // medium purple
    float               fontSize     { 22.0f };
    juce::Justification justification { juce::Justification::centred };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonLabel)
};
