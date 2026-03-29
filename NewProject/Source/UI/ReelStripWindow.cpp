#include "ReelStripWindow.h"
#include "TamuraLookAndFeel.h"

namespace tamura
{

//==============================================================================
// ColumnState helpers
//==============================================================================
void ReelStripWindow::ColumnState::fillRandom (juce::Random& randomGen)
{
    symbols.resize (static_cast<size_t> (BufferSymbols));
    for (auto& sym : symbols)
        sym = static_cast<ReelSymbol> (randomGen.nextInt (ReelSymbolCount));
}

//==============================================================================
// Constructor
//==============================================================================
ReelStripWindow::ReelStripWindow()
    : rng (juce::Random::getSystemRandom())
{
    setOpaque (false);

    for (auto& col : columns)
        col.fillRandom (rng);
}

//==============================================================================
// Spin control
//==============================================================================
void ReelStripWindow::startSpinning()
{
    stoppedCount = 0;

    for (auto& col : columns)
    {
        col.spinning      = true;
        col.decelerating  = false;
        col.scrollOffset  = 0.0f;
        col.scrollSpeed   = 12.0f + rng.nextFloat() * 4.0f; // 12-16 px/frame
        col.delayRemaining = 0;
        col.fillRandom (rng);
    }

    startTimer (TimerIntervalMs);
}

void ReelStripWindow::stopSpinning (const std::array<tamura::ReelSymbol, 3>& finalSymbols,
                                    int baseDelayMs)
{
    for (int i = 0; i < NumColumns; ++i)
    {
        columns[static_cast<size_t> (i)].targetSymbol    = finalSymbols[static_cast<size_t> (i)];
        columns[static_cast<size_t> (i)].delayRemaining  = juce::jmax (0, baseDelayMs + i * StopDelayStepMs);
        columns[static_cast<size_t> (i)].decelerating    = false;
    }
}

//==============================================================================
// Timer
//==============================================================================
void ReelStripWindow::timerCallback()
{
    bool anyActive = false;
    float ch = cellHeight();

    if (ch < 1.0f)
    {
        // Component not yet laid out -- skip this frame.
        return;
    }

    for (int ci = 0; ci < NumColumns; ++ci)
    {
        auto& col = columns[static_cast<size_t> (ci)];

        if (! col.spinning)
            continue;

        anyActive = true;

        //----------------------------------------------------------------------
        // Pre-deceleration delay
        //----------------------------------------------------------------------
        if (col.delayRemaining > 0)
        {
            col.delayRemaining -= TimerIntervalMs;
            if (col.delayRemaining <= 0)
            {
                col.delayRemaining = 0;
                col.decelerating   = true;
            }
        }

        //----------------------------------------------------------------------
        // Scroll offset accumulation (scroll downward: symbols move top-to-bottom)
        //----------------------------------------------------------------------
        col.scrollOffset += col.scrollSpeed;

        // When a full cell has scrolled past, rotate the strip: remove bottom,
        // add a new random symbol at the top.
        while (col.scrollOffset >= ch)
        {
            col.scrollOffset -= ch;

            if (col.symbols.size() > 1)
                col.symbols.pop_back();

            col.symbols.insert (col.symbols.begin(), randomSymbol());

            while (col.symbols.size() > static_cast<size_t> (BufferSymbols))
                col.symbols.pop_back();
        }

        //----------------------------------------------------------------------
        // Deceleration
        //----------------------------------------------------------------------
        if (col.decelerating)
        {
            col.scrollSpeed *= DecayFactor;

            if (col.scrollSpeed < SnapThreshold)
            {
                // Snap to final position: place targetSymbol in centre row.
                // Centre row index = 1 (of 0,1,2).
                col.scrollOffset = 0.0f;
                col.scrollSpeed  = 0.0f;
                col.spinning     = false;
                col.decelerating = false;

                // Ensure the target symbol is in centre row (index 2 in draw order).
                // Row 0 = above visible, row 1 = top visible, row 2 = payline, row 3 = bottom visible.
                col.symbols.clear();
                col.symbols.push_back (randomSymbol());            // above visible
                col.symbols.push_back (randomSymbol());            // top visible row
                col.symbols.push_back (col.targetSymbol);          // centre / payline row
                col.symbols.push_back (randomSymbol());            // bottom visible row

                // Pad up to buffer length for consistency
                while (col.symbols.size() < static_cast<size_t> (BufferSymbols))
                    col.symbols.push_back (randomSymbol());

                ++stoppedCount;

                if (onColumnStopped)
                    onColumnStopped (ci);

                if (stoppedCount >= NumColumns && onSpinComplete)
                    onSpinComplete();
            }
        }
    }

    repaint();

    if (! anyActive)
        stopTimer();
}

//==============================================================================
// Layout helpers
//==============================================================================
float ReelStripWindow::cellHeight() const
{
    auto reel = innerReelBounds();
    return reel.getHeight() / static_cast<float> (VisibleRows);
}

juce::Rectangle<float> ReelStripWindow::innerReelBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    float bezel = juce::jmax (6.0f, bounds.getWidth() * 0.035f);
    return bounds.reduced (bezel);
}

