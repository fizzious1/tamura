#pragma once

#include <juce_core/juce_core.h>

namespace tamura
{

// Manages the user's token balance. Handles spending, earning, daily grants,
// and persists state to a JSON file in the app data directory.
class TokenManager
{
public:
    TokenManager();

    // Token balance
    int getBalance() const;
    bool canAffordSpin() const;

    // Spend 1 token for a spin. Returns false if insufficient balance.
    bool spendToken();

    // Add tokens (from purchase, referral, etc.)
    void addTokens (int amount);

    // Check and grant daily login tokens (3 per day).
    // Call this when the plugin loads. Returns number of tokens granted (0 if already claimed today).
    int claimDailyTokens();

    // Persistence
    void saveState();
    void loadState();

    // Get the file where state is persisted
    juce::File getStateFile() const;

    // Serialization for DAW state (getStateInformation / setStateInformation)
    juce::var toVar() const;
    void fromVar (const juce::var& data);

    static constexpr int TokensPerSpin = 1;
    static constexpr int DailyTokenGrant = 3;
    static constexpr int StarterTokens = 50;

private:
    int tokenBalance = StarterTokens;
    juce::Time lastDailyClaimDate;

    bool isSameDay (juce::Time a, juce::Time b) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TokenManager)
};

} // namespace tamura
