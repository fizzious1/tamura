# Tamura — Product Design Document

## Vision

Tamura is a slot machine audio plugin. You spend tokens, spin three reels, and win sounds. Every spin pays out — the question is how good the sound is. Common spins give you bread-and-butter one-shots. Hit a jackpot and you unlock something rare that nobody else is using. It turns sample browsing from a chore into a game.

Built as a VST3/AU/Standalone plugin with JUCE. Runs inside any major DAW.

---

## Core Mechanic

### The Spin

- 3 reels, each with a set of symbols (cherry, lemon, bar, bell, seven, diamond)
- 1 spin costs 1 token
- Reels spin with realistic acceleration, hold, and deceleration
- Every spin awards exactly one sound — you never walk away empty
- The reel outcome determines which **tier** the sound comes from

### Outcome Tiers

| Result | Symbols | Probability | Tier |
|---|---|---|---|
| No match | All different | ~40% | Common |
| 2 match | Two identical | ~30% | Uncommon |
| 3 match | Three identical (non-seven) | ~20% | Rare |
| Jackpot | 7-7-7 | ~8% | Epic |
| Ultra Jackpot | Diamond-Diamond-Diamond | ~2% | Legendary |

Probabilities are server-side or embedded weights — not truly random reels. The RNG picks a tier first, then animates the reels to match.

### Sound Award

After the reels stop:
1. The tier label appears ("RARE!", "JACKPOT!", etc.)
2. The sound plays automatically as a preview
3. The sound is permanently added to the user's library
4. A "Save" button lets them export the .wav to disk or drag it directly into their DAW

Duplicate protection: if the user has already unlocked a sound, re-roll within the same tier so every spin gives something new. When a tier is fully exhausted, fall through to the next tier up.

---

## Sound Design

### Categories (v1)

Tamura ships with four sound categories. Each category exists across all five tiers.

| Category | Examples | Why it's here |
|---|---|---|
| **Drums & Percussion** | Kicks, snares, hats, claps, toms, shakers, rim shots | Universal need across all genres |
| **Melodic One-Shots** | Piano stabs, plucks, synth hits, chord stabs, bell tones | Immediate musicality, easy to use |
| **FX & Textures** | Risers, impacts, sweeps, atmospheres, glitches, noise bursts | High perceived value, clear tier differentiation |
| **Vocal Chops** | Processed vocal hits, shouts, breaths, pitched phrases | Trendy, shareable, personality |

### Tier Quality Guidelines

| Tier | Character | Production quality | Example |
|---|---|---|---|
| **Common** | Clean, functional, no surprises | Stock-level, well-recorded but generic | A solid 808 kick, a clean hi-hat |
| **Uncommon** | Has some personality, a bit of flavor | Above average, slight processing character | A vinyl-textured snare, a detuned pluck |
| **Rare** | Distinctive, you'd build a track around it | Professional, polished, unique processing | A heavily layered cinematic impact, a granular vocal texture |
| **Epic** | Signature quality, immediately recognizable | Top-tier, boutique sound design | A morphing bass hit with analog saturation, a processed vocal chop with reverb tail |
| **Legendary** | One-of-a-kind, conversation starter | Exceptional, could anchor an entire production | A rare vintage synth sample, a hand-crafted multi-layered texture with movement and depth |

### Sound Count (v1 Launch Target)

| Tier | Count per category | Total |
|---|---|---|
| Common | 25 | 100 |
| Uncommon | 15 | 60 |
| Rare | 10 | 40 |
| Epic | 5 | 20 |
| Legendary | 2 | 8 |
| **Total** | **57** | **228** |

### Sound Sourcing

- **Commission sound designers** on SoundBetter or Fiverr for Common through Rare tiers
- **Hire 1-2 specialist designers** for Epic and Legendary tiers — these must feel hand-crafted
- All sounds must be original, royalty-free, cleared for commercial use by end users
- Deliver as 44.1kHz / 24-bit WAV, normalized, trimmed, with consistent loudness per tier

---

## Token Economy

### Earning Tokens

| Method | Tokens | Frequency |
|---|---|---|
| Daily login | 3 | Once per day (resets at midnight UTC) |
| Share on social media | 5 | Once per week per platform |
| Refer a friend (they buy plugin) | 15 | Per referral, no cap |

Daily tokens create a habit loop. Users open the plugin, collect 3 tokens, spin, and stay engaged — even without paying.

### Buying Tokens

| Pack | Price | Tokens | Per-spin cost |
|---|---|---|---|
| Starter | $4.99 | 25 | $0.20 |
| Regular | $9.99 | 60 | $0.17 |
| Big Spender | $19.99 | 150 | $0.13 |
| High Roller | $39.99 | 400 | $0.10 |

