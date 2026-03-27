# CLAUDE.md — Tamura Project Instructions

## What is Tamura?

A slot machine audio plugin built with JUCE. Users spend tokens to spin 3 reels and win sounds tiered by rarity (Common through Legendary). See `design.md` for the full product spec.

## Before You Start

**Read `design.md` first.** It contains the product vision, mechanics, sound tiers, token economy, UI direction, and milestones. Every implementation decision should trace back to that document.

## Current State (Phase 1 complete)

Phase 1 code is written. The following components exist and are wired together:

- **TamuraAudioProcessor** — owns all engine components, exposes `performSpin()` API
- **TamuraAudioProcessorEditor** — functional UI with spin button, reel animation, token display, result banner
- **SpinEngine** — weighted RNG, tier-first selection, reel symbol generation
- **SamplePlayer** — WAV loading via AudioTransportSource, play/stop/preview
- **TokenManager** — balance tracking, daily 3-token grant, JSON persistence to app data dir
- **SoundLibrary** — manifest-based catalog, unlock tracking, tier fallthrough on exhaustion

**Not yet built** (Phase 2-3):
- `UI/` subdirectory with dedicated components (SlotReelComponent, TamuraLookAndFeel, etc.)
- Polished visual design (current UI is functional but plain)
- Sound browser / library panel
- Drag-to-DAW export
- Installer packaging

## Tech Stack

- **Framework:** JUCE 7+ (latest stable)
- **Language:** C++17
- **Project format:** `.jucer` managed by Projucer
- **Build systems:** Xcode (macOS), Visual Studio (Windows)
- **Target formats:** VST3, AU, Standalone
- **Audio format:** WAV (44.1kHz / 24-bit)

## Repo Structure

```
tamura/
├── design.md                    # Product spec (read this first)
├── CLAUDE.md                    # This file
├── NewProject/
│   ├── NewProject.jucer         # Projucer project file
│   ├── Source/
│   │   ├── PluginProcessor.h    # Main processor header
│   │   ├── PluginProcessor.cpp  # Main processor implementation
│   │   ├── PluginEditor.h       # Main editor header
│   │   ├── PluginEditor.cpp     # Main editor implementation
│   │   ├── SpinEngine.h         # RNG, tier selection, reel logic
│   │   ├── SpinEngine.cpp
│   │   ├── TokenManager.h       # Token balance, daily grants, persistence
│   │   ├── TokenManager.cpp
│   │   ├── SoundLibrary.h       # Sound catalog, unlocks, tier mapping
│   │   ├── SoundLibrary.cpp
│   │   ├── SamplePlayer.h       # Audio loading and playback
│   │   ├── SamplePlayer.cpp
│   │   └── UI/
│   │       ├── SlotReelComponent.h/.cpp    # Reel animation
│   │       ├── SpinButtonComponent.h/.cpp  # Spin button
│   │       ├── TokenDisplay.h/.cpp         # Balance readout
│   │       ├── ResultBanner.h/.cpp         # Win announcement
│   │       ├── SoundBrowser.h/.cpp         # Library panel
│   │       └── TamuraLookAndFeel.h/.cpp    # Custom LookAndFeel
│   ├── Assets/                  # Embedded images, fonts, UI sounds
│   ├── JuceLibraryCode/         # Auto-generated (do not edit)
│   └── Builds/
│       ├── MacOSX/              # Xcode project (auto-generated)
│       └── VisualStudio2022/    # VS project (auto-generated)
└── Sounds/                      # Sound library (not in plugin binary)
    ├── manifest.json            # Maps sound IDs to files, tiers, categories
    ├── Drums/
    │   ├── Common/
    │   ├── Uncommon/
    │   ├── Rare/
    │   ├── Epic/
    │   └── Legendary/
    ├── Melodic/
    ├── FX/
    └── Vocals/
```

## Architecture Rules

### Separation of Concerns

- **PluginProcessor** owns SpinEngine, TokenManager, SoundLibrary, and SamplePlayer as member objects
- **PluginEditor** owns all UI components and reads state from the processor
- UI components never access SpinEngine/TokenManager/SoundLibrary directly — they go through the processor's public API
- All mutable state lives in the processor or its owned managers. The editor is a view.

### Code Style

- **Classes:** PascalCase (`SpinEngine`, `TokenManager`, `SlotReelComponent`)
- **Methods:** camelCase (`spinReels()`, `getTokenBalance()`, `loadSound()`)
- **Member variables:** camelCase with no prefix (`tokenBalance`, not `m_tokenBalance` or `_tokenBalance`)
- **Constants:** ALL_CAPS for `#define`, PascalCase for `constexpr` / `static const`
- **Files:** Match class name. `SpinEngine.h` / `SpinEngine.cpp`
- **Namespaces:** Use `tamura::` namespace for all project-specific code

