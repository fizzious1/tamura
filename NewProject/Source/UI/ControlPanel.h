#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace tamura
{

//==============================================================================
// A horizontal strip of small, colored, metallic-beveled control buttons.
// Mimics the physical control buttons on slot-machine hardware.
//
// Buttons: [Preview] (green)  [Stop] (red)  [Save] (yellow)  [Library] (blue)
//
// All rendering is done via juce::Graphics — no external images.
//==============================================================================
class ControlPanel : public juce::Component
{
public:
    ControlPanel();
    ~ControlPanel() override = default;

    //--------------------------------------------------------------------------
    // Callbacks for each button action.
    std::function<void()> onPreview;
    std::function<void()> onStop;
    std::function<void()> onSave;
    std::function<void()> onLibrary;

    //--------------------------------------------------------------------------
    // juce::Component overrides
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==========================================================================
    // Internal helper: one small metallic button drawn entirely in paint().
    //==========================================================================
    struct PanelButton : public juce::Component
    {
        PanelButton (const juce::String& label, juce::Colour baseColour);
        ~PanelButton() override = default;

        std::function<void()> onClick;

        void paint (juce::Graphics& g) override;
        void mouseEnter (const juce::MouseEvent&) override;
        void mouseExit  (const juce::MouseEvent&) override;
        void mouseDown  (const juce::MouseEvent&) override;
        void mouseUp    (const juce::MouseEvent&) override;

    private:
        juce::String text;
        juce::Colour colour;
        bool hovering = false;
        bool pressed  = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelButton)
    };

    //--------------------------------------------------------------------------
    PanelButton previewButton;
    PanelButton stopButton;
    PanelButton saveButton;
    PanelButton libraryButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};

} // namespace tamura
