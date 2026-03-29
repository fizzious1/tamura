#include "PluginProcessor.h"
#include "PluginEditor.h"

TamuraAudioProcessorEditor::TamuraAudioProcessorEditor (TamuraAudioProcessor& p)
    : AudioProcessorEditor (&p), tamuraProcessor (p)
{
    setLookAndFeel (&tamuraLookAndFeel);

    // --- Title with neon frame ---
    titleLabel.setText ("SLOT MACHINE");
    titleLabel.setNeonColour (juce::Colour (0xFF9370DB));
    titleLabel.setFontSize (36.0f);
    titleLabel.setDrawFrame (true);
    titleLabel.setFramePadding (6.0f);
    addAndMakeVisible (titleLabel);

    // --- Token display (top right) ---
    tokenLabel.setNeonColour (juce::Colour (0xFFFFD700));
    tokenLabel.setFontSize (16.0f);
    tokenLabel.setJustification (juce::Justification::centredRight);
    addAndMakeVisible (tokenLabel);

    // --- Reels ---
    addAndMakeVisible (reel1);
    addAndMakeVisible (reel2);
    addAndMakeVisible (reel3);

    reel1.onSpinComplete = [this] { reelsStoppedCount++; if (reelsStoppedCount >= 3) onAllReelsStopped(); };
    reel2.onSpinComplete = [this] { reelsStoppedCount++; if (reelsStoppedCount >= 3) onAllReelsStopped(); };
    reel3.onSpinComplete = [this] { reelsStoppedCount++; if (reelsStoppedCount >= 3) onAllReelsStopped(); };

    // --- Reel strip window ---
    reelStrip.onColumnStopped = [this] (int columnIndex)
    {
        // When a strip column stops, reveal the symbol in the corresponding gauge
        if (! pendingOutcome.has_value())
            return;

        auto& reels = pendingOutcome->spinResult.reels;
        tamura::SlotReelComponent* gauges[] = { &reel1, &reel2, &reel3 };

        if (columnIndex >= 0 && columnIndex < 3)
        {
            gauges[columnIndex]->stopSpinning (reels[static_cast<size_t> (columnIndex)], 0);
        }
    };
    addAndMakeVisible (reelStrip);

    // --- Side lever ---
    sideLever.onPull = [this] { performSpin(); };
    addAndMakeVisible (sideLever);

    // --- Spin button ---
    spinBtn.onClick = [this] { performSpin(); };
    addAndMakeVisible (spinBtn);

    // --- Result labels ---
    resultLabel.setNeonColour (juce::Colour (0xFFFFD700));
    resultLabel.setFontSize (28.0f);
    addAndMakeVisible (resultLabel);

    soundNameLabel.setNeonColour (juce::Colour (0xFFF0E6D2));
    soundNameLabel.setFontSize (14.0f);
    addAndMakeVisible (soundNameLabel);

    // --- Control panel ---
    controlPanel.onPreview = [this]
    {
        if (pendingOutcome.has_value() && pendingOutcome->awardedSound != nullptr)
            tamuraProcessor.previewSound (pendingOutcome->awardedSound->id);
    };
    controlPanel.onStop = [this] { tamuraProcessor.stopPreview(); };
    controlPanel.onSave = [this] { saveLastSoundToDisk(); };
    controlPanel.onLibrary = [this] { toggleSoundBrowser(); };
    addAndMakeVisible (controlPanel);

    // --- Waveform display ---
    addAndMakeVisible (waveformDisplay);

    // --- Sound browser (initially hidden) ---
    soundBrowser.onPreview = [this] (const juce::String& soundId) { tamuraProcessor.previewSound (soundId); };
    soundBrowser.onSelect = [this] (const juce::String& soundId) { tamuraProcessor.previewSound (soundId); };
    soundBrowser.setVisible (false);
    addAndMakeVisible (soundBrowser);

    // --- Status bar ---
    statusLabel.setNeonColour (juce::Colour (0xFF4169E1));
    statusLabel.setFontSize (11.0f);
    statusLabel.setJustification (juce::Justification::centredLeft);
    addAndMakeVisible (statusLabel);

    // Initial state
    updateTokenDisplay();
    updateStatusBar();

    auto tokensGranted = tamuraProcessor.getTokenManager().claimDailyTokens();
    if (tokensGranted > 0)
    {
        tamuraProcessor.getTokenManager().saveState();
        showDailyGrantNotification (tokensGranted);
    }

    setSize (520, 840);
}

