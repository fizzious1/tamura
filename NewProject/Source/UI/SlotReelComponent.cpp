#include "SlotReelComponent.h"
#include "TamuraLookAndFeel.h"

namespace tamura
{

//==============================================================================
SlotReelComponent::SlotReelComponent()
    : rng (juce::Random::getSystemRandom())
{
    setOpaque (false);
}

//==============================================================================
// Spin control
//==============================================================================
void SlotReelComponent::startSpinning()
{
    spinning         = true;
    decelerating     = false;
    delayRemaining   = 0;
    decelerationStep = 0;
    currentIntervalMs = 33; // ~30 fps

    startTimer (currentIntervalMs);
}

void SlotReelComponent::stopSpinning (ReelSymbol finalSymbol, int delayMs)
{
    targetSymbol = finalSymbol;

    if (delayMs <= 0)
    {
        // Immediate stop — snap to the target symbol now
        currentSymbol = finalSymbol;
        spinning      = false;
        decelerating  = false;
        stopTimer();
        repaint();

        if (onSpinComplete)
            onSpinComplete();
        return;
    }

    delayRemaining = delayMs;

    // We will begin deceleration after delayRemaining has elapsed.
    // The actual deceleration uses 8 steps of increasing interval.
    decelerationSteps = 8;
    decelerationStep  = 0;
    decelerating      = false; // will become true once delay expires
}

void SlotReelComponent::setSymbol (ReelSymbol symbol)
{
    currentSymbol = symbol;
    repaint();
}

//==============================================================================
// Timer
//==============================================================================
void SlotReelComponent::timerCallback()
{
    if (! spinning)
    {
        stopTimer();
        return;
    }

    // --- Handle pre-deceleration delay -----------------------------------
    if (delayRemaining > 0)
    {
        delayRemaining -= currentIntervalMs;

        if (delayRemaining <= 0)
        {
            delayRemaining = 0;
            decelerating   = true;
            decelerationStep = 0;
        }

        // Continue showing random symbols while in the delay phase
        currentSymbol = randomSymbol();
        repaint();
        return;
    }

    // --- Deceleration phase ----------------------------------------------
    if (decelerating)
    {
        ++decelerationStep;

        if (decelerationStep >= decelerationSteps)
        {
            // Final stop -- land on the target symbol
            currentSymbol = targetSymbol;
            spinning      = false;
            decelerating  = false;
            stopTimer();
            repaint();

            if (onSpinComplete)
                onSpinComplete();
            return;
        }

        // Slow down: increase interval with each step (exponential-ish curve)
        float t = (float) decelerationStep / (float) decelerationSteps; // 0..1
        currentIntervalMs = 33 + (int) (t * t * 250.0f);               // 33ms -> ~283ms
        startTimer (currentIntervalMs);

        // On the last few deceleration frames, start showing the target symbol
        // intermittently to create a "landing" feel.
        if (decelerationStep >= decelerationSteps - 2)
            currentSymbol = targetSymbol;
        else
            currentSymbol = randomSymbol();

        repaint();
        return;
    }

    // --- Full-speed spinning ---------------------------------------------
    currentSymbol = randomSymbol();
    repaint();
}

//==============================================================================
// Paint
//==============================================================================
void SlotReelComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto side   = juce::jmin (bounds.getWidth(), bounds.getHeight());
    auto centre = bounds.getCentre();

    float outerRadius = side * 0.48f;
    float ringThickness = juce::jmax (3.0f, side * 0.045f);

    //----------------------------------------------------------------------
    // 1. Outer neon glow (subtle purple aura behind the chrome ring)
    //----------------------------------------------------------------------
    {
        const int glowPasses = 5;
        for (int i = glowPasses; i >= 1; --i)
        {
            float expand = (float) i * 2.0f;
            float alpha  = 0.06f * (1.0f - (float) i / (float) (glowPasses + 1));
            g.setColour (juce::Colour (Palette::neonPurple).withAlpha (alpha));
            g.drawEllipse (centre.x - outerRadius - expand,
                           centre.y - outerRadius - expand,
                           (outerRadius + expand) * 2.0f,
                           (outerRadius + expand) * 2.0f,
                           2.0f);
        }
    }

