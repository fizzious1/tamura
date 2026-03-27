#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tamura
{

//==============================================================================
// A large circular arcade-style button for triggering spins.
// Renders as a 3D physical red button with gradient shading, highlight arcs,
// and a drop shadow — all painted via juce::Graphics (no external images).
//==============================================================================
class SpinButton : public juce::Component
{
public:
    SpinButton();
    ~SpinButton() override = default;

    //--------------------------------------------------------------------------
    // Callback fired when the button is clicked.
    std::function<void()> onClick;

    //--------------------------------------------------------------------------
    // Enable / disable the button (disabled = gray, unclickable).
    void setEnabled (bool shouldBeEnabled);
    bool isEnabled() const noexcept { return enabled; }

    //--------------------------------------------------------------------------
    // Set the token cost string displayed beneath "SPIN" (e.g. "(1)").
    void setTokenCostText (const juce::String& text);

    //--------------------------------------------------------------------------
    // juce::Component overrides
    void paint (juce::Graphics& g) override;
    void resized() override {}

    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit  (const juce::MouseEvent& e) override;
    void mouseDown  (const juce::MouseEvent& e) override;
    void mouseUp    (const juce::MouseEvent& e) override;

private:
    bool enabled  = true;
    bool hovering = false;
    bool pressed  = false;

    juce::String costText { "(1)" };

    // Helpers
    juce::Rectangle<float> getButtonCircleBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpinButton)
};

} // namespace tamura
