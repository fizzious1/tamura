#include "SideLever.h"
#include "TamuraLookAndFeel.h"

namespace tamura
{

//==============================================================================
// Construction
//==============================================================================

SideLever::SideLever()
{
    setInterceptsMouseClicks (true, false);
}

//==============================================================================
// Enable / Disable
//==============================================================================

void SideLever::setEnabled (bool shouldBeEnabled)
{
    if (enabled == shouldBeEnabled)
        return;

    enabled = shouldBeEnabled;
    repaint();
}

//==============================================================================
// Geometry helpers
//==============================================================================

float SideLever::getCentreX() const noexcept
{
    return getWidth() * 0.5f;
}

float SideLever::getBallRadius() const noexcept
{
    // Roughly 16 px at the reference width of 35 px, scales proportionally.
    return juce::jmax (6.0f, getWidth() * 0.46f);
}

float SideLever::getRestY() const noexcept
{
    // Top of the component + room for the ball radius + a small margin.
    return getBallRadius() + 4.0f;
}

float SideLever::getMaxPullY() const noexcept
{
    // Stop above the base plate region (bottom ~20 % of the component).
    return getHeight() * 0.72f;
}

float SideLever::getHandleY (float position) const noexcept
{
    return getRestY() + position * (getMaxPullY() - getRestY());
}

//==============================================================================
// Paint
//==============================================================================

void SideLever::paint (juce::Graphics& g)
{
    const float handleY = getHandleY (leverPosition);

    drawGuideTrack (g);
    drawBasePlate  (g);
    drawArmShaft   (g, handleY);
    drawBallHandle (g, handleY);

    // Dim overlay when disabled.
    if (! enabled)
    {
        g.setColour (juce::Colour (Palette::background).withAlpha (0.55f));
        g.fillRect (getLocalBounds().toFloat());
    }
}

//==============================================================================
// Paint helpers
//==============================================================================

void SideLever::drawGuideTrack (juce::Graphics& g) const
{
    // A thin dark vertical slot showing the lever's travel range.
    const float cx        = getCentreX();
    const float trackW    = juce::jmax (3.0f, getWidth() * 0.10f);
    const float topY      = getRestY();
    const float bottomY   = getMaxPullY();

    juce::Rectangle<float> trackRect (cx - trackW * 0.5f, topY,
                                       trackW, bottomY - topY);

    // Outer dark inset.
    g.setColour (juce::Colour (0xFF101018));
    g.fillRoundedRectangle (trackRect, trackW * 0.5f);

    // Subtle inner shadow on left edge.
    g.setColour (juce::Colour (0xFF000008));
    g.fillRoundedRectangle (trackRect.removeFromLeft (trackW * 0.35f), trackW * 0.5f);
}

void SideLever::drawBasePlate (juce::Graphics& g) const
{
    // A small metallic rectangle at the bottom with a pivot circle.
    const float cx     = getCentreX();
    const float plateH = juce::jmax (18.0f, getHeight() * 0.10f);
    const float plateW = juce::jmax (26.0f, getWidth() * 0.80f);
    const float plateY = getHeight() - plateH - 4.0f;

    juce::Rectangle<float> plateBounds (cx - plateW * 0.5f, plateY,
                                         plateW, plateH);

    TamuraLookAndFeel::drawMetallicSurface (g, plateBounds, 2.0f);

    // Pivot circle in the centre of the plate.
    const float pivotR = juce::jmax (3.0f, plateH * 0.22f);
    const juce::Point<float> pivotCentre (cx, plateY + plateH * 0.5f);

    g.setColour (juce::Colour (Palette::steelShadow));
    g.fillEllipse (pivotCentre.x - pivotR, pivotCentre.y - pivotR,
                   pivotR * 2.0f, pivotR * 2.0f);

    g.setColour (juce::Colour (Palette::steelHighlight).withAlpha (0.6f));
    g.drawEllipse (pivotCentre.x - pivotR, pivotCentre.y - pivotR,
                   pivotR * 2.0f, pivotR * 2.0f, 1.0f);
}

void SideLever::drawArmShaft (juce::Graphics& g, float handleCentreY) const
{
    const float cx       = getCentreX();
    const float shaftW   = juce::jmax (6.0f, getWidth() * 0.22f);
    const float shaftTop = handleCentreY + getBallRadius() * 0.5f;

    // Bottom of shaft: top of the base plate region.
    const float plateH    = juce::jmax (18.0f, getHeight() * 0.10f);
    const float shaftBot  = getHeight() - plateH - 4.0f;

    if (shaftBot <= shaftTop)
        return;

    juce::Rectangle<float> shaftRect (cx - shaftW * 0.5f, shaftTop,
                                       shaftW, shaftBot - shaftTop);

    // Chrome gradient left-to-right.
    juce::ColourGradient chromeGrad (juce::Colour (Palette::steelLight), shaftRect.getX(), 0.0f,
                                     juce::Colour (Palette::steelDark),  shaftRect.getRight(), 0.0f,
                                     false);
    g.setGradientFill (chromeGrad);
    g.fillRoundedRectangle (shaftRect, shaftW * 0.25f);

    // Thin specular highlight line running down the left third.
    const float hlX = cx - shaftW * 0.2f;

    g.setColour (juce::Colour (Palette::steelHighlight).withAlpha (0.55f));
    g.drawLine (hlX, shaftTop + 2.0f, hlX, shaftBot - 2.0f, 1.0f);

    // Subtle dark outline.
    g.setColour (juce::Colour (Palette::steelShadow).withAlpha (0.6f));
    g.drawRoundedRectangle (shaftRect, shaftW * 0.25f, 0.75f);
}

void SideLever::drawBallHandle (juce::Graphics& g, float handleCentreY) const
{
    const float cx = getCentreX();
    const float r  = getBallRadius();

    // --- Main red sphere: radial gradient bright to dark. ----------------
    const juce::Point<float> centre (cx, handleCentreY);
    const juce::Point<float> highlight (cx - r * 0.35f, handleCentreY - r * 0.35f);

    juce::ColourGradient ballGrad (juce::Colour (0xFFDD3333), highlight.x, highlight.y,
                                   juce::Colour (0xFF881111), cx + r * 0.5f, handleCentreY + r * 0.5f,
                                   true);  // radial
    g.setGradientFill (ballGrad);
    g.fillEllipse (cx - r, handleCentreY - r, r * 2.0f, r * 2.0f);

    // --- Thin dark rim around the sphere. --------------------------------
    g.setColour (juce::Colour (0xFF551111).withAlpha (0.8f));
    g.drawEllipse (cx - r, handleCentreY - r, r * 2.0f, r * 2.0f, 1.0f);

    // --- Small white specular highlight dot (upper-left quadrant). -------
    const float specR = juce::jmax (2.0f, r * 0.22f);
    const float specX = cx - r * 0.32f;
    const float specY = handleCentreY - r * 0.32f;

    juce::ColourGradient specGrad (juce::Colour (0xCCFFFFFF), specX, specY,
                                   juce::Colour (0x00FFFFFF), specX + specR, specY + specR,
                                   true);
    g.setGradientFill (specGrad);
    g.fillEllipse (specX - specR, specY - specR, specR * 2.0f, specR * 2.0f);
}

//==============================================================================
// Mouse interaction
//==============================================================================

void SideLever::mouseDown (const juce::MouseEvent& e)
{
    if (! enabled || isAnimatingBack)
        return;

    const float handleY = getHandleY (leverPosition);
    const float r       = getBallRadius();

    // Only begin drag if the click is within the ball area.
    const juce::Point<float> handleCentre (getCentreX(), handleY);
    if (e.position.getDistanceFrom (handleCentre) > r * 1.4f)
        return;

    isDragging        = true;
    dragAnchorOffsetY = e.position.y - handleY;
    repaint();
}

void SideLever::mouseDrag (const juce::MouseEvent& e)
{
    if (! isDragging)
        return;

    // Map mouse Y to leverPosition 0..1.
    const float desiredY = e.position.y - dragAnchorOffsetY;
    const float restY    = getRestY();
    const float maxY     = getMaxPullY();

    leverPosition = juce::jlimit (0.0f, 1.0f,
                                  (desiredY - restY) / (maxY - restY));
    repaint();
}

void SideLever::mouseUp (const juce::MouseEvent&)
{
    if (! isDragging)
        return;

    isDragging = false;

    // If pulled past 40 % threshold, fire callback.
    if (leverPosition > 0.4f && onPull)
        onPull();

    // Begin spring-back animation regardless of pull distance.
    if (leverPosition > 0.0f)
    {
        isAnimatingBack = true;
        startTimerHz (30);  // ~30 fps
    }
}

//==============================================================================
// Spring-back animation
//==============================================================================

void SideLever::timerCallback()
{
    // Exponential ease-out: multiply by decay factor each frame.
    leverPosition *= 0.78f;

    // Snap to zero once close enough.
    if (leverPosition < 0.02f)
    {
        leverPosition   = 0.0f;
        isAnimatingBack = false;
        stopTimer();
    }

    repaint();
}

} // namespace tamura