### Hard Rules

1. **No code outside class or function bodies.** Every variable, function, and statement must be inside a class definition or function implementation.
2. **No global variables.** All state belongs in a class.
3. **No duplicate definitions.** Every function is defined exactly once.
4. **No raw `new`/`delete`.** Use `std::unique_ptr`, `std::shared_ptr`, or JUCE's `OwnedArray`.
5. **No blocking operations on the audio thread.** File I/O, allocations, and locking go on a background thread. `processBlock` must be lock-free and allocation-free.
6. **State persistence uses JSON** via `juce::var` and `juce::JSON`. No custom binary formats.

## Build Instructions

### Prerequisites

Install CMake and clone JUCE:

```bash
brew install cmake
git clone https://github.com/juce-framework/JUCE.git ../JUCE
```

### Build with CMake (recommended)

```bash
cd tamura
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target Tamura_Standalone
```

The Standalone app will be at `build/Tamura_artefacts/Debug/Standalone/Tamura.app`.

For all targets (VST3, AU, Standalone):
```bash
cmake --build build
```

### Build with Projucer (alternative)

1. Open Projucer (from JUCE installation)
2. Open `NewProject/NewProject.jucer`
3. Verify export targets include Xcode
4. Click "Save and Open in IDE"
5. In Xcode: select the Standalone target, build and run (Cmd+R)
6. For AU/VST3: build the respective targets, copy to `/Library/Audio/Plug-Ins/`

### Adding New Source Files

When using CMake: add the file to `target_sources()` in `CMakeLists.txt`.
When using Projucer: add through Projucer UI, then save (regenerates IDE projects).

## Implementation Order

Follow the phases in `design.md` milestones section. For each phase:

1. **Read** the relevant section of design.md
2. **Implement** the core logic classes first, then the UI
3. **Test in Standalone** before testing in a DAW host
4. **Verify state persistence** — close and reopen the plugin, confirm data survives

### Phase 1 specifics (DONE)

All items below are complete:
1. ~~Clean up PluginProcessor.h/cpp~~ — rewritten as `TamuraAudioProcessor`
2. ~~Clean up PluginEditor.h/cpp~~ — rewritten as `TamuraAudioProcessorEditor`
3. ~~Create SpinEngine~~ — `spin()` returns `SpinResult` with tier + reel symbols
4. ~~Create SamplePlayer~~ — wraps `AudioTransportSource` for WAV playback
5. ~~Wire spin button~~ — editor spin -> `processor.performSpin()` -> award + play sound
6. Needs verification: compile and run as Standalone (requires JUCE installation)

### Phase 2 specifics (start here)

1. Implement TokenManager daily grant UI notification
2. Create `UI/SlotReelComponent` — dedicated reel with smooth animation (easing curves, sequential stop)
3. Create `UI/TamuraLookAndFeel` — custom colors, button styling, fonts
4. Create `UI/SoundBrowser` — grid of unlocked sounds with preview
5. Create `UI/ResultBanner` — tier-specific animations (shake, glow, particles)
6. Add UI sound effects for spins and wins

## Testing Checklist

Before considering any phase complete:

- [ ] Builds without warnings on macOS (Xcode)
- [ ] Runs as Standalone without crashes
- [ ] Loads as AU in Logic Pro / GarageBand
- [ ] Loads as VST3 in Ableton Live / Reaper
- [ ] State persists across DAW save/load (getStateInformation / setStateInformation)
- [ ] No audio glitches, clicks, or pops during playback
- [ ] No memory leaks (use Xcode Instruments or AddressSanitizer)
- [ ] UI is responsive — no freezes during spin animation or file loading

## Sound Library Setup (for testing)

For development, create a minimal test library:
- 5 sounds per tier, 1 category (Drums)
- Place in `Sounds/Drums/{Tier}/` with short descriptive filenames
- Create a `manifest.json` following this schema:

```json
{
  "sounds": [
    {
      "id": "drum_common_001",
      "name": "Clean Kick",
      "category": "Drums",
      "tier": "Common",
      "file": "Drums/Common/clean_kick.wav"
    }
  ]
}
```

## Key Decisions Log

| Decision | Choice | Rationale |
|---|---|---|
| Token storage | Local JSON file | No server needed for v1, simplifies launch |
| RNG approach | Pick tier first, then animate reels to match | Ensures correct probability distribution |
| Sound delivery | Bundled installer, not streamed | Offline-first, no server dependency |
| UI framework | JUCE native components + custom LookAndFeel | No web views, stays within JUCE ecosystem |
| Plugin name in code | Renamed to "Tamura" | .jucer updated, class renamed to TamuraAudioProcessor |
| Sound library for dev | 20 entries across Drums + FX, 5 tiers | manifest.json in Sounds/ with placeholder WAVs |