//==============================================================================
// Paint
//==============================================================================
void ReelStripWindow::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float bezel = juce::jmax (6.0f, bounds.getWidth() * 0.035f);

    //--------------------------------------------------------------------------
    // 1. Chrome beveled outer frame
    //--------------------------------------------------------------------------
    TamuraLookAndFeel::drawMetallicSurface (g, bounds, bezel);

    //--------------------------------------------------------------------------
    // 2. Dark interior fill
    //--------------------------------------------------------------------------
    auto reel = innerReelBounds();
    {
        juce::ColourGradient bg (
            juce::Colour (Palette::surfaceDark),  reel.getX(), reel.getY(),
            juce::Colour (Palette::background),   reel.getX(), reel.getBottom(),
            false);
        g.setGradientFill (bg);
        g.fillRect (reel);
    }

    //--------------------------------------------------------------------------
    // 3. Clip to reel interior, then paint columns
    //--------------------------------------------------------------------------
    {
        g.saveState();
        g.reduceClipRegion (reel.toNearestInt());

        float colWidth = reel.getWidth() / static_cast<float> (NumColumns);

        for (int i = 0; i < NumColumns; ++i)
        {
            auto stripBounds = juce::Rectangle<float> (
                reel.getX() + static_cast<float> (i) * colWidth,
                reel.getY(),
                colWidth,
                reel.getHeight());

            paintColumn (g, columns[static_cast<size_t> (i)], stripBounds);
        }

        g.restoreState();
    }

    //--------------------------------------------------------------------------
    // 4. Column separators
    //--------------------------------------------------------------------------
    paintSeparators (g, reel);

    //--------------------------------------------------------------------------
    // 5. Payline highlight and arrows
    //--------------------------------------------------------------------------
    paintPayline (g, reel);

    //--------------------------------------------------------------------------
    // 6. Glass reflection overlay
    //--------------------------------------------------------------------------
    paintGlassOverlay (g, reel);

    //--------------------------------------------------------------------------
    // 7. Inner border (thin dark line inside bezel)
    //--------------------------------------------------------------------------
    g.setColour (juce::Colour (Palette::background).withAlpha (0.6f));
    g.drawRect (reel, 1.0f);
}

//==============================================================================
void ReelStripWindow::paintColumn (juce::Graphics& g,
                                   const ColumnState& col,
                                   juce::Rectangle<float> stripBounds)
{
    float ch = stripBounds.getHeight() / static_cast<float> (VisibleRows);
    float symbolFontSize = juce::jmax (14.0f, ch * 0.5f);
    auto font = juce::Font (juce::FontOptions().withHeight (symbolFontSize).withStyle ("Bold"));

    // We draw enough symbols to cover the visible area plus one extra above
    // and below to handle the scroll offset.
    int drawCount = VisibleRows + 2;

    for (int row = 0; row < drawCount; ++row)
    {
        if (row >= static_cast<int> (col.symbols.size()))
            break;

        // row 0 = one cell above the visible top, row 1..VisibleRows = visible,
        // row VisibleRows+1 = one cell below visible bottom.
        // scrollOffset pushes symbols downward (top-to-bottom scroll direction).
        float y = stripBounds.getY() + static_cast<float> (row - 1) * ch + col.scrollOffset;

        auto cellRect = juce::Rectangle<float> (
            stripBounds.getX(), y, stripBounds.getWidth(), ch);

        // Skip if entirely outside the strip (clipping handles it but saves work)
        if (cellRect.getBottom() < stripBounds.getY() - ch ||
            cellRect.getY() > stripBounds.getBottom() + ch)
            continue;

        ReelSymbol sym = col.symbols[static_cast<size_t> (row)];
        juce::Colour symColour = getSymbolColour (sym);
        juce::String symText   = getSymbolDisplayText (sym);

        // Text shadow
        g.setFont (font);
        g.setColour (symColour.withAlpha (0.2f));
        g.drawText (symText, cellRect.translated (1.0f, 1.0f),
                    juce::Justification::centred, false);

        // Main symbol
        g.setColour (symColour);
        g.drawText (symText, cellRect,
                    juce::Justification::centred, false);

        // Small name label beneath the symbol
        float nameFontSize = juce::jmax (8.0f, ch * 0.16f);
        auto nameFont = juce::Font (juce::FontOptions().withHeight (nameFontSize));
        g.setFont (nameFont);
        g.setColour (juce::Colour (Palette::textDim).withAlpha (0.6f));
        auto nameRect = cellRect.withTrimmedTop (ch * 0.62f).withHeight (nameFontSize * 1.3f);
        g.drawText (symbolToString (sym), nameRect,
                    juce::Justification::centred, false);
    }
}

