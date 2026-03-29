#include "SoundBrowser.h"
#include "TamuraLookAndFeel.h"

namespace tamura
{

//==============================================================================
// Tier colour mapping (as specified in design)
//==============================================================================
static juce::Colour tierColourFor (Tier tier)
{
    switch (tier)
    {
        case Tier::Common:    return juce::Colour (0xFFF0E6D2);
        case Tier::Uncommon:  return juce::Colour (0xFF22AA44);
        case Tier::Rare:      return juce::Colour (0xFF4169E1);
        case Tier::Epic:      return juce::Colour (0xFF9370DB);
        case Tier::Legendary: return juce::Colour (0xFFFFD700);
        default:              return juce::Colour (0xFFF0E6D2);
    }
}

//==============================================================================
// SoundBrowser
//==============================================================================
SoundBrowser::SoundBrowser()
{
    setOpaque (true);

    // Viewport setup
    viewport.setViewedComponent (&gridContent, false /* don't own it */);
    viewport.setScrollBarsShown (true, false);
    viewport.getVerticalScrollBar().setColour (
        juce::ScrollBar::thumbColourId, juce::Colour (Palette::steelLight));
    viewport.getVerticalScrollBar().setColour (
        juce::ScrollBar::trackColourId, juce::Colour (Palette::surfaceDark));
    addAndMakeVisible (viewport);

    // Filter buttons
    auto setupFilter = [this] (FilterButton& btn, const juce::String& category)
    {
        addAndMakeVisible (btn);
        btn.onClick = [this, category] { onFilterClicked (category); };
    };

    setupFilter (filterAll,   "");
    setupFilter (filterDrums, "Drums");
    setupFilter (filterFX,    "FX");

    // "All" is active by default
    filterAll.setToggleState (true, juce::dontSendNotification);
}

//==============================================================================
void SoundBrowser::setSounds (const std::vector<const tamura::SoundEntry*>& sounds)
{
    allSounds = sounds;
    rebuildFilteredList();
}

void SoundBrowser::setFilter (const juce::String& category)
{
    activeFilter = category;

    filterAll.setToggleState   (category.isEmpty(),         juce::dontSendNotification);
    filterDrums.setToggleState (category == "Drums",        juce::dontSendNotification);
    filterFX.setToggleState    (category == "FX",           juce::dontSendNotification);

    rebuildFilteredList();
}

//==============================================================================
void SoundBrowser::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (Palette::background));
}

void SoundBrowser::resized()
{
    auto area = getLocalBounds();

    //------------------------------------------------------------------
    // Filter bar at the top
    //------------------------------------------------------------------
    auto filterBar = area.removeFromTop (FilterBarHeight);
    int filterWidth = 50;

    filterAll.setBounds   (filterBar.removeFromLeft (filterWidth));
    filterBar.removeFromLeft (FilterSpacing);
    filterDrums.setBounds (filterBar.removeFromLeft (filterWidth));
    filterBar.removeFromLeft (FilterSpacing);
    filterFX.setBounds    (filterBar.removeFromLeft (filterWidth));

    //------------------------------------------------------------------
    // Viewport fills the rest
    //------------------------------------------------------------------
    area.removeFromTop (CardSpacing);
    viewport.setBounds (area);

    layoutCards();
}

//==============================================================================
// Private helpers
//==============================================================================
void SoundBrowser::rebuildFilteredList()
{
    filteredSounds.clear();

    for (auto* entry : allSounds)
    {
        if (entry == nullptr)
            continue;

        if (activeFilter.isEmpty() || entry->category == activeFilter)
            filteredSounds.push_back (entry);
    }

    // Rebuild card components
    cards.clear();

    for (auto* entry : filteredSounds)
    {
        auto* card = cards.add (new SoundCard (entry, *this));
        gridContent.addAndMakeVisible (card);
    }

    layoutCards();
    repaint();
}

void SoundBrowser::layoutCards()
{
    if (cards.isEmpty())
    {
        gridContent.setSize (viewport.getWidth(), 0);
        return;
    }

    int viewWidth = viewport.getWidth() - viewport.getScrollBarThickness();
    if (viewWidth <= 0)
        viewWidth = viewport.getWidth();

    int cardWidth = (viewWidth - (ColumnCount - 1) * CardSpacing) / ColumnCount;
    int numRows   = (cards.size() + ColumnCount - 1) / ColumnCount;
    int totalHeight = numRows * CardHeight + (numRows - 1) * CardSpacing;

    gridContent.setSize (viewWidth, totalHeight);

    for (int i = 0; i < cards.size(); ++i)
    {
        int col = i % ColumnCount;
        int row = i / ColumnCount;
        int x   = col * (cardWidth + CardSpacing);
        int y   = row * (CardHeight + CardSpacing);

        cards[i]->setBounds (x, y, cardWidth, CardHeight);
    }
}

void SoundBrowser::onFilterClicked (const juce::String& category)
{
    setFilter (category);
}

