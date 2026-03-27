#include "TamuraLookAndFeel.h"

namespace tamura
{

//==============================================================================
TamuraLookAndFeel::TamuraLookAndFeel()
{
    // -- Colour scheme ---------------------------------------------------
    setColour (juce::ResizableWindow::backgroundColourId,
               juce::Colour (Palette::background));

    // Default label colours
    setColour (juce::Label::textColourId,
               juce::Colour (Palette::textCream));
    setColour (juce::Label::backgroundColourId,
               juce::Colours::transparentBlack);

    // TextButton colours
    setColour (juce::TextButton::buttonColourId,
               juce::Colour (Palette::steelDark));
    setColour (juce::TextButton::buttonOnColourId,
               juce::Colour (Palette::neonPurple));
    setColour (juce::TextButton::textColourOffId,
               juce::Colour (Palette::textCream));
    setColour (juce::TextButton::textColourOnId,
               juce::Colour (Palette::textBright));

    // Slider colours
    setColour (juce::Slider::thumbColourId,
               juce::Colour (Palette::neonPurple));
    setColour (juce::Slider::rotarySliderFillColourId,
               juce::Colour (Palette::neonBlue));
    setColour (juce::Slider::rotarySliderOutlineColourId,
               juce::Colour (Palette::steelDark));
    setColour (juce::Slider::trackColourId,
               juce::Colour (Palette::steelShadow));

    // Tooltip
    setColour (juce::TooltipWindow::backgroundColourId,
               juce::Colour (Palette::surfaceDark));
    setColour (juce::TooltipWindow::textColourId,
               juce::Colour (Palette::textCream));

    // Set the default sans-serif typeface name
    setDefaultSansSerifTypefaceName ("Arial");
}

//==============================================================================
// drawButtonBackground
//==============================================================================
void TamuraLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour& /*backgroundColour*/,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const float cornerRadius = 6.0f;

    // --- Metallic body gradient (vertical) --------------------------------
    juce::Colour topColour    = juce::Colour (Palette::steelLight);
    juce::Colour bottomColour = juce::Colour (Palette::steelDark);

    if (shouldDrawButtonAsDown)
    {
        topColour    = juce::Colour (Palette::steelDark);
        bottomColour = juce::Colour (Palette::steelShadow);
    }
    else if (shouldDrawButtonAsHighlighted)
    {
        topColour    = juce::Colour (Palette::steelHighlight);
        bottomColour = juce::Colour (Palette::steelLight);
    }

    g.setGradientFill (juce::ColourGradient (topColour,    0.0f, bounds.getY(),
                                              bottomColour, 0.0f, bounds.getBottom(),
                                              false));
    g.fillRoundedRectangle (bounds, cornerRadius);

    // --- Bevel highlights / shadows ----------------------------------------
    const float bevelPx = 1.5f;

    // Top / left highlight
    g.setColour (juce::Colour (Palette::chromeBright).withAlpha (0.35f));
    g.drawRoundedRectangle (bounds.reduced (bevelPx * 0.5f), cornerRadius, bevelPx);

    // Bottom / right shadow (draw a slightly offset inner rect)
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.drawRoundedRectangle (bounds.translated (0.5f, 0.5f).reduced (bevelPx), cornerRadius, bevelPx);

    // --- Neon outline glow on toggle-on buttons ----------------------------
    if (button.getToggleState())
    {
        drawNeonGlow (g, bounds, juce::Colour (Palette::neonPurple), 4.0f);
        g.setColour (juce::Colour (Palette::neonPurple).withAlpha (0.7f));
        g.drawRoundedRectangle (bounds, cornerRadius, 1.5f);
    }
}

//==============================================================================
// drawButtonText
//==============================================================================
void TamuraLookAndFeel::drawButtonText (juce::Graphics& g,
                                         juce::TextButton& button,
                                         bool /*shouldDrawButtonAsHighlighted*/,
                                         bool shouldDrawButtonAsDown)
{
    auto font = juce::Font (juce::FontOptions()
                    .withHeight (juce::jmin (16.0f, (float) button.getHeight() * 0.6f))
                    .withStyle ("Bold"));

    g.setFont (font);

    auto textColour = button.findColour (button.getToggleState()
                                             ? juce::TextButton::textColourOnId
                                             : juce::TextButton::textColourOffId);
    g.setColour (textColour);

    auto yOffset = shouldDrawButtonAsDown ? 1 : 0;

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().translated (0, yOffset),
                      juce::Justification::centred, 1);
}