TamuraAudioProcessorEditor::~TamuraAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void TamuraAudioProcessorEditor::timerCallback()
{
    sirenPulsePhase += 0.15f;
    // Only repaint the small siren area
    repaint (juce::Rectangle<int> (getWidth() / 2 - 20, 58, 40, 24));
}

//==============================================================================
void TamuraAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Dark background
    g.fillAll (juce::Colour (0xFF0D0D12));

    // ── Blue neon side bars ──────────────────────────────────────────────
    {
        float barWidth = 4.0f;
        float barMargin = 3.0f;
        auto leftBar = juce::Rectangle<float> (barMargin, 60.0f, barWidth, bounds.getHeight() - 80.0f);
        auto rightBar = juce::Rectangle<float> (bounds.getWidth() - barMargin - barWidth, 60.0f, barWidth, bounds.getHeight() - 80.0f);

        for (int pass = 4; pass >= 1; --pass)
        {
            float expand = static_cast<float> (pass) * 2.0f;
            float alpha = 0.04f * (1.0f - static_cast<float> (pass) / 5.0f);
            g.setColour (juce::Colour (tamura::Palette::neonBlue).withAlpha (alpha));
            g.fillRoundedRectangle (leftBar.expanded (expand, 0.0f), 2.0f);
            g.fillRoundedRectangle (rightBar.expanded (expand, 0.0f), 2.0f);
        }

        g.setColour (juce::Colour (tamura::Palette::neonBlue).withAlpha (0.7f));
        g.fillRoundedRectangle (leftBar, 2.0f);
        g.fillRoundedRectangle (rightBar, 2.0f);

        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.fillRoundedRectangle (leftBar.reduced (1.0f, 0.0f), 1.0f);
        g.fillRoundedRectangle (rightBar.reduced (1.0f, 0.0f), 1.0f);
    }

    // ── Machine body ─────────────────────────────────────────────────────
    auto machineBody = bounds.reduced (10.0f, 0.0f).withY (60.0f).withHeight (bounds.getHeight() - 70.0f);
    tamura::TamuraLookAndFeel::drawMetallicSurface (g, machineBody, 3.0f);

    // Darker inset for the reel area
    auto reelInset = juce::Rectangle<float> (30.0f, 82.0f, bounds.getWidth() - 60.0f, 170.0f);
    g.setColour (juce::Colour (0xFF0A0A14));
    g.fillRoundedRectangle (reelInset, 6.0f);
    g.setColour (juce::Colour (0xFF303038));
    g.drawRoundedRectangle (reelInset, 6.0f, 1.5f);

    // Screws in corners of the machine body
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getX() + 15.0f, machineBody.getY() + 15.0f }, 5.0f);
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getRight() - 15.0f, machineBody.getY() + 15.0f }, 5.0f);
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getX() + 15.0f, machineBody.getBottom() - 15.0f }, 5.0f);
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getRight() - 15.0f, machineBody.getBottom() - 15.0f }, 5.0f);

    // ── Red siren dome light ─────────────────────────────────────────────
    {
        float sirenCx = bounds.getWidth() * 0.5f;
        float sirenCy = 70.0f;
        float sirenRadius = 8.0f;

        float glowAlpha = sirenActive
            ? 0.3f + 0.15f * std::sin (sirenPulsePhase)
            : 0.08f;

        g.setColour (juce::Colour (0xFFFF2020).withAlpha (glowAlpha));
        g.fillEllipse (sirenCx - 16.0f, sirenCy - 16.0f, 32.0f, 32.0f);

        juce::ColourGradient sirenGrad (juce::Colour (0xFFEE3333), sirenCx, sirenCy - sirenRadius,
                                         juce::Colour (0xFF881111), sirenCx, sirenCy + sirenRadius, false);
        g.setGradientFill (sirenGrad);
        g.fillEllipse (sirenCx - sirenRadius, sirenCy - sirenRadius, sirenRadius * 2.0f, sirenRadius * 2.0f);

        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.fillEllipse (sirenCx - 3.0f, sirenCy - 5.0f, 5.0f, 4.0f);
    }

    // ── Metallic strip above control panel ───────────────────────────────
    auto stripArea = juce::Rectangle<float> (20.0f, 573.0f, bounds.getWidth() - 40.0f, 4.0f);
    g.setGradientFill (juce::ColourGradient (
        juce::Colour (0xFF606068), stripArea.getX(), stripArea.getY(),
        juce::Colour (0xFF909098), stripArea.getRight(), stripArea.getY(), false));
    g.fillRect (stripArea);
}

