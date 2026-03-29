#include "PluginProcessor.h"
#include "PluginEditor.h"

TamuraAudioProcessor::TamuraAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    initializeSoundLibrary();
}

TamuraAudioProcessor::~TamuraAudioProcessor()
{
}

void TamuraAudioProcessor::initializeSoundLibrary()
{
    auto appDataDir = juce::File::getSpecialLocation (
        juce::File::userApplicationDataDirectory);

#if JUCE_MAC
    auto tamuraDir = appDataDir.getChildFile ("Application Support").getChildFile ("Tamura");
#else
    auto tamuraDir = appDataDir.getChildFile ("Tamura");
#endif

    auto soundsDir = tamuraDir.getChildFile ("Sounds");
    soundLibrary.setSoundsDirectory (soundsDir);

    auto manifestFile = soundsDir.getChildFile ("manifest.json");
    soundLibrary.loadManifest (manifestFile);
}

//==============================================================================
const juce::String TamuraAudioProcessor::getName() const
{
    return "Tamura";
}

bool TamuraAudioProcessor::acceptsMidi() const    { return false; }
bool TamuraAudioProcessor::producesMidi() const   { return false; }
bool TamuraAudioProcessor::isMidiEffect() const   { return false; }
double TamuraAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TamuraAudioProcessor::getNumPrograms()                              { return 1; }
int TamuraAudioProcessor::getCurrentProgram()                           { return 0; }
void TamuraAudioProcessor::setCurrentProgram (int)                      {}
const juce::String TamuraAudioProcessor::getProgramName (int)           { return {}; }
void TamuraAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void TamuraAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    samplePlayer.prepare (sampleRate, samplesPerBlock);
}

void TamuraAudioProcessor::releaseResources()
{
    samplePlayer.release();
}

bool TamuraAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto mainOutput = layouts.getMainOutputChannelSet();
    return mainOutput == juce::AudioChannelSet::mono()
        || mainOutput == juce::AudioChannelSet::stereo();
}

void TamuraAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::AudioSourceChannelInfo bufferInfo (buffer);
    samplePlayer.getNextAudioBlock (bufferInfo);
}

//==============================================================================
bool TamuraAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* TamuraAudioProcessor::createEditor()
{
    return new TamuraAudioProcessorEditor (*this);
}

//==============================================================================
void TamuraAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = std::make_unique<juce::DynamicObject>();
    state->setProperty ("tokens", tokenManager.toVar());
    state->setProperty ("unlockedSounds", soundLibrary.toVar());

    juce::var stateVar (state.release());
    auto jsonString = juce::JSON::toString (stateVar);
    destData.reset();
    destData.append (jsonString.toRawUTF8(), jsonString.getNumBytesAsUTF8());
}

void TamuraAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto jsonString = juce::String::fromUTF8 (static_cast<const char*> (data), sizeInBytes);
    auto state = juce::JSON::parse (jsonString);

    if (state.isObject())
    {
        if (state.hasProperty ("tokens"))
            tokenManager.fromVar (state["tokens"]);
        if (state.hasProperty ("unlockedSounds"))
            soundLibrary.fromVar (state["unlockedSounds"]);
    }
}

//==============================================================================
std::optional<TamuraAudioProcessor::SpinOutcome> TamuraAudioProcessor::performSpin()
{
    if (! tokenManager.spendToken())
        return std::nullopt;

    auto spinResult = spinEngine.spin();
    auto* awardedSound = soundLibrary.awardSound (spinResult.tier);

    SpinOutcome outcome { spinResult, awardedSound };
    lastSpinOutcome = outcome;

    tokenManager.saveState();
    return outcome;
}

void TamuraAudioProcessor::previewSound (const juce::String& soundId)
{
    for (const auto& entry : soundLibrary.getUnlockedSounds())
    {
        if (entry->id == soundId)
        {
            auto soundFile = soundLibrary.getSoundFile (*entry);
            if (soundFile.existsAsFile())
            {
                samplePlayer.loadFile (soundFile.getFullPathName());
                samplePlayer.play();
            }
            break;
        }
    }
}

void TamuraAudioProcessor::stopPreview()
{
    samplePlayer.stop();
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TamuraAudioProcessor();
}
