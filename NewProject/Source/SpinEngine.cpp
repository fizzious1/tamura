#include "SpinEngine.h"

namespace tamura
{

SpinEngine::SpinEngine()
    : rng (juce::Random::getSystemRandom())
{
}

SpinResult SpinEngine::spin()
{
    auto tier = rollTier();
    auto reels = generateReels (tier);
    return { tier, reels };
}

float SpinEngine::getProbability (Tier tier)
{
    switch (tier)
    {
        case Tier::Common:    return 0.40f;
        case Tier::Uncommon:  return 0.30f;
        case Tier::Rare:      return 0.20f;
        case Tier::Epic:      return 0.08f;
        case Tier::Legendary: return 0.02f;
        default:              return 0.0f;
    }
}

Tier SpinEngine::rollTier()
{
    float roll = rng.nextFloat(); // 0.0 to 1.0
    float cumulative = 0.0f;

    // Walk through tiers in order, accumulating probability
    static constexpr Tier tiers[] = {
        Tier::Common, Tier::Uncommon, Tier::Rare, Tier::Epic, Tier::Legendary
    };

    for (auto tier : tiers)
    {
        cumulative += getProbability (tier);
        if (roll < cumulative)
            return tier;
    }

    return Tier::Common; // fallback
}

std::array<ReelSymbol, 3> SpinEngine::generateReels (Tier tier)
{
    switch (tier)
    {
        case Tier::Legendary:
        {
            // Diamond-Diamond-Diamond
            return { ReelSymbol::Diamond, ReelSymbol::Diamond, ReelSymbol::Diamond };
        }

        case Tier::Epic:
        {
            // 7-7-7
            return { ReelSymbol::Seven, ReelSymbol::Seven, ReelSymbol::Seven };
        }

        case Tier::Rare:
        {
            // Three matching (not Seven or Diamond)
            ReelSymbol choices[] = { ReelSymbol::Cherry, ReelSymbol::Lemon, ReelSymbol::Bar, ReelSymbol::Bell };
            auto symbol = choices[rng.nextInt (4)];
            return { symbol, symbol, symbol };
        }

        case Tier::Uncommon:
        {
            // Two matching, one different
            auto matchSymbol = randomSymbol();
            auto oddSymbol = randomSymbolExcluding (matchSymbol);
            int oddPosition = rng.nextInt (3);

            std::array<ReelSymbol, 3> result = { matchSymbol, matchSymbol, matchSymbol };
            result[(size_t) oddPosition] = oddSymbol;
            return result;
        }

        case Tier::Common:
        default:
        {
            // All different
            auto first = randomSymbol();
            auto second = randomSymbolExcluding (first);
            ReelSymbol third;
            do {
                third = randomSymbol();
            } while (third == first || third == second);

            return { first, second, third };
        }
    }
}

ReelSymbol SpinEngine::randomSymbol()
{
    return static_cast<ReelSymbol> (rng.nextInt (6));
}

ReelSymbol SpinEngine::randomSymbolExcluding (ReelSymbol exclude)
{
    ReelSymbol result;
    do {
        result = randomSymbol();
    } while (result == exclude);
    return result;
}

} // namespace tamura
