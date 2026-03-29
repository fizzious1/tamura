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
#include "UI/SoundBrowser.h"
#include "UI/ReelStripWindow.h"
#include "UI/SideLever.h"

class TamuraAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    TamuraAudioProcessorEditor (TamuraAudioProcessor&);
    ~TamuraAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void performSpin();
    void onAllReelsStopped();
    void updateTokenDisplay();
    void updateStatusBar();
    void updateWaveformDisplay();
    void saveLastSoundToDisk();
    void showDailyGrantNotification (int tokensGranted);
    void toggleSoundBrowser();
    void refreshSoundBrowser();

    TamuraAudioProcessor& tamuraProcessor;
    tamura::TamuraLookAndFeel tamuraLookAndFeel;

    // Title
    tamura::NeonLabel titleLabel;

    // 3 reel gauges
    tamura::SlotReelComponent reel1;
    tamura::SlotReelComponent reel2;
    tamura::SlotReelComponent reel3;

    // Reel strip window (traditional slot strip below gauges)
    tamura::ReelStripWindow reelStrip;

    // Side lever
    tamura::SideLever sideLever;

    // Spin button
    tamura::SpinButton spinBtn;

    // Result display
    tamura::NeonLabel resultLabel;
    tamura::NeonLabel soundNameLabel;

    // Token display
    tamura::NeonLabel tokenLabel;

    // Control panel (Preview, Stop, Save, Library)
    tamura::ControlPanel controlPanel;

    // Waveform display
    tamura::WaveformDisplay waveformDisplay;

    // Sound browser (toggled by Library button)
    tamura::SoundBrowser soundBrowser;
    bool soundBrowserVisible = false;

    // Status bar
    tamura::NeonLabel statusLabel;

    // File chooser (must be a member — JUCE async choosers must outlive the callback)
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Spin state
    bool isSpinning = false;
    int reelsStoppedCount = 0;
    std::optional<TamuraAudioProcessor::SpinOutcome> pendingOutcome;

    // Siren animation
    float sirenPulsePhase = 0.0f;
    bool sirenActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TamuraAudioProcessorEditor)
};
