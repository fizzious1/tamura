#include "SamplePlayer.h"

namespace tamura
{

SamplePlayer::SamplePlayer()
{
    formatManager.registerBasicFormats();
    transportSource.addChangeListener (this);
}

SamplePlayer::~SamplePlayer()
{
    transportSource.removeChangeListener (this);
    transportSource.setSource (nullptr);
}

void SamplePlayer::prepare (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay (samplesPerBlock, sampleRate);
}

void SamplePlayer::release()
{
    transportSource.releaseResources();
}

void SamplePlayer::loadFile (const juce::String& path)
{
    auto file = juce::File (path);
    if (! file.existsAsFile())
        return;

    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr)
        return;

    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
    transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
    readerSource = std::move (newSource);

    transportSource.setPosition (0.0);
}

void SamplePlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferInfo)
{
    if (readerSource == nullptr)
    {
        bufferInfo.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock (bufferInfo);
}

void SamplePlayer::play()
{
    transportSource.setPosition (0.0);
    transportSource.start();
}

void SamplePlayer::stop()
{
    transportSource.stop();
}

bool SamplePlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

void SamplePlayer::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // Transport state changed — could notify UI in the future
}

} // namespace tamura
