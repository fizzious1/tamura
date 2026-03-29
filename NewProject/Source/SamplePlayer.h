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

    // Get the currently loaded audio buffer (for waveform display).
    // Returns nullptr if no file is loaded.
    const juce::AudioBuffer<float>* getAudioBuffer() const;

    // Get the file path of the currently loaded sound
    juce::String getLoadedFilePath() const;

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioBuffer<float> loadedBuffer;
    juce::String loadedFilePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplePlayer)
};

} // namespace tamura
