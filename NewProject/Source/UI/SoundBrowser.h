#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../SoundLibrary.h"
#include "../SpinEngine.h"
#include <functional>
#include <vector>

namespace tamura
{

//==============================================================================
/**
    A scrollable grid-based browser for the user's unlocked sounds.

    Displays sound cards in a 2-column grid inside a juce::Viewport.
    Each card shows the sound name, category, and a tier-coloured indicator dot.
    Click to preview; double-click to select for export.

    Filter buttons at the top let the user narrow by category
    ("All", "Drums", "FX", etc.).
*/
class SoundBrowser : public juce::Component
{
public:
    SoundBrowser();
    ~SoundBrowser() override = default;

    //==========================================================================
    // Data
    //==========================================================================

    /** Replace the displayed sound list. */
    void setSounds (const std::vector<const tamura::SoundEntry*>& sounds);

    /** Filter displayed sounds by category. Empty string shows all. */
    void setFilter (const juce::String& category);

    //==========================================================================
    // Callbacks
    //==========================================================================

    /** Fired when the user single-clicks a sound card (preview). */
    std::function<void (const juce::String& soundId)> onPreview;

    /** Fired when the user double-clicks a sound card (select / export). */
    std::function<void (const juce::String& soundId)> onSelect;

    //==========================================================================
    // Component overrides
    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==========================================================================
    // Internal types
    //==========================================================================

    /** A single card in the grid. */
    class SoundCard : public juce::Component
    {
    public:
        SoundCard (const SoundEntry* entry, SoundBrowser& owner);
        ~SoundCard() override = default;

        const SoundEntry* getEntry() const noexcept { return soundEntry; }

        void paint (juce::Graphics& g) override;
        void mouseEnter (const juce::MouseEvent& event) override;
        void mouseExit (const juce::MouseEvent& event) override;
        void mouseUp (const juce::MouseEvent& event) override;
        void mouseDoubleClick (const juce::MouseEvent& event) override;

    private:
        static juce::Colour getTierColour (Tier tier);

        const SoundEntry* soundEntry = nullptr;
        SoundBrowser& browserOwner;
        bool hovered = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundCard)
    };

    /** The scrollable content component that hosts all SoundCards. */
    class GridContent : public juce::Component
    {
    public:
        GridContent() = default;
        ~GridContent() override = default;

        void paint (juce::Graphics& g) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GridContent)
    };

    /** A simple toggle-style filter button. */
    class FilterButton : public juce::TextButton
    {
    public:
        FilterButton (const juce::String& text);
        ~FilterButton() override = default;

        void paintButton (juce::Graphics& g,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterButton)
    };

    //==========================================================================
    // Internal helpers
    //==========================================================================
    void rebuildFilteredList();
    void layoutCards();
    void onFilterClicked (const juce::String& category);

    //==========================================================================
    // Layout constants
    //==========================================================================
    static constexpr int ColumnCount     = 2;
    static constexpr int CardHeight      = 40;
    static constexpr int CardSpacing     = 4;
    static constexpr int FilterBarHeight = 28;
    static constexpr int FilterSpacing   = 4;

    //==========================================================================
    // State
    //==========================================================================
    std::vector<const SoundEntry*> allSounds;
    std::vector<const SoundEntry*> filteredSounds;
    juce::String activeFilter;  // empty = show all

    //==========================================================================
    // Child components
    //==========================================================================
    juce::Viewport viewport;
    GridContent gridContent;
    juce::OwnedArray<SoundCard> cards;

    FilterButton filterAll    { "All" };
    FilterButton filterDrums  { "Drums" };
    FilterButton filterFX     { "FX" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBrowser)
};

} // namespace tamura
