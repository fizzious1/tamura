#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../SpinEngine.h"

namespace tamura
{

//==============================================================================
/**
    A single reel rendered as a circular chrome-framed gauge.

    Visual: chrome metallic ring  ->  dark interior  ->  symbol text centred
    with a glass dome highlight over the top.

    Animation: timer-based symbol cycling at ~30 fps, with deceleration on stop.
*/
class SlotReelComponent : public juce::Component,
                          public juce::Timer
{
public:
    SlotReelComponent();
    ~SlotReelComponent() override = default;

    //==========================================================================
    // Spin control
    //==========================================================================

    /** Begin spinning -- rapidly cycles through random symbols. */
    void startSpinning();

    /** Schedule a stop.
        The reel will continue spinning for @p delayMs milliseconds,
        then decelerate and land on @p finalSymbol.
    */
    void stopSpinning (ReelSymbol finalSymbol, int delayMs);

    /** Returns true while the reel is still animating (spinning or decelerating). */
    bool isSpinning() const noexcept { return spinning; }

    /** Immediately set the displayed symbol (no animation). */
    void setSymbol (ReelSymbol symbol);

    /** Get the currently displayed symbol. */
    ReelSymbol getSymbol() const noexcept { return currentSymbol; }

    //==========================================================================
    // Callbacks
    //==========================================================================

    /** Called on the message thread when the reel has fully stopped. */
    std::function<void()> onSpinComplete;

    //==========================================================================
    // Component overrides
    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==========================================================================
    // Timer override
    //==========================================================================
    void timerCallback() override;

    //==========================================================================
    // Internal helpers
    //==========================================================================

    /** Pick a random symbol for the spin animation. */
    ReelSymbol randomSymbol();

    //==========================================================================
    // State
    //==========================================================================
    ReelSymbol currentSymbol    = ReelSymbol::Cherry;
    ReelSymbol targetSymbol     = ReelSymbol::Cherry;

    bool  spinning              = false;
    bool  decelerating          = false;

    /** Milliseconds remaining before deceleration begins (after stopSpinning called). */
    int   delayRemaining        = 0;

    /** Deceleration phase: counts down from decelerationSteps to 0. */
    int   decelerationStep      = 0;
    int   decelerationSteps     = 0;

    /** Frame interval grows during deceleration to simulate slowing down. */
    int   currentIntervalMs     = 33;  // ~30 fps

    /** Random number generator for animation symbols. */
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlotReelComponent)
};

} // namespace tamura
