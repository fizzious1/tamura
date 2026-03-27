#pragma once

#include <juce_core/juce_core.h>
#include "SpinEngine.h"
#include <vector>
#include <set>

namespace tamura
{

struct SoundEntry
{
    juce::String id;           // e.g. "drum_common_001"
    juce::String name;         // e.g. "Clean Kick"
    juce::String category;     // e.g. "Drums"
    Tier tier;
    juce::String relativePath; // e.g. "Drums/Common/clean_kick.wav"
};

// Manages the catalog of available sounds, tracks which ones the user has unlocked,
// and selects sounds from the appropriate tier on each spin.
class SoundLibrary
{
public:
    SoundLibrary();

    // Load the sound catalog from a manifest.json file
    bool loadManifest (const juce::File& manifestFile);

    // Get a random unlocked sound for the given tier. Returns nullptr if
    // all sounds in that tier are already unlocked (or tier is empty).
    // Falls through to the next tier up if current tier is exhausted.
    const SoundEntry* awardSound (Tier tier);

    // Get the full file path for a sound entry
    juce::File getSoundFile (const SoundEntry& entry) const;

    // Unlock tracking
    bool isSoundUnlocked (const juce::String& soundId) const;
    int getUnlockedCount() const;
    int getTotalSoundCount() const;
    std::vector<const SoundEntry*> getUnlockedSounds() const;

    // Persistence
    juce::var toVar() const;
    void fromVar (const juce::var& data);

    // Set the root directory where sounds are stored
    void setSoundsDirectory (const juce::File& dir);
    juce::File getSoundsDirectory() const;

private:
    Tier nextTierUp (Tier tier) const;
    std::vector<const SoundEntry*> getAvailableSounds (Tier tier) const;

    std::vector<SoundEntry> catalog;
    std::set<juce::String> unlockedSoundIds;
    juce::File soundsDirectory;
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundLibrary)
};

} // namespace tamura
