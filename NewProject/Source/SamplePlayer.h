#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

namespace tamura
{

// Loads and plays WAV samples from disk.
// All file I/O happens off the audio thread.
class SamplePlayer : private juce::ChangeListener
{
public:
    SamplePlayer();
    ~SamplePlayer() override;

    // Call from prepareToPlay
    void prepare (double sampleRate, int samplesPerBlock);

    // Call from releaseResources
    void release();

    // Load a WAV file by path. Non-blocking — loading happens asynchronously.
    void loadFile (const juce::String& path);

    // Fill the output buffer with audio from the loaded sample.
    // Call from processBlock.
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferInfo);

    // Transport controls
    void play();
    void stop();
    bool isPlaying() const;

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplePlayer)
};

} // namespace tamura