Token purchases happen on the Tamura website. The plugin links out to a purchase page. After purchase, the user enters a redemption code or the balance syncs via a lightweight account system (email + token balance stored server-side or locally with signed receipts).

### Token Persistence

- Token balance is stored locally in a JSON file in the user's app data directory
- For v1, no server-side account is required — keep it simple
- The JSON file stores: token balance, daily login timestamp, unlocked sound IDs
- Future versions can add cloud sync and anti-cheat

---

## Pricing

### Plugin Purchase

| Edition | Price | Includes |
|---|---|---|
| **Standard** | $29 | Plugin (VST3 + AU + Standalone) + 50 tokens |
| **Premium** | $49 | Plugin + 150 tokens + 10 bonus Rare sounds pre-unlocked |

### Revenue Model

The plugin purchase is the entry point. Token packs are the recurring revenue engine.

Conservative projection at 1,000 users:
- Launch revenue: ~$29,000 (assuming mostly Standard)
- Monthly recurring: ~$3,000 (30% of users buy one pack/month at ~$10 avg)
- Daily free tokens ensure non-paying users stay engaged and eventually convert

---

## Distribution

### Launch Channels

| Channel | Purpose | Priority |
|---|---|---|
| **Own website** (Gumroad or Shopify + Stripe) | Full margin, token sales, direct relationship | Primary |
| **Plugin Boutique** | Largest discovery platform for plugins | Secondary |
| **KVR Audio** | Free listing, community credibility, SEO | Day-one |
| **ADSR Sounds** | Secondary marketplace | Post-launch |

### Installer Deliverables

| Platform | Formats | Installer |
|---|---|---|
| macOS | VST3, AU, Standalone | .pkg installer |
| Windows | VST3, Standalone | .exe installer (Inno Setup or similar) |

### DAW Compatibility Targets

Ableton Live, Logic Pro, FL Studio, Reaper, Cubase, Studio One, Bitwig.

Test matrix: each format in each DAW on each OS before release.

---

## UI Design

### Visual Direction

**Retro-modern arcade.** Think Japanese pachinko meets brushed-metal hardware. Warm, tactile, physical. Not flat-minimal, not skeuomorphic-Apple-2012, not neon-gamer, not AI-generated-gradient.

### Color Palette

