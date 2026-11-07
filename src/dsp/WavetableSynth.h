#pragma once

#include "../SynthParameters.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>

namespace wave
{
class WavetableBank;
class ImportedWavetable;
class WavetableImports;
float lfoSample(double phase, float rampAmount) noexcept;

class SynthEngine
{
public:
    struct Modulation
    {
        float phase;
        float position;
        float position2;
        float cutoff;
        bool playing;
    };

    SynthEngine(WavetableImports& mainImports, WavetableImports& imports);
    void prepare(double sampleRate, const Settings& settings);
    void reset() noexcept;
    void process(juce::AudioBuffer<float>& audio, const juce::MidiBuffer& midi,
                 Settings settings, double tempo) noexcept;

    Modulation modulation() const noexcept;
    float preview(float phase, float position, int tableIndex, const ImportedWavetable* imported) const noexcept;

private:
    static constexpr std::size_t unisonCount = 7;
    static constexpr std::size_t polyphony = 8;

    struct Stereo
    {
        float left = 0.0f;
        float right = 0.0f;
    };

    struct Filter
    {
        float integrator1 = 0.0f;
        float integrator2 = 0.0f;

        float process(float input, float g, float damping) noexcept;
    };

    struct Voice
    {
        std::array<double, unisonCount> phases {};
        double layerPhase = 0.0;
        double lfoPhase = 0.0;
        double shotPhase = 0.0;
        double frequency = 0.0;
        juce::ADSR envelope;
        Filter leftFilter;
        Filter rightFilter;
        Stereo lastSample;
        Stereo stealTail;
        float lastAmplitude = 0.0f;
        float stealAmplitude = 0.0f;
        float velocity = 0.0f;
        int tailSamples = 0;
        int note = 0;
        int channel = 0;
        std::uint64_t age = 0;
        bool held = false;
        bool sustained = false;
    };

    void setTargets(Settings settings, bool immediate) noexcept;
    void handleMidi(juce::MidiMessageMetadata event, const Settings& current) noexcept;
    void startNote(int channel, int note, int velocity, const Settings& current) noexcept;
    void stopNote(int channel, int note) noexcept;
    void handleController(int channel, int controller, int value) noexcept;
    void silence(Voice& voice) noexcept;

    const WavetableBank& tables;
    WavetableImports& mainImports;
    WavetableImports& imports;
    std::array<Voice, polyphony> voices;
    std::array<bool, 16> sustainPedal {};
    std::array<juce::SmoothedValue<float>, parameterCount> smooth;
    std::array<juce::SmoothedValue<float>, unisonCount> offsets;
    std::array<juce::SmoothedValue<float>, unisonCount> weights;
    std::vector<Stereo> delayLine;
    std::size_t delayWrite = 0;
    double rate = 44100.0;
    double freePhase = 0.0;
    std::uint64_t noteCounter = 0;
    int fadeSamples = 1;

    std::atomic<float> displayPhase { 0.0f };
    std::atomic<float> displayPosition { 0.0f };
    std::atomic<float> displayPosition2 { 0.0f };
    std::atomic<float> displayCutoff { 20000.0f };
    std::atomic<bool> displayPlaying { false };
};
}
