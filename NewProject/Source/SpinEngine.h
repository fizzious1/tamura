#pragma once

#include <juce_core/juce_core.h>
#include <array>

namespace tamura
{

enum class Tier
{
    Common,     // ~40%
    Uncommon,   // ~30%
    Rare,       // ~20%
    Epic,       // ~8%
    Legendary   // ~2%
};

inline juce::String tierToString (Tier tier)
{
    switch (tier)
    {
        case Tier::Common:    return "Common";
        case Tier::Uncommon:  return "Uncommon";
        case Tier::Rare:      return "Rare";
        case Tier::Epic:      return "Epic";
        case Tier::Legendary: return "Legendary";
        default:              return "Unknown";
    }
}

// Symbols that appear on the reels
enum class ReelSymbol
{
    Cherry,
    Lemon,
    Bar,
    Bell,
    Seven,
    Diamond
};

inline juce::String symbolToString (ReelSymbol symbol)
{
    switch (symbol)
    {
        case ReelSymbol::Cherry:  return "Cherry";
        case ReelSymbol::Lemon:   return "Lemon";
        case ReelSymbol::Bar:     return "Bar";
        case ReelSymbol::Bell:    return "Bell";
        case ReelSymbol::Seven:   return "Seven";
        case ReelSymbol::Diamond: return "Diamond";
        default:                  return "?";
    }
}

struct SpinResult
{
    Tier tier;
    std::array<ReelSymbol, 3> reels;
};

// Weighted RNG for tier selection. Generates reel symbols to match.
class SpinEngine
{
public:
    SpinEngine();

    // Perform a spin. Returns the resulting tier and reel symbols.
    SpinResult spin();

    // Get the probability weight for a given tier (0.0 to 1.0)
    static float getProbability (Tier tier);

private:
    // Pick a tier based on weighted random distribution
    Tier rollTier();

    // Generate reel symbols that visually match the chosen tier
    std::array<ReelSymbol, 3> generateReels (Tier tier);

    // Pick a random symbol (excluding specific ones if needed)
    ReelSymbol randomSymbol();
    ReelSymbol randomSymbolExcluding (ReelSymbol exclude);

    juce::Random rng;
};

} // namespace tamura
