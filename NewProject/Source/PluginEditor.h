#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/TamuraLookAndFeel.h"
#include "UI/SlotReelComponent.h"
#include "UI/SpinButton.h"
#include "UI/ControlPanel.h"
#include "UI/WaveformDisplay.h"
#include "UI/NeonLabel.h"

class TamuraAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TamuraAudioProcessorEditor (TamuraAudioProcessor&);
    ~TamuraAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void performSpin();
    void onAllReelsStopped();
    void updateTokenDisplay();
    void updateStatusBar();

    TamuraAudioProcessor& tamuraProcessor;
    tamura::TamuraLookAndFeel tamuraLookAndFeel;

    // Title
    NeonLabel titleLabel;

    // 3 reel gauges
    tamura::SlotReelComponent reel1;
    tamura::SlotReelComponent reel2;
    tamura::SlotReelComponent reel3;

    // Spin button
    tamura::SpinButton spinBtn;

    // Result display
    NeonLabel resultLabel;
    NeonLabel soundNameLabel;

    // Token display
    NeonLabel tokenLabel;

    // Control panel (Preview, Stop, Save, Library)
    tamura::ControlPanel controlPanel;

    // Waveform display
    WaveformDisplay waveformDisplay;

    // Status bar
    NeonLabel statusLabel;

    // Spin state
    bool isSpinning = false;
    int reelsStoppedCount = 0;
    std::optional<TamuraAudioProcessor::SpinOutcome> pendingOutcome;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TamuraAudioProcessorEditor)
};