//==============================================================================
// drawLabel
//==============================================================================
void TamuraLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    auto bounds = label.getLocalBounds().toFloat();

    // Fill background if set
    auto bgColour = label.findColour (juce::Label::backgroundColourId);
    if (! bgColour.isTransparent())
    {
        g.setColour (bgColour);
        g.fillRect (bounds);
    }

    // Text
    auto textColour = label.findColour (juce::Label::textColourId);
    g.setColour (textColour);

    auto font = label.getFont();
    g.setFont (font);

    auto textBounds = label.getBorderSize().subtractedFrom (label.getLocalBounds());

    // Subtle neon text shadow (one pixel offset, purple tint)
    g.setColour (juce::Colour (Palette::neonPurple).withAlpha (0.20f));
    g.drawFittedText (label.getText(),
                      textBounds.translated (1, 1),
                      label.getJustificationType(),
                      juce::jmax (1, (int) ((float) textBounds.getHeight() / font.getHeight())),
                      label.getMinimumHorizontalScale());

    // Main text
    g.setColour (textColour);
    g.drawFittedText (label.getText(),
                      textBounds,
                      label.getJustificationType(),
                      juce::jmax (1, (int) ((float) textBounds.getHeight() / font.getHeight())),
                      label.getMinimumHorizontalScale());
}

//==============================================================================
// drawRotarySlider
//==============================================================================
void TamuraLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                           int x, int y, int width, int height,
                                           float sliderPos,
                                           float rotaryStartAngle,
                                           float rotaryEndAngle,
                                           juce::Slider& /*slider*/)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centre = bounds.getCentre();

    // -- Outer chrome ring -----------------------------------------------
    drawChromeRing (g, centre, radius, 3.0f);

    // -- Knob body -------------------------------------------------------
    float knobRadius = radius - 5.0f;
    juce::ColourGradient knobGrad (juce::Colour (Palette::steelLight),
                                   centre.x, centre.y - knobRadius,
                                   juce::Colour (Palette::steelShadow),
                                   centre.x, centre.y + knobRadius,
                                   false);
    g.setGradientFill (knobGrad);
    g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius,
                   knobRadius * 2.0f, knobRadius * 2.0f);

    // -- Indicator line --------------------------------------------------
    float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    float lineLen  = knobRadius * 0.65f;
    float lineEndX = centre.x + lineLen * std::sin (angle);
    float lineEndY = centre.y - lineLen * std::cos (angle);

    g.setColour (juce::Colour (Palette::neonPurple));
    g.drawLine (centre.x, centre.y, lineEndX, lineEndY, 2.5f);

    // -- Centre dot ------------------------------------------------------
    g.setColour (juce::Colour (Palette::neonLavender));
    g.fillEllipse (centre.x - 3.0f, centre.y - 3.0f, 6.0f, 6.0f);
}

//==============================================================================
// drawMetallicSurface
//==============================================================================
void TamuraLookAndFeel::drawMetallicSurface (juce::Graphics& g,
                                              juce::Rectangle<float> bounds,
                                              float bevelPx)
{
    // Vertical steel gradient
    juce::ColourGradient grad (juce::Colour (Palette::steelLight),
                               0.0f, bounds.getY(),
                               juce::Colour (Palette::steelDark),
                               0.0f, bounds.getBottom(),
                               false);
    g.setGradientFill (grad);
    g.fillRect (bounds);

    // Top bevel highlight
    g.setColour (juce::Colour (Palette::chromeBright).withAlpha (0.3f));
    g.fillRect (bounds.removeFromTop (bevelPx));

    // Left highlight
    g.setColour (juce::Colour (Palette::chromeBright).withAlpha (0.15f));
    g.fillRect (bounds.removeFromLeft (bevelPx));

    // Bottom shadow
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRect (bounds.removeFromBottom (bevelPx));

    // Right shadow
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRect (bounds.removeFromRight (bevelPx));
}

//==============================================================================
// drawNeonGlow
//==============================================================================
void TamuraLookAndFeel::drawNeonGlow (juce::Graphics& g,
                                       juce::Rectangle<float> bounds,
                                       juce::Colour glowColour,
                                       float radius)
{
    // Draw multiple expanding, increasingly transparent rounded rectangles
    const int passes = juce::jmax (1, (int) radius);

    for (int i = passes; i >= 1; --i)
    {
        float expand = (float) i * 1.0f;
        float alpha  = glowColour.getFloatAlpha() * (1.0f - (float) i / ((float) passes + 1.0f)) * 0.35f;

        g.setColour (glowColour.withAlpha (alpha));
        g.drawRoundedRectangle (bounds.expanded (expand), 6.0f, 1.5f);
    }
}

