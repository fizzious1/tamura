#include "PluginProcessor.h"
#include "PluginEditor.h"

TamuraAudioProcessorEditor::TamuraAudioProcessorEditor (TamuraAudioProcessor& p)
    : AudioProcessorEditor (&p), tamuraProcessor (p)
{
    setLookAndFeel (&tamuraLookAndFeel);

    // --- Title ---
    titleLabel.setText ("SLOT MACHINE");
    titleLabel.setNeonColour (juce::Colour (0xFF9370DB));
    titleLabel.setFontSize (36.0f);
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

    // Set up reel completion callbacks
    reel1.onSpinComplete = [this] { reelsStoppedCount++; if (reelsStoppedCount >= 3) onAllReelsStopped(); };
    reel2.onSpinComplete = [this] { reelsStoppedCount++; if (reelsStoppedCount >= 3) onAllReelsStopped(); };
    reel3.onSpinComplete = [this] { reelsStoppedCount++; if (reelsStoppedCount >= 3) onAllReelsStopped(); };

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
    controlPanel.onSave = [] {}; // TODO: implement save-to-disk
    controlPanel.onLibrary = [] {}; // TODO: implement library browser
    addAndMakeVisible (controlPanel);

    // --- Waveform display ---
    addAndMakeVisible (waveformDisplay);

    // --- Status bar ---
    statusLabel.setNeonColour (juce::Colour (0xFF4169E1));
    statusLabel.setFontSize (11.0f);
    statusLabel.setJustification (juce::Justification::centredLeft);
    addAndMakeVisible (statusLabel);

    // Initial state
    updateTokenDisplay();
    updateStatusBar();

    setSize (520, 740);
}

TamuraAudioProcessorEditor::~TamuraAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void TamuraAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background
    g.fillAll (juce::Colour (0xFF0D0D12));

    auto bounds = getLocalBounds().toFloat();

    // Machine body — metallic panel behind the reels area
    auto machineBody = bounds.reduced (10.0f, 0.0f).withY (60.0f).withHeight (bounds.getHeight() - 70.0f);
    tamura::TamuraLookAndFeel::drawMetallicSurface (g, machineBody, 3.0f);

    // Darker inset for the reel area
    auto reelInset = juce::Rectangle<float> (30.0f, 80.0f, bounds.getWidth() - 60.0f, 170.0f);
    g.setColour (juce::Colour (0xFF0A0A14));
    g.fillRoundedRectangle (reelInset, 6.0f);
    g.setColour (juce::Colour (0xFF303038));
    g.drawRoundedRectangle (reelInset, 6.0f, 1.5f);

    // Screws in corners of the machine body
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getX() + 15.0f, machineBody.getY() + 15.0f }, 5.0f);
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getRight() - 15.0f, machineBody.getY() + 15.0f }, 5.0f);
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getX() + 15.0f, machineBody.getBottom() - 15.0f }, 5.0f);
    tamura::TamuraLookAndFeel::drawScrewHead (g, { machineBody.getRight() - 15.0f, machineBody.getBottom() - 15.0f }, 5.0f);

    // Metallic strip above control panel
    auto stripArea = juce::Rectangle<float> (20.0f, 510.0f, bounds.getWidth() - 40.0f, 4.0f);
    g.setGradientFill (juce::ColourGradient (
        juce::Colour (0xFF606068), stripArea.getX(), stripArea.getY(),
        juce::Colour (0xFF909098), stripArea.getRight(), stripArea.getY(), false));
    g.fillRect (stripArea);
}

void TamuraAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto cx = bounds.getCentreX();

    // Title — top neon sign
    titleLabel.setBounds (bounds.getX(), 8, bounds.getWidth(), 50);

    // Token display — top right
    tokenLabel.setBounds (bounds.getWidth() - 180, 12, 170, 30);

    // 3 reels — centered, circular gauges
    int reelSize = 130;
    int reelY = 95;
    int totalReelWidth = reelSize * 3 + 30; // 30px spacing
    int reelStartX = cx - totalReelWidth / 2;

    reel1.setBounds (reelStartX, reelY, reelSize, reelSize);
    reel2.setBounds (reelStartX + reelSize + 15, reelY, reelSize, reelSize);
    reel3.setBounds (reelStartX + 2 * (reelSize + 15), reelY, reelSize, reelSize);

    // Spin button — centered below reels
    int spinSize = 110;
    spinBtn.setBounds (cx - spinSize / 2, 260, spinSize, spinSize);

    // Result display
    resultLabel.setBounds (cx - 150, 380, 300, 40);
    soundNameLabel.setBounds (cx - 150, 418, 300, 25);

    // Control panel — row of buttons
    controlPanel.setBounds (cx - 160, 455, 320, 45);

    // Waveform display
    waveformDisplay.setBounds (30, 520, bounds.getWidth() - 60, 150);

    // Status bar
    statusLabel.setBounds (30, bounds.getHeight() - 30, bounds.getWidth() - 60, 25);
}

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

    resultLabel.setText ("");
    soundNameLabel.setText ("");

    // Start all reels spinning
    reel1.startSpinning();
    reel2.startSpinning();
    reel3.startSpinning();

    // Stop them sequentially with the correct symbols
    auto& reels = outcome->spinResult.reels;
    reel1.stopSpinning (reels[0], 600);   // stops after 600ms
    reel2.stopSpinning (reels[1], 1000);  // stops after 1000ms
    reel3.stopSpinning (reels[2], 1400);  // stops after 1400ms
}

void TamuraAudioProcessorEditor::onAllReelsStopped()
{
    isSpinning = false;
    spinBtn.setEnabled (true);
    updateTokenDisplay();
    updateStatusBar();

    if (! pendingOutcome.has_value())
        return;

    auto tier = pendingOutcome->spinResult.tier;
    auto tierStr = tamura::tierToString (tier);

    // Set tier-specific neon color
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
        soundNameLabel.setText (pendingOutcome->awardedSound->name);
    else
        soundNameLabel.setText ("ALL UNLOCKED");
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
