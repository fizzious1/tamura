#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../SpinEngine.h"
#include <array>
#include <vector>
#include <functional>

namespace tamura
{

//==============================================================================
/**
    A classic slot machine reel strip with 3 columns of symbols scrolling
    vertically behind a glass window.

    Each column shows 3 visible rows; the centre row is the active payline,
    highlighted with a golden tint and flanked by triangular payline arrows.

    Usage:
        reelStrip.startSpinning();
        // ... later, when the result is known:
        reelStrip.stopSpinning (spinResult.reels, 400);
        reelStrip.onSpinComplete = [this]() { handleAllReelsStopped(); };

    Animation runs at ~30 fps via juce::Timer.  Each column decelerates
    independently with a staggered delay to mimic a real machine.
*/
class ReelStripWindow : public juce::Component,
                        public juce::Timer
{
public:
    ReelStripWindow();
    ~ReelStripWindow() override = default;

    //==========================================================================
    // Spin control
    //==========================================================================

    /** Start all 3 columns spinning (rapid upward scroll). */
    void startSpinning();

    /** Schedule each column to stop and land on the given final symbols.
        @param finalSymbols  The 3 symbols to land in each column's centre row.
        @param baseDelayMs   Delay before column 0 begins decelerating.
                             Column 1 gets +300 ms, column 2 gets +600 ms.
    */
    void stopSpinning (const std::array<tamura::ReelSymbol, 3>& finalSymbols,
                       int baseDelayMs);

    /** Fired on the message thread once all 3 columns have come to rest. */
    std::function<void()> onSpinComplete;

    /** Fired when an individual column stops. Parameter is the column index (0, 1, 2). */
    std::function<void (int columnIndex)> onColumnStopped;

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
    // Per-column state
    //==========================================================================
    static constexpr int NumColumns       = 3;
    static constexpr int VisibleRows      = 3;     // rows shown in the window
    static constexpr int BufferSymbols    = 6;     // total symbols kept per strip
    static constexpr int TimerIntervalMs  = 33;    // ~30 fps
    static constexpr float DecayFactor    = 0.88f; // exponential speed decay
    static constexpr float SnapThreshold  = 1.2f;  // px/frame below which we snap
    static constexpr int StopDelayStepMs  = 300;   // inter-column stop stagger

    struct ColumnState
    {
        std::vector<ReelSymbol> symbols;  // symbol strip (BufferSymbols long)
        float scrollOffset  = 0.0f;       // current vertical pixel offset
        float scrollSpeed   = 0.0f;       // pixels per frame (positive = upward)
        bool  spinning      = false;
        bool  decelerating  = false;
        int   delayRemaining = 0;         // ms before deceleration starts
        ReelSymbol targetSymbol = ReelSymbol::Cherry;

        void fillRandom (juce::Random& rng);
    };

    std::array<ColumnState, NumColumns> columns;

    /** Number of columns that have finished stopping (0..3). */
    int stoppedCount = 0;

    /** Random number generator for symbol cycling. */
    juce::Random rng;

    //==========================================================================
    // Internal helpers
    //==========================================================================

    /** Generate a random ReelSymbol. */
    ReelSymbol randomSymbol();

    /** Compute the pixel height of one symbol cell given the current bounds. */
    float cellHeight() const;

    /** Return the rectangle for the inner reel area (inside the chrome bezel). */
    juce::Rectangle<float> innerReelBounds() const;

    /** Paint one column of symbols into the given strip area. */
    void paintColumn (juce::Graphics& g,
                      const ColumnState& col,
                      juce::Rectangle<float> stripBounds);

    /** Paint the centre-row payline highlight and arrows. */
    void paintPayline (juce::Graphics& g,
                       juce::Rectangle<float> reelBounds);

    /** Paint the glass reflection overlay. */
    void paintGlassOverlay (juce::Graphics& g,
                            juce::Rectangle<float> reelBounds);

    /** Paint the column separators. */
    void paintSeparators (juce::Graphics& g,
                          juce::Rectangle<float> reelBounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReelStripWindow)
};

} // namespace tamura