//==============================================================================
// drawScrewHead
//==============================================================================
void TamuraLookAndFeel::drawScrewHead (juce::Graphics& g,
                                        juce::Point<float> centre,
                                        float radius)
{
    // Outer ring
    juce::ColourGradient grad (juce::Colour (Palette::steelHighlight),
                               centre.x - radius, centre.y - radius,
                               juce::Colour (Palette::steelShadow),
                               centre.x + radius, centre.y + radius,
                               false);
    g.setGradientFill (grad);
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    // Inner circle (slightly darker)
    float inner = radius * 0.7f;
    g.setColour (juce::Colour (Palette::steelDark));
    g.fillEllipse (centre.x - inner, centre.y - inner, inner * 2.0f, inner * 2.0f);

    // Cross slot
    g.setColour (juce::Colour (Palette::steelShadow).darker (0.4f));
    float slotLen  = radius * 0.55f;
    float slotThick = juce::jmax (1.0f, radius * 0.15f);

    g.fillRect (juce::Rectangle<float> (centre.x - slotLen, centre.y - slotThick * 0.5f,
                                         slotLen * 2.0f, slotThick));
    g.fillRect (juce::Rectangle<float> (centre.x - slotThick * 0.5f, centre.y - slotLen,
                                         slotThick, slotLen * 2.0f));
}

//==============================================================================
// drawChromeRing
//==============================================================================
void TamuraLookAndFeel::drawChromeRing (juce::Graphics& g,
                                         juce::Point<float> centre,
                                         float outerRadius,
                                         float thickness)
{
    // Build a path for the ring (annulus)
    juce::Path ring;
    ring.addEllipse (centre.x - outerRadius, centre.y - outerRadius,
                     outerRadius * 2.0f, outerRadius * 2.0f);

    float innerRadius = outerRadius - thickness;
    juce::Path hole;
    hole.addEllipse (centre.x - innerRadius, centre.y - innerRadius,
                     innerRadius * 2.0f, innerRadius * 2.0f);

    ring.setUsingNonZeroWinding (false);
    ring.addPath (hole);

    // Chrome gradient: bright at top-left, dim at bottom-right (simulates lighting)
    juce::ColourGradient chromeGrad (juce::Colour (Palette::chromeBright),
                                     centre.x - outerRadius * 0.5f,
                                     centre.y - outerRadius * 0.5f,
                                     juce::Colour (Palette::chromeDim),
                                     centre.x + outerRadius * 0.5f,
                                     centre.y + outerRadius * 0.5f,
                                     false);
    chromeGrad.addColour (0.5, juce::Colour (Palette::chromeMid));
    g.setGradientFill (chromeGrad);
    g.fillPath (ring);

    // Specular highlight along the top-left edge of the ring
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    juce::Path highlight;
    float hlRadius = outerRadius - thickness * 0.3f;
    highlight.addCentredArc (centre.x, centre.y, hlRadius, hlRadius,
                             0.0f,
                             -juce::MathConstants<float>::pi * 0.75f,
                             -juce::MathConstants<float>::pi * 0.15f,
                             true);
    g.strokePath (highlight, juce::PathStrokeType (thickness * 0.5f));
}

//==============================================================================
// drawGlassDome
//==============================================================================
void TamuraLookAndFeel::drawGlassDome (juce::Graphics& g,
                                        juce::Point<float> centre,
                                        float radius)
{
    // Semi-transparent white gradient clipped to the top half of the circle
    // to simulate a convex glass dome catching overhead light.

    float domeRadius = radius * 0.88f;

    // Gradient from semi-transparent white at top to fully transparent at centre
    juce::ColourGradient domeGrad (juce::Colours::white.withAlpha (0.18f),
                                   centre.x, centre.y - domeRadius,
                                   juce::Colours::white.withAlpha (0.0f),
                                   centre.x, centre.y,
                                   false);

    // Clip to an ellipse (squashed vertically to top half only)
    juce::Path domePath;
    domePath.addEllipse (centre.x - domeRadius,
                         centre.y - domeRadius,
                         domeRadius * 2.0f,
                         domeRadius * 1.3f);

    g.saveState();
    g.reduceClipRegion (domePath);
    g.setGradientFill (domeGrad);
    g.fillEllipse (centre.x - domeRadius, centre.y - domeRadius,
                   domeRadius * 2.0f, domeRadius * 2.0f);
    g.restoreState();

    // Small specular "hot spot" arc near the top
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    juce::Path hotSpot;
    hotSpot.addCentredArc (centre.x, centre.y,
                           domeRadius * 0.6f, domeRadius * 0.6f,
                           0.0f,
                           -juce::MathConstants<float>::pi * 0.65f,
                           -juce::MathConstants<float>::pi * 0.35f,
                           true);
    g.strokePath (hotSpot, juce::PathStrokeType (2.0f,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
}

//==============================================================================
juce::Font TamuraLookAndFeel::getDefaultFont() const
{
    return juce::Font (juce::FontOptions().withHeight (15.0f));
}

} // namespace tamura