//==============================================================================
void ReelStripWindow::paintPayline (juce::Graphics& g,
                                    juce::Rectangle<float> reelBounds)
{
    float ch = reelBounds.getHeight() / static_cast<float> (VisibleRows);

    // Centre row occupies the middle third of the reel area
    float centreRowY = reelBounds.getY() + ch;
    auto paylineStrip = juce::Rectangle<float> (
        reelBounds.getX(), centreRowY,
        reelBounds.getWidth(), ch);

    //--------------------------------------------------------------------------
    // Golden tint over the centre row
    //--------------------------------------------------------------------------
    g.setColour (juce::Colour (0x12FFD700)); // #FFD700 at very low alpha
    g.fillRect (paylineStrip);

    // Thin gold border lines top and bottom of centre row
    g.setColour (juce::Colour (0x30FFD700));
    g.drawHorizontalLine (static_cast<int> (centreRowY),
                          reelBounds.getX(), reelBounds.getRight());
    g.drawHorizontalLine (static_cast<int> (centreRowY + ch),
                          reelBounds.getX(), reelBounds.getRight());

    //--------------------------------------------------------------------------
    // Payline arrows (triangles pointing inward)
    //--------------------------------------------------------------------------
    float arrowSize = juce::jmax (6.0f, ch * 0.2f);
    float midY = centreRowY + ch * 0.5f;
    juce::Colour arrowColour (0xFFFFD700);

    // Left arrow (pointing right)
    {
        juce::Path arrow;
        float ax = reelBounds.getX();
        arrow.addTriangle (ax, midY - arrowSize * 0.5f,
                           ax, midY + arrowSize * 0.5f,
                           ax + arrowSize, midY);
        g.setColour (arrowColour);
        g.fillPath (arrow);
    }

    // Right arrow (pointing left)
    {
        juce::Path arrow;
        float ax = reelBounds.getRight();
        arrow.addTriangle (ax, midY - arrowSize * 0.5f,
                           ax, midY + arrowSize * 0.5f,
                           ax - arrowSize, midY);
        g.setColour (arrowColour);
        g.fillPath (arrow);
    }
}

//==============================================================================
void ReelStripWindow::paintGlassOverlay (juce::Graphics& g,
                                         juce::Rectangle<float> reelBounds)
{
    // Semi-transparent gradient from top to centre simulating glass reflection
    float halfH = reelBounds.getHeight() * 0.5f;

    juce::ColourGradient glassGrad (
        juce::Colours::white.withAlpha (0.08f),
        reelBounds.getX(), reelBounds.getY(),
        juce::Colours::transparentWhite,
        reelBounds.getX(), reelBounds.getY() + halfH,
        false);

    g.setGradientFill (glassGrad);
    g.fillRect (reelBounds.withHeight (halfH));

    // Subtle bright line at the very top edge for a crisp glass edge
    g.setColour (juce::Colours::white.withAlpha (0.1f));
    g.drawHorizontalLine (static_cast<int> (reelBounds.getY()),
                          reelBounds.getX(), reelBounds.getRight());
}

//==============================================================================
void ReelStripWindow::paintSeparators (juce::Graphics& g,
                                       juce::Rectangle<float> reelBounds)
{
    float colWidth = reelBounds.getWidth() / static_cast<float> (NumColumns);
    g.setColour (juce::Colour (0xFF303038));

    for (int i = 1; i < NumColumns; ++i)
    {
        float x = reelBounds.getX() + static_cast<float> (i) * colWidth;
        g.drawVerticalLine (static_cast<int> (x),
                            reelBounds.getY(), reelBounds.getBottom());
    }
}

//==============================================================================
void ReelStripWindow::resized()
{
    // All painting is relative to bounds -- nothing to layout.
}

//==============================================================================
// Helpers
//==============================================================================
ReelSymbol ReelStripWindow::randomSymbol()
{
    return static_cast<ReelSymbol> (rng.nextInt (ReelSymbolCount));
}

} // namespace tamura
