#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * WaveformDisplay — oscilloscope-style waveform viewer.
 *
 * Renders the currently loaded audio buffer as a centered waveform on a dark
 * background with grid lines, scanline overlay, metallic bezel, and decorative
 * LED indicators.  When no buffer is set a flat line with subtle noise is drawn.
 *
 * Colour palette (from cyberpunk / neon design language):
 *   Background  #0A0A14
 *   Waveform    #00BFFF  (electric blue, with glow)
 *   Bezel       gradient  #606068 -> #909098
 */
namespace tamura
{

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override = default;

    // ── Public API ──────────────────────────────────────────────────────

    /** Set (or clear) the audio buffer to display.  Passing nullptr clears. */
    void setAudioBuffer (const juce::AudioBuffer<float>* buffer);

    /** Set the zoom level: how many source samples map to one horizontal pixel. */
    void setSamplesPerPixel (int spp);

    // ── Component overrides ─────────────────────────────────────────────

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    // ── Painting helpers ────────────────────────────────────────────────

    /** Returns the inner rectangle (inside the bezel). */
    juce::Rectangle<float> getScreenArea() const;

    void paintBezel         (juce::Graphics& g);
    void paintLEDs          (juce::Graphics& g, juce::Rectangle<float> bezelRect);
    void paintBackground    (juce::Graphics& g, juce::Rectangle<float> area);
    void paintGrid          (juce::Graphics& g, juce::Rectangle<float> area);
    void paintWaveform      (juce::Graphics& g, juce::Rectangle<float> area);
    void paintIdleLine      (juce::Graphics& g, juce::Rectangle<float> area);
    void paintScanlines     (juce::Graphics& g, juce::Rectangle<float> area);

    // ── Data ────────────────────────────────────────────────────────────

    const juce::AudioBuffer<float>* audioBuffer = nullptr;
    int samplesPerPixel = 256;

    static constexpr float bezelThickness = 6.0f;

    // Palette
    static constexpr juce::uint32 colBackground   = 0xFF0A0A14;
    static constexpr juce::uint32 colWaveform      = 0xFF00BFFF;
    static constexpr juce::uint32 colBezelDark     = 0xFF606068;
    static constexpr juce::uint32 colBezelLight    = 0xFF909098;
    static constexpr juce::uint32 colGrid          = 0x18FFFFFF;  // very faint white
    static constexpr juce::uint32 colScanline      = 0x0CFFFFFF;  // even fainter

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};

} // namespace tamura
