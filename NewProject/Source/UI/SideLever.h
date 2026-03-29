#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tamura
{

//==============================================================================
/**
    Classic slot-machine side lever (one-armed bandit handle).

    Visual (top to bottom):
      - Red sphere handle with specular highlight
      - Chrome arm shaft with thin highlight line
      - Dark guide track behind the arm
      - Metallic base mount plate with pivot circle

    Interaction:
      Drag the handle downward to pull.  Release past the 40 % threshold to
      trigger the onPull callback.  The lever then spring-animates back to
      its rest position.
*/
class SideLever : public juce::Component,
                  public juce::Timer
{
public:
    SideLever();
    ~SideLever() override = default;

    //==========================================================================
    // Callback -- fired when the lever is released after a sufficient pull.
    //==========================================================================
    std::function<void()> onPull;

    //==========================================================================
    // Enable / disable the lever.
    //==========================================================================
    void setEnabled (bool shouldBeEnabled);
    bool isEnabled() const noexcept { return enabled; }

    //==========================================================================
    // Component overrides
    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override {}

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

private:
    //==========================================================================
    // Timer override -- drives the spring-back animation (~30 fps).
    //==========================================================================
    void timerCallback() override;

    //==========================================================================
    // Internal paint helpers
    //==========================================================================
    void drawGuideTrack    (juce::Graphics& g) const;
    void drawBasePlate     (juce::Graphics& g) const;
    void drawArmShaft      (juce::Graphics& g, float handleCentreY) const;
    void drawBallHandle    (juce::Graphics& g, float handleCentreY) const;

    //==========================================================================
    // Geometry helpers
    //==========================================================================

    /** Centre X for the lever -- always the horizontal midpoint. */
    float getCentreX() const noexcept;

    /** Y position of the ball handle centre at the given leverPosition (0..1). */
    float getHandleY (float position) const noexcept;

    /** Radius of the red ball handle, derived from component width. */
    float getBallRadius() const noexcept;

    /** Top Y of the resting handle centre. */
    float getRestY() const noexcept;

    /** Bottom Y of the fully-pulled handle centre. */
    float getMaxPullY() const noexcept;

    //==========================================================================
    // State
    //==========================================================================
    float leverPosition    = 0.0f;   // 0 = rest, 1 = fully pulled
    bool  isDragging       = false;
    bool  isAnimatingBack  = false;
    bool  enabled          = true;

    /** Y offset in component-local coords where the user grabbed the handle. */
    float dragAnchorOffsetY = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideLever)
};

} // namespace tamura
