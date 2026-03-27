#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tamura
{

//==============================================================================
/** Colour palette constants for the Tamura cyberpunk-industrial aesthetic. */
namespace Palette
{
    // Background & surfaces
    static constexpr juce::uint32 background        = 0xFF0D0D12;
    static constexpr juce::uint32 surfaceDark       = 0xFF1A1A24;
    static constexpr juce::uint32 surfaceMid        = 0xFF22222E;

    // Metallic gradients (steel)
    static constexpr juce::uint32 steelDark         = 0xFF3A3A42;
    static constexpr juce::uint32 steelLight        = 0xFF5A5A64;
    static constexpr juce::uint32 steelHighlight    = 0xFF787880;
    static constexpr juce::uint32 steelShadow       = 0xFF28282E;

    // Chrome / bevel highlights
    static constexpr juce::uint32 chromeBright      = 0xFFA0A0AA;
    static constexpr juce::uint32 chromeMid         = 0xFF6E6E78;
    static constexpr juce::uint32 chromeDim         = 0xFF484852;

    // Neon accent colours
    static constexpr juce::uint32 neonPurple        = 0xFF7B68EE;
    static constexpr juce::uint32 neonBlue          = 0xFF4169E1;
    static constexpr juce::uint32 neonLavender      = 0xFF9370DB;
    static constexpr juce::uint32 neonGlow          = 0x407B68EE;  // semi-transparent glow

    // Text colours
    static constexpr juce::uint32 textCream         = 0xFFF0E6D2;
    static constexpr juce::uint32 textDim           = 0xFF9E9E8E;
    static constexpr juce::uint32 textBright        = 0xFFFFFFFF;

    // Tier colours (for result feedback)
    static constexpr juce::uint32 tierCommon        = 0xFFA0A0A0;
    static constexpr juce::uint32 tierUncommon      = 0xFF4CAF50;
    static constexpr juce::uint32 tierRare          = 0xFF2196F3;
    static constexpr juce::uint32 tierEpic          = 0xFF9C27B0;
    static constexpr juce::uint32 tierLegendary     = 0xFFFFD700;
}

//==============================================================================
/**
    Custom LookAndFeel for the Tamura slot-machine audio plugin.

    Provides a dark cyberpunk-industrial aesthetic with brushed steel surfaces,
    chrome bevels, and neon purple/blue accents.  All rendering is done through
    JUCE Graphics calls -- no external images required.
*/
class TamuraLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TamuraLookAndFeel();
    ~TamuraLookAndFeel() override = default;

    //==========================================================================
    // LookAndFeel overrides
    //==========================================================================

    /** Metallic button with beveled edges and optional neon outline. */
    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText (juce::Graphics& g,
                         juce::TextButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;

    /** Neon-styled label text. */
    void drawLabel (juce::Graphics& g, juce::Label& label) override;

    /** Rotary slider rendered as a chrome knob with neon indicator. */
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override;

    //==========================================================================
    // Static helper methods -- callable from any component that casts its L&F.
    //==========================================================================

    /** Paint a rectangular brushed-steel surface with beveled edges.
        @param g         Graphics context
        @param bounds    Rectangle to fill
        @param bevelPx   Width of the bevel highlight/shadow edges (default 2)
    */
    static void drawMetallicSurface (juce::Graphics& g,
                                     juce::Rectangle<float> bounds,
                                     float bevelPx = 2.0f);

    /** Paint a soft neon glow around the given rectangle.
        @param g          Graphics context
        @param bounds     Area to glow around
        @param glowColour Base glow colour (alpha will be modulated)
        @param radius     Spread of the glow in pixels
    */
    static void drawNeonGlow (juce::Graphics& g,
                              juce::Rectangle<float> bounds,
                              juce::Colour glowColour,
                              float radius = 6.0f);

    /** Paint a small decorative screw head (circle with slot).
        @param g       Graphics context
        @param centre  Centre point of the screw
        @param radius  Radius of the screw head
    */
    static void drawScrewHead (juce::Graphics& g,
                               juce::Point<float> centre,
                               float radius = 4.0f);

    /** Paint a chrome ring (used for reel frames, knobs, etc.).
        @param g           Graphics context
        @param centre      Centre of the ring
        @param outerRadius Outer radius
        @param thickness   Ring thickness in pixels
    */
    static void drawChromeRing (juce::Graphics& g,
                                juce::Point<float> centre,
                                float outerRadius,
                                float thickness = 4.0f);

    /** Paint a glass dome highlight arc over a circular area.
        @param g       Graphics context
        @param centre  Centre of the circle
        @param radius  Radius of the dome area
    */
    static void drawGlassDome (juce::Graphics& g,
                               juce::Point<float> centre,
                               float radius);

private:
    juce::Font getDefaultFont() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TamuraLookAndFeel)
};

} // namespace tamura
