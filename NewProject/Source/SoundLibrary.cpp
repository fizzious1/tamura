#include "SoundLibrary.h"

namespace tamura
{

SoundLibrary::SoundLibrary()
    : rng (juce::Random::getSystemRandom())
{
}

bool SoundLibrary::loadManifest (const juce::File& manifestFile)
{
    if (! manifestFile.existsAsFile())
        return false;

    auto jsonString = manifestFile.loadFileAsString();
    auto parsed = juce::JSON::parse (jsonString);

    if (! parsed.isObject() || ! parsed.hasProperty ("sounds"))
        return false;

    auto soundsArray = parsed["sounds"];
    if (! soundsArray.isArray())
        return false;

    catalog.clear();

    for (int i = 0; i < soundsArray.size(); ++i)
    {
        auto entry = soundsArray[i];
        if (! entry.isObject())
            continue;

        SoundEntry sound;
        sound.id = entry["id"].toString();
        sound.name = entry["name"].toString();
        sound.category = entry["category"].toString();
        sound.relativePath = entry["file"].toString();

        auto tierStr = entry["tier"].toString();
        if      (tierStr == "Common")    sound.tier = Tier::Common;
        else if (tierStr == "Uncommon")  sound.tier = Tier::Uncommon;
        else if (tierStr == "Rare")      sound.tier = Tier::Rare;
        else if (tierStr == "Epic")      sound.tier = Tier::Epic;
        else if (tierStr == "Legendary") sound.tier = Tier::Legendary;
        else continue; // skip unknown tiers

        if (sound.id.isNotEmpty() && sound.name.isNotEmpty())
            catalog.push_back (std::move (sound));
    }

    return ! catalog.empty();
}

const SoundEntry* SoundLibrary::awardSound (Tier tier)
{
    // Try the requested tier, then fall through upward
    auto currentTier = tier;

    for (int attempt = 0; attempt < 5; ++attempt)
    {
        auto available = getAvailableSounds (currentTier);
        if (! available.empty())
        {
            auto* chosen = available[(size_t) rng.nextInt ((int) available.size())];
            unlockedSoundIds.insert (chosen->id);
            return chosen;
        }

        // This tier is exhausted — try the next one up
        auto next = nextTierUp (currentTier);
        if (next == currentTier)
            break; // already at max tier
        currentTier = next;
    }

    return nullptr; // everything unlocked or catalog empty
}

juce::File SoundLibrary::getSoundFile (const SoundEntry& entry) const
{
    return soundsDirectory.getChildFile (entry.relativePath);
}

bool SoundLibrary::isSoundUnlocked (const juce::String& soundId) const
{
    return unlockedSoundIds.count (soundId) > 0;
}

int SoundLibrary::getUnlockedCount() const
{
    return static_cast<int> (unlockedSoundIds.size());
}

int SoundLibrary::getTotalSoundCount() const
{
    return static_cast<int> (catalog.size());
}

std::vector<const SoundEntry*> SoundLibrary::getUnlockedSounds() const
{
    std::vector<const SoundEntry*> result;
    for (const auto& entry : catalog)
    {
        if (isSoundUnlocked (entry.id))
            result.push_back (&entry);
    }
    return result;
}

juce::var SoundLibrary::toVar() const
{
    juce::Array<juce::var> ids;
    for (const auto& id : unlockedSoundIds)
        ids.add (id);
    return ids;
}

void SoundLibrary::fromVar (const juce::var& data)
{
    unlockedSoundIds.clear();
    if (data.isArray())
    {
        for (int i = 0; i < data.size(); ++i)
            unlockedSoundIds.insert (data[i].toString());
    }
}

void SoundLibrary::setSoundsDirectory (const juce::File& dir)
{
    soundsDirectory = dir;
}

juce::File SoundLibrary::getSoundsDirectory() const
{
    return soundsDirectory;
}

Tier SoundLibrary::nextTierUp (Tier tier) const
{
    switch (tier)
    {
        case Tier::Common:    return Tier::Uncommon;
        case Tier::Uncommon:  return Tier::Rare;
        case Tier::Rare:      return Tier::Epic;
        case Tier::Epic:      return Tier::Legendary;
        case Tier::Legendary: return Tier::Legendary; // can't go higher
        default:              return Tier::Common;
    }
}

std::vector<const SoundEntry*> SoundLibrary::getAvailableSounds (Tier tier) const
{
    std::vector<const SoundEntry*> result;
    for (const auto& entry : catalog)
    {
        if (entry.tier == tier && ! isSoundUnlocked (entry.id))
            result.push_back (&entry);
    }
    return result;
}

} // namespace tamura
