#include "TokenManager.h"

namespace tamura
{

TokenManager::TokenManager()
{
    loadState();
    claimDailyTokens();
}

int TokenManager::getBalance() const
{
    return tokenBalance;
}

bool TokenManager::canAffordSpin() const
{
    return tokenBalance >= TokensPerSpin;
}

bool TokenManager::spendToken()
{
    if (! canAffordSpin())
        return false;

    tokenBalance -= TokensPerSpin;
    saveState();
    return true;
}

void TokenManager::addTokens (int amount)
{
    if (amount > 0)
    {
        tokenBalance += amount;
        saveState();
    }
}

int TokenManager::claimDailyTokens()
{
    auto now = juce::Time::getCurrentTime();

    if (isSameDay (now, lastDailyClaimDate))
        return 0;

    lastDailyClaimDate = now;
    tokenBalance += DailyTokenGrant;
    saveState();
    return DailyTokenGrant;
}

void TokenManager::saveState()
{
    auto file = getStateFile();
    file.getParentDirectory().createDirectory();

    auto data = toVar();
    auto jsonString = juce::JSON::toString (data);
    file.replaceWithText (jsonString);
}

void TokenManager::loadState()
{
    auto file = getStateFile();
    if (! file.existsAsFile())
        return;

    auto jsonString = file.loadFileAsString();
    auto data = juce::JSON::parse (jsonString);

    if (data.isObject())
        fromVar (data);
}

juce::File TokenManager::getStateFile() const
{
    auto appDataDir = juce::File::getSpecialLocation (
        juce::File::userApplicationDataDirectory);

#if JUCE_MAC
    auto tamuraDir = appDataDir.getChildFile ("Application Support").getChildFile ("Tamura");
#else
    auto tamuraDir = appDataDir.getChildFile ("Tamura");
#endif

    return tamuraDir.getChildFile ("token_state.json");
}

juce::var TokenManager::toVar() const
{
    auto obj = std::make_unique<juce::DynamicObject>();
    obj->setProperty ("tokenBalance", tokenBalance);
    obj->setProperty ("lastDailyClaim", lastDailyClaimDate.toMilliseconds());
    return juce::var (obj.release());
}

void TokenManager::fromVar (const juce::var& data)
{
    if (data.hasProperty ("tokenBalance"))
        tokenBalance = static_cast<int> (data["tokenBalance"]);

    if (data.hasProperty ("lastDailyClaim"))
        lastDailyClaimDate = juce::Time (static_cast<juce::int64> (data["lastDailyClaim"]));
}

bool TokenManager::isSameDay (juce::Time a, juce::Time b) const
{
    return a.getYear() == b.getYear()
        && a.getMonth() == b.getMonth()
        && a.getDayOfMonth() == b.getDayOfMonth();
}

} // namespace tamura