    //----------------------------------------------------------------------
    // 2. Dark interior disc
    //----------------------------------------------------------------------
    float innerRadius = outerRadius - ringThickness;
    {
        // Radial-ish dark fill (simulated with a vertical gradient from
        // slightly lighter at centre to darker at edges)
        juce::ColourGradient interiorGrad (
            juce::Colour (Palette::surfaceMid),
            centre.x, centre.y,
            juce::Colour (Palette::surfaceDark).darker (0.3f),
            centre.x, centre.y + innerRadius,
            true /* radial */);
        g.setGradientFill (interiorGrad);
        g.fillEllipse (centre.x - innerRadius, centre.y - innerRadius,
                       innerRadius * 2.0f, innerRadius * 2.0f);
    }

    //----------------------------------------------------------------------
    // 3. Chrome ring frame
    //----------------------------------------------------------------------
    TamuraLookAndFeel::drawChromeRing (g, centre, outerRadius, ringThickness);

    //----------------------------------------------------------------------
    // 4. Symbol text
    //----------------------------------------------------------------------
    {
        juce::String displayText = getSymbolDisplayText (currentSymbol);

        float fontSize = innerRadius * 0.65f;
        auto font = juce::Font (juce::FontOptions().withHeight (fontSize).withStyle ("Bold"));
        g.setFont (font);

        // Symbol colour with neon-ish tint
        juce::Colour symColour = getSymbolColour (currentSymbol);

        // Text shadow / glow
        g.setColour (symColour.withAlpha (0.25f));
        auto textRect = juce::Rectangle<float> (centre.x - innerRadius,
                                                 centre.y - innerRadius,
                                                 innerRadius * 2.0f,
                                                 innerRadius * 2.0f);
        g.drawText (displayText, textRect.translated (1.0f, 1.0f),
                    juce::Justification::centred, false);

        // Main text
        g.setColour (symColour);
        g.drawText (displayText, textRect, juce::Justification::centred, false);
    }

    //----------------------------------------------------------------------
    // 5. Symbol name subtitle (small text below the main symbol)
    //----------------------------------------------------------------------
    {
        juce::String nameText = symbolToString (currentSymbol);

        float subFontSize = juce::jmax (9.0f, innerRadius * 0.22f);
        auto subFont = juce::Font (juce::FontOptions().withHeight (subFontSize));
        g.setFont (subFont);
        g.setColour (juce::Colour (Palette::textDim));

        auto nameRect = juce::Rectangle<float> (
            centre.x - innerRadius * 0.8f,
            centre.y + innerRadius * 0.35f,
            innerRadius * 1.6f,
            subFontSize * 1.5f);
        g.drawText (nameText, nameRect, juce::Justification::centred, false);
    }

    //----------------------------------------------------------------------
    // 6. Glass dome highlight (on top of everything)
    //----------------------------------------------------------------------
    TamuraLookAndFeel::drawGlassDome (g, centre, innerRadius);

    //----------------------------------------------------------------------
    // 7. Spin blur indicator (when spinning, draw motion lines)
    //----------------------------------------------------------------------
    if (spinning && ! decelerating)
    {
        g.setColour (juce::Colours::white.withAlpha (0.04f));
        for (int i = 0; i < 3; ++i)
        {
            float yOff = (float) (i - 1) * innerRadius * 0.35f;
            g.drawLine (centre.x - innerRadius * 0.5f,
                        centre.y + yOff,
                        centre.x + innerRadius * 0.5f,
                        centre.y + yOff,
                        1.0f);
        }
    }
}

//==============================================================================
void SlotReelComponent::resized()
{
    // No child components to layout; painting is entirely dynamic.
}

//==============================================================================
// Helpers
//==============================================================================
ReelSymbol SlotReelComponent::randomSymbol()
{
    return static_cast<ReelSymbol> (rng.nextInt (6));
}

} // namespace tamura