//==============================================================================
// GridContent
//==============================================================================
void SoundBrowser::GridContent::paint (juce::Graphics& /* g */)
{
    // Cards draw themselves; nothing additional needed here.
}

//==============================================================================
// SoundCard
//==============================================================================
SoundBrowser::SoundCard::SoundCard (const SoundEntry* entry, SoundBrowser& owner)
    : soundEntry (entry),
      browserOwner (owner)
{
    setRepaintsOnMouseActivity (true);
}

void SoundBrowser::SoundCard::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    //------------------------------------------------------------------
    // Card background
    //------------------------------------------------------------------
    juce::Colour bgColour = juce::Colour (Palette::surfaceDark);
    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, 4.0f);

    //------------------------------------------------------------------
    // Border (brighter on hover)
    //------------------------------------------------------------------
    juce::Colour borderColour = hovered
        ? juce::Colour (Palette::steelLight)
        : juce::Colour (0xFF303038);
    g.setColour (borderColour);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);

    if (soundEntry == nullptr)
        return;

    //------------------------------------------------------------------
    // Tier indicator dot
    //------------------------------------------------------------------
    float dotRadius = 4.0f;
    float dotX = bounds.getX() + 10.0f;
    float dotY = bounds.getCentreY();

    g.setColour (getTierColour (soundEntry->tier));
    g.fillEllipse (dotX - dotRadius, dotY - dotRadius,
                   dotRadius * 2.0f, dotRadius * 2.0f);

    //------------------------------------------------------------------
    // Sound name
    //------------------------------------------------------------------
    float textLeft = dotX + dotRadius + 8.0f;
    float textRight = bounds.getRight() - 6.0f;
    float textWidth = textRight - textLeft;

    auto nameRect = juce::Rectangle<float> (textLeft, bounds.getY() + 4.0f,
                                            textWidth, 18.0f);
    g.setColour (juce::Colour (Palette::textBright));
    g.setFont (juce::Font (juce::FontOptions().withHeight (13.0f)));
    g.drawText (soundEntry->name, nameRect, juce::Justification::centredLeft, true);

    //------------------------------------------------------------------
    // Category label (small, grey)
    //------------------------------------------------------------------
    auto categoryRect = juce::Rectangle<float> (textLeft, bounds.getY() + 21.0f,
                                                textWidth, 14.0f);
    g.setColour (juce::Colour (Palette::textDim));
    g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));

    juce::String label = soundEntry->category + " / " + tierToString (soundEntry->tier);
    g.drawText (label, categoryRect, juce::Justification::centredLeft, true);
}

void SoundBrowser::SoundCard::mouseEnter (const juce::MouseEvent& /* event */)
{
    hovered = true;
    repaint();
}

void SoundBrowser::SoundCard::mouseExit (const juce::MouseEvent& /* event */)
{
    hovered = false;
    repaint();
}

void SoundBrowser::SoundCard::mouseUp (const juce::MouseEvent& event)
{
    if (soundEntry != nullptr && event.mouseWasClicked() && browserOwner.onPreview)
        browserOwner.onPreview (soundEntry->id);
}

void SoundBrowser::SoundCard::mouseDoubleClick (const juce::MouseEvent& /* event */)
{
    if (soundEntry != nullptr && browserOwner.onSelect)
        browserOwner.onSelect (soundEntry->id);
}

juce::Colour SoundBrowser::SoundCard::getTierColour (Tier tier)
{
    return tierColourFor (tier);
}

//==============================================================================
// FilterButton
//==============================================================================
SoundBrowser::FilterButton::FilterButton (const juce::String& text)
    : juce::TextButton (text)
{
    setClickingTogglesState (true);
}

void SoundBrowser::FilterButton::paintButton (juce::Graphics& g,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool /* shouldDrawButtonAsDown */)
{
    auto bounds = getLocalBounds().toFloat();

    bool active = getToggleState();

    //------------------------------------------------------------------
    // Background
    //------------------------------------------------------------------
    juce::Colour bg = active
        ? juce::Colour (Palette::surfaceMid)
        : juce::Colour (Palette::surfaceDark);

    if (shouldDrawButtonAsHighlighted && ! active)
        bg = bg.brighter (0.08f);

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 3.0f);

    //------------------------------------------------------------------
    // Border
    //------------------------------------------------------------------
    juce::Colour border = active
        ? juce::Colour (Palette::neonPurple)
        : juce::Colour (0xFF303038);
    g.setColour (border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 1.0f);

    //------------------------------------------------------------------
    // Text
    //------------------------------------------------------------------
    juce::Colour textColour = active
        ? juce::Colour (Palette::textBright)
        : juce::Colour (Palette::textDim);
    g.setColour (textColour);
    g.setFont (juce::Font (juce::FontOptions().withHeight (12.0f)));
    g.drawText (getButtonText(), bounds, juce::Justification::centred, false);
}

} // namespace tamura