//==============================================================================
void TamuraAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto cx = bounds.getCentreX();

    // Title — top neon sign with frame
    titleLabel.setBounds (bounds.getX(), 4, bounds.getWidth(), 52);

    // Token display — top right
    tokenLabel.setBounds (bounds.getWidth() - 180, 8, 170, 30);

    // 3 reels — centered, circular gauges
    int reelSize = 130;
    int reelY = 97;
    int totalReelWidth = reelSize * 3 + 30;
    int reelStartX = cx - totalReelWidth / 2;

    reel1.setBounds (reelStartX, reelY, reelSize, reelSize);
    reel2.setBounds (reelStartX + reelSize + 15, reelY, reelSize, reelSize);
    reel3.setBounds (reelStartX + 2 * (reelSize + 15), reelY, reelSize, reelSize);

    // Side lever — right edge
    sideLever.setBounds (475, 85, 35, 270);

    // Spin button — centered below reels
    int spinSize = 100;
    spinBtn.setBounds (cx - spinSize / 2, 255, spinSize, spinSize);

    // Result display
    resultLabel.setBounds (cx - 150, 365, 300, 35);
    soundNameLabel.setBounds (cx - 150, 398, 300, 22);

    // Reel strip window — traditional slot strip
    reelStrip.setBounds (30, 425, bounds.getWidth() - 60, 130);

    // Control panel — row of buttons
    controlPanel.setBounds (cx - 160, 580, 320, 45);

    // Waveform display
    waveformDisplay.setBounds (30, 640, bounds.getWidth() - 60, 140);

    // Sound browser — overlays the waveform area when visible
    soundBrowser.setBounds (30, 640, bounds.getWidth() - 60, 140);

    // Status bar
    statusLabel.setBounds (30, bounds.getHeight() - 30, bounds.getWidth() - 60, 25);
}

//==============================================================================
void TamuraAudioProcessorEditor::performSpin()
{
    if (isSpinning)
        return;

    auto outcome = tamuraProcessor.performSpin();
    if (! outcome.has_value())
    {
        resultLabel.setNeonColour (juce::Colour (0xFFCC2222));
        resultLabel.setText ("NO TOKENS");
        soundNameLabel.setText ("");
        return;
    }

    pendingOutcome = outcome;
    isSpinning = true;
    reelsStoppedCount = 0;
    spinBtn.setEnabled (false);
    sideLever.setEnabled (false);

    resultLabel.setText ("");
    soundNameLabel.setText ("");

    // Start siren pulsing
    sirenActive = true;
    startTimer (33);

    // Start circular reels (they keep spinning until reel strip columns stop)
    reel1.startSpinning();
    reel2.startSpinning();
    reel3.startSpinning();

    auto& reels = outcome->spinResult.reels;

    // Reel strip drives the timing — circular gauges follow via onColumnStopped
    reelStrip.startSpinning();
    reelStrip.stopSpinning (reels, 800);
}

