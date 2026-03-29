# Tamura

A slot machine audio plugin built with JUCE. Spend tokens, spin three reels, win sounds tiered by rarity (Common through Legendary). Every spin pays out — the question is how rare.

Built as VST3, AU, and Standalone formats for macOS and Windows.

## Quick Start

### Prerequisites

```bash
brew install cmake
git clone https://github.com/juce-framework/JUCE.git ../JUCE
```

### Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target Tamura_Standalone
open build/Tamura_artefacts/Debug/Standalone/Tamura.app
```

### Install Test Sounds

Copy the bundled test library to the app support directory:

```bash
mkdir -p ~/Library/Application\ Support/Tamura/Sounds
cp -R Sounds/* ~/Library/Application\ Support/Tamura/Sounds/
```

### Build All Formats

```bash
cmake --build build
```

Outputs: `build/Tamura_artefacts/Debug/{Standalone,VST3,AU}/`

## How It Works

1. You start with 50 tokens. Each spin costs 1.
2. The RNG picks a rarity tier first (Common 40%, Uncommon 30%, Rare 20%, Epic 8%, Legendary 2%), then animates the reels to match.
3. You win a sound from that tier, permanently added to your library.
4. 3 free tokens daily. Save sounds to disk or browse your collection.

See [design.md](design.md) for the full product spec.
