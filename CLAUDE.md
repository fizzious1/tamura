# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is Tamura?

A slot machine audio plugin built with JUCE. Users spend tokens to spin 3 reels and win sounds tiered by rarity (Common through Legendary). See `design.md` for the full product spec, UI direction, and milestones.

## Build Commands

### Prerequisites
```bash
brew install cmake
git clone https://github.com/juce-framework/JUCE.git ../JUCE  # or set JUCE_DIR env var
```

### Build (CMake, recommended)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target Tamura_Standalone   # standalone app only
cmake --build build                               # all targets (VST3, AU, Standalone)
```

Standalone output: `build/Tamura_artefacts/Debug/Standalone/Tamura.app`

### Build (Projucer, alternative)
Open `NewProject/NewProject.jucer` in Projucer, save and open in Xcode, build Standalone target (Cmd+R).

### Adding New Source Files
Add to `target_sources()` in `CMakeLists.txt`. If using Projucer, add through its UI instead.

## Architecture

### Data Flow
```
TamuraAudioProcessor (owns all state and engine objects)
├── SpinEngine       — weighted RNG: picks tier first, then generates matching reel symbols
├── TokenManager     — balance, daily 3-token grant, JSON persistence to app data dir
├── SoundLibrary     — manifest-based catalog (Sounds/manifest.json), unlock tracking, tier fallthrough
└── SamplePlayer     — WAV playback via AudioTransportSource

TamuraAudioProcessorEditor (view layer, reads state via processor's public API)
├── SlotReelComponent (x3) — animated reel gauges
├── SpinButton              — main spin trigger
├── ControlPanel            — preview/stop/save/library buttons
├── WaveformDisplay         — audio waveform visualization
├── NeonLabel               — styled text labels (title, result, token count, status)
└── TamuraLookAndFeel       — custom JUCE LookAndFeel for the entire UI
```

**Key rule:** UI components never access SpinEngine/TokenManager/SoundLibrary directly. They call `TamuraAudioProcessor`'s public API (`performSpin()`, `previewSound()`, `getTokenManager()`, etc.). All mutable state lives in the processor.

### Spin Flow
1. Editor calls `processor.performSpin()`
2. Processor checks `tokenManager.canAffordSpin()`, spends token
3. `spinEngine.spin()` rolls a tier via weighted RNG, generates matching reel symbols
4. `soundLibrary.awardSound(tier)` picks an unowned sound (falls through to next tier up if exhausted)
5. `samplePlayer.loadFile()` + `play()` previews the awarded sound
6. Editor animates reels sequentially (left to right), then displays result

### State Persistence
- DAW state: `getStateInformation`/`setStateInformation` serializes token balance + unlocked sound IDs as JSON
- Standalone: `TokenManager` also persists to a JSON file in the app data directory

## Current State

**Phase 1 complete.** Core spin-to-win loop works: spin button, token spending, tier-based RNG, sound awarding, playback, reel animation, and neon-styled UI.

**Phase 2 (next):** Daily grant UI notification, polished reel animations (easing curves, sequential stop), SoundBrowser panel, tier-specific result effects (shake/glow/particles), UI sound effects. See `design.md` milestones for full scope.

## Code Style

- **Namespace:** `tamura::` for all project-specific code
- **Classes:** PascalCase — `SpinEngine`, `SlotReelComponent`
- **Methods:** camelCase — `spinReels()`, `getTokenBalance()`
- **Members:** camelCase, no prefix — `tokenBalance`, not `m_tokenBalance`
- **Constants:** ALL_CAPS for `#define`, PascalCase for `constexpr`/`static const`
- **Files:** Match class name — `SpinEngine.h`/`SpinEngine.cpp`

## Hard Rules

1. **No code outside class or function bodies.** Every variable, function, and statement must be inside a class definition or function implementation.
2. **No global variables.** All state belongs in a class.
3. **No raw `new`/`delete`.** Use `std::unique_ptr`, `std::shared_ptr`, or JUCE's `OwnedArray`.
4. **No blocking on the audio thread.** `processBlock` must be lock-free and allocation-free. File I/O goes on background threads.
5. **State persistence uses JSON** via `juce::var` and `juce::JSON`. No custom binary formats.
6. **No duplicate definitions.** Every function is defined exactly once.