void TamuraAudioProcessorEditor::onAllReelsStopped()
{
    isSpinning = false;
    spinBtn.setEnabled (true);
    sideLever.setEnabled (true);

    // Stop siren
    sirenActive = false;
    sirenPulsePhase = 0.0f;
    stopTimer();
    repaint (juce::Rectangle<int> (getWidth() / 2 - 20, 58, 40, 24));

    updateTokenDisplay();
    updateStatusBar();

    if (! pendingOutcome.has_value())
        return;

    auto tier = pendingOutcome->spinResult.tier;
    auto tierStr = tamura::tierToString (tier);

    switch (tier)
    {
        case tamura::Tier::Common:    resultLabel.setNeonColour (juce::Colour (0xFFF0E6D2)); break;
        case tamura::Tier::Uncommon:  resultLabel.setNeonColour (juce::Colour (0xFF22AA44)); break;
        case tamura::Tier::Rare:      resultLabel.setNeonColour (juce::Colour (0xFF4169E1)); break;
        case tamura::Tier::Epic:      resultLabel.setNeonColour (juce::Colour (0xFF9370DB)); break;
        case tamura::Tier::Legendary: resultLabel.setNeonColour (juce::Colour (0xFFFFD700)); break;
        default: break;
    }

    resultLabel.setText (tierStr + "!");

    if (pendingOutcome->awardedSound != nullptr)
    {
        soundNameLabel.setText (pendingOutcome->awardedSound->name);
        tamuraProcessor.previewSound (pendingOutcome->awardedSound->id);
    }
    else
    {
        soundNameLabel.setText ("ALL UNLOCKED");
    }

    updateWaveformDisplay();
}

void TamuraAudioProcessorEditor::updateTokenDisplay()
{
    auto balance = tamuraProcessor.getTokenManager().getBalance();
    tokenLabel.setText ("TOKENS " + juce::String (balance));
}

void TamuraAudioProcessorEditor::updateStatusBar()
{
    auto unlocked = tamuraProcessor.getSoundLibrary().getUnlockedCount();
    auto total = tamuraProcessor.getSoundLibrary().getTotalSoundCount();
    statusLabel.setText ("SOUNDS " + juce::String (unlocked) + "/" + juce::String (total));
}

void TamuraAudioProcessorEditor::updateWaveformDisplay()
{
    auto* buffer = tamuraProcessor.getSamplePlayer().getAudioBuffer();
    waveformDisplay.setAudioBuffer (buffer);

    if (buffer != nullptr)
    {
        auto displayWidth = waveformDisplay.getWidth();
        if (displayWidth > 0)
        {
            int spp = buffer->getNumSamples() / displayWidth;
            waveformDisplay.setSamplesPerPixel (juce::jmax (1, spp));
        }
    }
}

void TamuraAudioProcessorEditor::saveLastSoundToDisk()
{
    if (! pendingOutcome.has_value() || pendingOutcome->awardedSound == nullptr)
        return;

    auto& sound = *pendingOutcome->awardedSound;
    auto sourceFile = tamuraProcessor.getSoundLibrary().getSoundFile (sound);
    if (! sourceFile.existsAsFile())
        return;

    auto defaultName = sound.name.replaceCharacters (" ", "_") + ".wav";

    fileChooser = std::make_unique<juce::FileChooser> (
        "Save Sound", juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile (defaultName),
        "*.wav");

    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [sourceFile] (const juce::FileChooser& chooser)
        {
            auto dest = chooser.getResult();
            if (dest != juce::File())
                sourceFile.copyFileTo (dest);
        });
}

void TamuraAudioProcessorEditor::showDailyGrantNotification (int tokensGranted)
{
    resultLabel.setNeonColour (juce::Colour (0xFFFFD700));
    resultLabel.setText ("DAILY BONUS!");
    soundNameLabel.setText ("+" + juce::String (tokensGranted) + " tokens");
    updateTokenDisplay();
}

void TamuraAudioProcessorEditor::toggleSoundBrowser()
{
    soundBrowserVisible = ! soundBrowserVisible;

    if (soundBrowserVisible)
        refreshSoundBrowser();

    soundBrowser.setVisible (soundBrowserVisible);
    waveformDisplay.setVisible (! soundBrowserVisible);
}

void TamuraAudioProcessorEditor::refreshSoundBrowser()
{
    auto unlocked = tamuraProcessor.getSoundLibrary().getUnlockedSounds();
    soundBrowser.setSounds (unlocked);
}
