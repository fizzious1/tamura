#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "SpinEngine.h"
#include "SamplePlayer.h"
#include "TokenManager.h"
#include "SoundLibrary.h"

class TamuraAudioProcessor : public juce::AudioProcessor
{
public:
    TamuraAudioProcessor();
    ~TamuraAudioProcessor() override;

    // Audio lifecycle
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // Plugin info
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // Programs (unused but required)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // State persistence
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- Tamura public API (called by the editor) ---

    struct SpinOutcome
    {
        tamura::SpinResult spinResult;
        const tamura::SoundEntry* awardedSound = nullptr;
    };

    // Perform a spin: spend a token, roll RNG, award a sound, play it.
    // Returns nullopt if the user can't afford a spin.
    std::optional<SpinOutcome> performSpin();

    // Preview a previously unlocked sound
    void previewSound (const juce::String& soundId);

    // Stop any playing preview
    void stopPreview();

    // Accessors for UI
    tamura::TokenManager& getTokenManager()   { return tokenManager; }
    tamura::SoundLibrary& getSoundLibrary()   { return soundLibrary; }
    tamura::SpinEngine&   getSpinEngine()     { return spinEngine; }
    tamura::SamplePlayer& getSamplePlayer()   { return samplePlayer; }

    // Last spin result (for UI to read after a spin)
    std::optional<SpinOutcome> getLastSpinOutcome() const { return lastSpinOutcome; }

private:
    tamura::SpinEngine   spinEngine;
    tamura::SamplePlayer samplePlayer;
    tamura::TokenManager tokenManager;
    tamura::SoundLibrary soundLibrary;

    std::optional<SpinOutcome> lastSpinOutcome;

    void initializeSoundLibrary();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TamuraAudioProcessor)
};
