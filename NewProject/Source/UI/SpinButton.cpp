#include "SpinButton.h"

namespace tamura
{

//==============================================================================
SpinButton::SpinButton()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

//==============================================================================
void SpinButton::setEnabled (bool shouldBeEnabled)
{
    if (enabled != shouldBeEnabled)
    {
        enabled = shouldBeEnabled;

        if (! enabled)
        {
            hovering = false;
            pressed  = false;
            setMouseCursor (juce::MouseCursor::NormalCursor);
        }
        else
        {
            setMouseCursor (juce::MouseCursor::PointingHandCursor);
        }

        repaint();
    }
}

void SpinButton::setTokenCostText (const juce::String& text)
{
    costText = text;
    repaint();
}

//==============================================================================
juce::Rectangle<float> SpinButton::getButtonCircleBounds() const
{
    // Inset slightly so the shadow fits beneath
    auto area = getLocalBounds().toFloat().reduced (4.0f);
    // Keep it square/circular
    auto side = juce::jmin (area.getWidth(), area.getHeight() - 6.0f); // 6px room for shadow
    return juce::Rectangle<float> (side, side)
               .withCentre ({ area.getCentreX(), area.getCentreY() - 3.0f });
}

//==============================================================================
void SpinButton::paint (juce::Graphics& g)
{
    auto bounds = getButtonCircleBounds();
    auto cx = bounds.getCentreX();
    auto cy = bounds.getCentreY();
    auto radius = bounds.getWidth() * 0.5f;

    //----------------------------------------------------------------------
    // 1. Drop shadow (dark ellipse underneath)
    //----------------------------------------------------------------------
    {
        auto shadowBounds = bounds.translated (0.0f, 5.0f).expanded (2.0f, -2.0f);
        juce::ColourGradient shadowGrad (juce::Colour (0x80000000), shadowBounds.getCentreX(), shadowBounds.getCentreY(),
                                         juce::Colour (0x00000000), shadowBounds.getCentreX(), shadowBounds.getBottom(),
                                         true);
        g.setGradientFill (shadowGrad);
        g.fillEllipse (shadowBounds);
    }

    //----------------------------------------------------------------------
    // 2. Outer rim / bezel — gives the button a raised metallic ring
    //----------------------------------------------------------------------
    {
        auto rimBounds = bounds.expanded (2.0f);
        juce::ColourGradient rimGrad (juce::Colour (0xFF555555), rimBounds.getX(), rimBounds.getY(),
                                      juce::Colour (0xFF222222), rimBounds.getRight(), rimBounds.getBottom(),
                                      false);
        g.setGradientFill (rimGrad);
        g.fillEllipse (rimBounds);
    }

    //----------------------------------------------------------------------
    // 3. Main button face
    //----------------------------------------------------------------------
    {
        juce::Colour topColour, bottomColour;

        if (! enabled)
        {
            // Disabled — grey
            topColour    = juce::Colour (0xFF666666);
            bottomColour = juce::Colour (0xFF3A3A3A);
        }
        else if (pressed)
        {
            // Pressed — darker, gradient reversed to simulate being pushed in
            topColour    = juce::Colour (0xFF661111);
            bottomColour = juce::Colour (0xFFAA2222);
        }
        else if (hovering)
        {
            // Hover — brighter red
            topColour    = juce::Colour (0xFFDD3333);
            bottomColour = juce::Colour (0xFF991515);
        }
        else
        {
            // Default — rich red gradient
            topColour    = juce::Colour (0xFFCC2222);
            bottomColour = juce::Colour (0xFF881111);
        }

        juce::ColourGradient faceGrad (topColour, cx, bounds.getY(),
                                       bottomColour, cx, bounds.getBottom(),
                                       false);
        g.setGradientFill (faceGrad);
        g.fillEllipse (bounds);
    }

    //----------------------------------------------------------------------
    // 4. Specular highlight arc (top-left)
    //----------------------------------------------------------------------
    if (enabled)
    {
        float highlightRadius = radius * (pressed ? 0.35f : 0.55f);
        float hx = cx - radius * (pressed ? 0.15f : 0.25f);
        float hy = cy - radius * (pressed ? 0.15f : 0.30f);

        juce::ColourGradient highlightGrad (
            juce::Colour (pressed ? 0x30FFFFFF : 0x55FFFFFF), hx, hy,
            juce::Colour (0x00FFFFFF), hx + highlightRadius, hy + highlightRadius,
            true);
        g.setGradientFill (highlightGrad);
        g.fillEllipse (hx - highlightRadius, hy - highlightRadius,
                       highlightRadius * 2.0f, highlightRadius * 2.0f);
    }

    //----------------------------------------------------------------------
    // 5. Subtle inner rim (dark outline around the face)
    //----------------------------------------------------------------------
    {
        g.setColour (juce::Colour (enabled ? 0x60000000 : 0x40000000));
        g.drawEllipse (bounds.reduced (1.0f), 1.5f);
    }

    //----------------------------------------------------------------------
    // 6. Text — "SPIN" and token cost
    //----------------------------------------------------------------------
    {
        g.setColour (juce::Colours::white.withAlpha (enabled ? 1.0f : 0.45f));

        // "SPIN" — bold, centered
        auto spinFont = juce::FontOptions (radius * 0.52f, juce::Font::bold);
        g.setFont (spinFont);
        g.drawText ("SPIN", bounds.withTrimmedBottom (radius * 0.28f),
                    juce::Justification::centred, false);

        // Cost text below, smaller
        auto costFont = juce::FontOptions (radius * 0.30f);
        g.setFont (costFont);
        g.drawText (costText, bounds.withTrimmedTop (radius * 0.55f),
                    juce::Justification::centred, false);
    }
}

//==============================================================================
void SpinButton::mouseEnter (const juce::MouseEvent&)
{
    if (enabled)
    {
        hovering = true;
        repaint();
    }
}

void SpinButton::mouseExit (const juce::MouseEvent&)
{
    hovering = false;
    pressed  = false;
    repaint();
}

void SpinButton::mouseDown (const juce::MouseEvent&)
{
    if (enabled)
    {
        pressed = true;
        repaint();
    }
}

void SpinButton::mouseUp (const juce::MouseEvent& e)
{
    if (pressed && enabled)
    {
        pressed = false;
        repaint();

        // Only trigger if mouse is still inside the button circle
        if (getButtonCircleBounds().contains (e.position))
        {
            if (onClick)
                onClick();
        }
    }
}

} // namespace tamura
