#include "ControlPanel.h"

namespace tamura
{

//==============================================================================
// PanelButton implementation
//==============================================================================

ControlPanel::PanelButton::PanelButton (const juce::String& label,
                                         juce::Colour baseColour)
    : text (label), colour (baseColour)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void ControlPanel::PanelButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    //------------------------------------------------------------------
    // Determine face colour based on interaction state
    //------------------------------------------------------------------
    juce::Colour faceColour = colour;

    if (pressed)
        faceColour = colour.darker (0.35f);
    else if (hovering)
        faceColour = colour.brighter (0.20f);

    //------------------------------------------------------------------
    // 1. Beveled outer edge — lighter top-left, darker bottom-right
    //------------------------------------------------------------------
    {
        // Light bevel (top + left)
        g.setColour (faceColour.brighter (0.45f));
        g.drawLine (bounds.getX(), bounds.getY(),
                    bounds.getRight(), bounds.getY(), 1.5f);          // top
        g.drawLine (bounds.getX(), bounds.getY(),
                    bounds.getX(), bounds.getBottom(), 1.5f);         // left

        // Dark bevel (bottom + right)
        g.setColour (faceColour.darker (0.50f));
        g.drawLine (bounds.getX(), bounds.getBottom(),
                    bounds.getRight(), bounds.getBottom(), 1.5f);     // bottom
        g.drawLine (bounds.getRight(), bounds.getY(),
                    bounds.getRight(), bounds.getBottom(), 1.5f);     // right
    }

    //------------------------------------------------------------------
    // 2. Metallic gradient face
    //------------------------------------------------------------------
    {
        auto face = bounds.reduced (1.5f);

        juce::ColourGradient grad (faceColour.brighter (0.25f), face.getX(), face.getY(),
                                   faceColour.darker (0.20f), face.getX(), face.getBottom(),
                                   false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (face, 2.0f);
    }

    //------------------------------------------------------------------
    // 3. Subtle specular highlight stripe across the top third
    //------------------------------------------------------------------
    {
        auto face = bounds.reduced (2.0f);
        auto highlightRect = face.withHeight (face.getHeight() * 0.35f);

        juce::ColourGradient shine (juce::Colour (0x30FFFFFF), highlightRect.getX(), highlightRect.getY(),
                                    juce::Colour (0x00FFFFFF), highlightRect.getX(), highlightRect.getBottom(),
                                    false);
        g.setGradientFill (shine);
        g.fillRoundedRectangle (highlightRect, 1.5f);
    }

    //------------------------------------------------------------------
    // 4. Text label
    //------------------------------------------------------------------
    {
        // Dark text for yellow buttons for readability, white elsewhere
        bool useDarkText = (colour.getGreen() > 180 && colour.getRed() > 180);
        g.setColour (useDarkText ? juce::Colour (0xFF1A1410) : juce::Colours::white);

        auto textFont = juce::FontOptions (11.0f, juce::Font::bold);
        g.setFont (textFont);
        g.drawText (text, bounds, juce::Justification::centred, false);
    }
}

//----------------------------------------------------------------------
void ControlPanel::PanelButton::mouseEnter (const juce::MouseEvent&)
{
    hovering = true;
    repaint();
}

void ControlPanel::PanelButton::mouseExit (const juce::MouseEvent&)
{
    hovering = false;
    pressed  = false;
    repaint();
}

void ControlPanel::PanelButton::mouseDown (const juce::MouseEvent&)
{
    pressed = true;
    repaint();
}

void ControlPanel::PanelButton::mouseUp (const juce::MouseEvent& e)
{
    if (pressed)
    {
        pressed = false;
        repaint();

        if (getLocalBounds().toFloat().contains (e.position))
        {
            if (onClick)
                onClick();
        }
    }
}

//==============================================================================
// ControlPanel implementation
//==============================================================================

ControlPanel::ControlPanel()
    : previewButton ("Preview", juce::Colour (0xFF22AA44)),   // green
      stopButton    ("Stop",    juce::Colour (0xFFCC2222)),   // red
      saveButton    ("Save",    juce::Colour (0xFFCCAA22)),   // yellow / gold
      libraryButton ("Library", juce::Colour (0xFF2266CC))    // blue
{
    addAndMakeVisible (previewButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (libraryButton);

    // Wire internal button clicks to the public callbacks
    previewButton.onClick = [this] { if (onPreview) onPreview(); };
    stopButton.onClick    = [this] { if (onStop)    onStop();    };
    saveButton.onClick    = [this] { if (onSave)    onSave();    };
    libraryButton.onClick = [this] { if (onLibrary) onLibrary(); };
}

//==============================================================================
void ControlPanel::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    //------------------------------------------------------------------
    // Metallic strip background — brushed-metal look
    //------------------------------------------------------------------
    {
        juce::ColourGradient stripGrad (juce::Colour (0xFF3A3A3A), area.getX(), area.getY(),
                                        juce::Colour (0xFF282828), area.getX(), area.getBottom(),
                                        false);
        g.setGradientFill (stripGrad);
        g.fillRoundedRectangle (area, 3.0f);
    }

    // Subtle top highlight line (simulates brushed metal reflection)
    {
        g.setColour (juce::Colour (0x18FFFFFF));
        g.drawHorizontalLine (static_cast<int> (area.getY() + 1),
                              area.getX() + 4.0f, area.getRight() - 4.0f);
    }

    // Bottom shadow line
    {
        g.setColour (juce::Colour (0x30000000));
        g.drawHorizontalLine (static_cast<int> (area.getBottom() - 1),
                              area.getX() + 4.0f, area.getRight() - 4.0f);
    }

    // Thin metallic border
    {
        g.setColour (juce::Colour (0xFF4A4A4A));
        g.drawRoundedRectangle (area.reduced (0.5f), 3.0f, 1.0f);
    }
}

//==============================================================================
void ControlPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);

    const int buttonWidth  = 60;
    const int buttonHeight = juce::jmin (area.getHeight(), 30);
    const int spacing      = 8;

    // Total width of all buttons + gaps
    int totalWidth = 4 * buttonWidth + 3 * spacing;

    // Center the row horizontally
    int startX = area.getX() + (area.getWidth() - totalWidth) / 2;
    int y      = area.getY() + (area.getHeight() - buttonHeight) / 2;

    previewButton.setBounds (startX, y, buttonWidth, buttonHeight);
    stopButton.setBounds    (startX + (buttonWidth + spacing),     y, buttonWidth, buttonHeight);
    saveButton.setBounds    (startX + 2 * (buttonWidth + spacing), y, buttonWidth, buttonHeight);
    libraryButton.setBounds (startX + 3 * (buttonWidth + spacing), y, buttonWidth, buttonHeight);
}

} // namespace tamura