- **Primary:** Deep gold (#C5963A), warm brass
- **Secondary:** Rich crimson (#8B1A1A), burgundy
- **Background:** Dark walnut wood texture, near-black (#1A1410)
- **Accents:** Cream white (#F5E6C8) for text, soft amber for glow effects
- **Highlights:** Bright gold (#FFD700) for jackpot moments

### Typography

- **Display font:** One bold, slightly retro display face for "JACKPOT!", tier names, and the logo. Something with weight and character — not a default sans-serif.
- **UI font:** One clean, readable sans-serif for labels, token counts, and buttons. Inter, DM Sans, or similar.
- **Maximum two fonts total.**

### Layout (600 x 450 pixels)

```
+------------------------------------------------------+
|  TAMURA                          TOKENS: 37           |
|------------------------------------------------------|
|                                                      |
|   +------------+  +------------+  +------------+     |
|   |            |  |            |  |            |     |
|   |   REEL 1   |  |   REEL 2   |  |   REEL 3   |     |
|   |            |  |            |  |            |     |
|   +------------+  +------------+  +------------+     |
|                                                      |
|             +----------------------+                  |
|             |     S P I N  (1)     |                  |
|             +----------------------+                  |
|                                                      |
|   LAST WIN: Rare - "Vinyl Crunch Snare"              |
|   [ > Preview ]   [ Save to Disk ]                   |
|                                                      |
|------------------------------------------------------|
|   [ My Sounds (23/228) ]    [ Buy Tokens ]           |
+------------------------------------------------------+
```

### Key UI Components

1. **Reels** — Three vertical strips with symbols. Spin animation uses easing curves (fast start, gradual deceleration, slight overshoot-and-settle on stop). Each reel stops sequentially left-to-right with a brief delay between them.

2. **Spin Button** — Large, centered, tactile. Shows token cost "(1)". Disabled + greyed out when balance is 0. Satisfying press animation (scale down on click, bounce back on release).

3. **Token Display** — Top-right corner. Shows current balance as a number. Brief coin-flip animation when balance changes.

4. **Result Banner** — Appears after spin completes. Shows tier name with tier-appropriate styling:
   - Common: simple white text
   - Uncommon: green glow
   - Rare: blue glow + slight screen flash
   - Epic: purple glow + screen shake
   - Legendary: gold glow + particle burst + extended fanfare

5. **Preview & Save** — Below the result. Preview plays the sound. Save exports the .wav or enables drag-to-DAW.

6. **Sound Library Panel** — Expandable bottom panel or separate view. Grid of unlocked sounds organized by category and tier. Click to preview, drag to export.

7. **Buy Tokens Button** — Opens the browser to the purchase page. Not pushy — always visible but never intrusive.

### Sound Effects (UI audio, not the awarded sounds)

- Reel spin: mechanical clicking/whirring
- Reel stop: satisfying thunk per reel
- Win (Common/Uncommon): brief coin clink
- Win (Rare): ascending chime
- Win (Epic): slot machine bell ring
- Win (Legendary): full jackpot fanfare (3-4 seconds)
- Button click: subtle tactile pop

### Anti-"AI Look" Rules

- No smooth gradients that look like diffusion model outputs
- No perfect symmetry everywhere — add subtle imperfections to textures
- No glassmorphism or frosted-glass panels
- No generic stock-illustration icons
- Hand-craft or commission the reel symbols and textures
- Use real material references: actual wood grain, actual brushed metal, actual felt
- Typography should have personality — not just system fonts

### Design Execution

Hire a **game UI designer** (not a web/app designer). Game UI designers understand tactile, physical interfaces. Source from Dribbble or ArtStation. Budget: $500-$1,500 for the complete plugin skin including all states and animations as mockups.

---

## Technical Architecture (High-Level)

### Component Map

```
NewProjectAudioProcessor (PluginProcessor)
├── SpinEngine          — RNG, tier selection, reel outcome generation
├── TokenManager        — Balance tracking, daily login, persistence
├── SoundLibrary        — Sound catalog, tier mapping, unlock tracking
└── SamplePlayer        — Audio file loading, playback, transport

NewProjectAudioProcessorEditor (PluginEditor)
├── SlotReelComponent   — 3 animated reels with symbol rendering
├── SpinButtonComponent — The main spin button with press animation
├── TokenDisplay        — Current balance readout
├── ResultBanner        — Tier announcement + sound preview
├── SoundBrowser        — Library panel for browsing unlocked sounds
└── LookAndFeel         — Custom JUCE LookAndFeel for the entire UI
```

### State Persistence

Plugin state saved via `getStateInformation` / `setStateInformation`:
- Current token balance
- Last daily login timestamp
- List of unlocked sound IDs
- User preferences (volume, last selected category)

Stored as a JSON blob inside the JUCE binary state, plus a backup JSON file in the app data directory for cross-session persistence outside the DAW.

### Sound File Management

- Sounds ship as a bundle alongside the plugin installer
- Installed to a known location (e.g., `~/Library/Application Support/Tamura/Sounds/` on macOS)
- Organized as: `Sounds/{category}/{tier}/sound_name.wav`
- A manifest JSON maps sound IDs to file paths, tier, category, and metadata

---

## Milestones

### Phase 1: Foundation (Weeks 1-2)
- Rewrite processor and editor from clean JUCE template
- Implement SpinEngine with weighted RNG
- Implement SamplePlayer with basic file loading and playback
- Basic UI: 3 static reels + spin button + console output of results

### Phase 2: Token System (Weeks 3-4)
- Implement TokenManager with balance, spend, earn logic
- Daily login token grant
- JSON persistence for tokens and unlocked sounds
- Implement SoundLibrary catalog with tier/category mapping

### Phase 3: Full UI (Weeks 4-6)
- Reel spin animation with easing and sequential stop
- Result banner with tier-specific effects
- Token display with live balance
- Sound preview and save/export
- Custom LookAndFeel with the design palette

### Phase 4: Sound Library Panel (Weeks 6-7)
- Browsable grid of unlocked sounds
- Filter by category and tier
- Click-to-preview, drag-to-DAW export
- Progress indicator (X/228 unlocked)

### Phase 5: Polish & Distribution (Weeks 7-9)
- UI sound effects for spins, wins, clicks
- Installer creation (macOS .pkg, Windows .exe)
- DAW compatibility testing across the full matrix
- Website setup (Gumroad or Shopify)
- Plugin Boutique and KVR listings
- Marketing assets: screenshots, demo video, social posts

### Phase 6: Launch
- Publish on own site + Plugin Boutique + KVR
- Social media announcement
- Send to YouTube plugin reviewers
- Monitor feedback, patch bugs, plan v1.1 content update (new sounds)
