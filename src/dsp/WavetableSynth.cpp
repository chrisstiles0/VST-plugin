#include "WavetableSynth.h"

#include <algorithm>
#include <cmath>

namespace wave
{
namespace
{
constexpr float pi = juce::MathConstants<float>::pi;
constexpr double twoPi = juce::MathConstants<double>::twoPi;
constexpr int frameCount = 4;
constexpr int levelCount = 9;
constexpr int tableSize = 2048;
constexpr int tableMask = tableSize - 1;

double wrap(double phase) noexcept
{
    return phase - std::floor(phase);
}

float lfo(double phase) noexcept
{
    return static_cast<float>(std::sin(twoPi * phase));
}

float harmonicAmplitude(int bank, int frame, int harmonic)
{
    const auto h = static_cast<float>(harmonic);
    const bool odd = harmonic % 2 != 0;
    switch (frame)
    {
        case 0: return harmonic == 1 ? 1.0f : 0.0f;
        case 1: return odd ? 8.0f / (pi * pi * h * h) * (harmonic % 4 == 1 ? 1.0f : -1.0f) : 0.0f;
        case 2: return bank == 0 ? 2.0f / (pi * h)
                                 : (odd ? 4.0f / (pi * h) : 0.0f);
        case 3: return bank == 0 ? (odd ? 4.0f / (pi * h) : 0.0f)
                                 : 4.0f * std::sin(pi * h * 0.25f) / (pi * h);
        default: return 0.0f;
    }
}

float limit(float sample) noexcept
{
    const auto excess = std::abs(sample) - 0.9f;
    return excess > 0.0f ? std::copysign(0.9f + 0.1f * excess / (0.1f + excess), sample)
                         : sample;
}
}

class WavetableBank
{
public:
    WavetableBank()
    {
        // Accumulate Fourier partials once, retaining an octave of bandwidth per level.
        for (int frame = 0; frame < frameCount; ++frame)
        {
            std::array<Wave, 2> sums {};
            for (int harmonic = 1; harmonic <= (1 << (levelCount - 1)); ++harmonic)
            {
                const auto classic = harmonicAmplitude(0, frame, harmonic);
                const auto hollow = harmonicAmplitude(1, frame, harmonic);
                if (classic != 0.0f || hollow != 0.0f)
                {
                    for (int sample = 0; sample < tableSize; ++sample)
                    {
                        const auto partial = static_cast<float>(std::sin(twoPi * harmonic * sample / tableSize));
                        sums[0][sample] += classic * partial;
                        sums[1][sample] += hollow * partial;
                    }
                }

                if ((harmonic & (harmonic - 1)) == 0)
                {
                    const auto level = juce::findHighestSetBit(static_cast<juce::uint32>(harmonic));
                    for (int bank = 0; bank < 2; ++bank)
                        data[bank][level][frame] = sums[bank];
                }
            }

            for (auto& bank : data)
            {
                const auto& full = bank.back()[frame];
                const auto [minimum, maximum] = std::minmax_element(full.begin(), full.end());
                const auto scale = 1.0f / std::max(std::abs(*minimum), std::abs(*maximum));
                for (auto& level : bank)
                    for (auto& sample : level[frame])
                        sample *= scale;
            }
        }
    }

    float render(double phase, double step, float framePosition, float blend) const noexcept
    {
        constexpr double bandwidth = 0.45;
        if (step >= bandwidth)
            return 0.0f;

        // Crossfade only between levels whose highest partial is inside the guard band.
        int exponent = 0;
        const auto fraction = std::frexp(bandwidth / step, &exponent);
        const auto upper = std::min(exponent - 1, levelCount - 1);
        const auto mix = exponent > levelCount ? 1.0f : static_cast<float>(2.0 * fraction - 1.0);
        auto value = sample(phase, framePosition, blend, upper);
        if (upper > 0 && mix < 1.0f)
            value = juce::jmap(mix, sample(phase, framePosition, blend, upper - 1), value);
        return value * static_cast<float>(std::min(1.0, (bandwidth - step) / 0.05));
    }

    float sample(double phase, float framePosition, float blend, int level = levelCount - 1) const noexcept
    {
        const auto phaseIndex = phase * tableSize;
        const auto index = static_cast<int>(phaseIndex);
        const auto phaseMix = static_cast<float>(phaseIndex - index);
        const auto frameIndex = framePosition * (frameCount - 1);
        const auto first = static_cast<int>(frameIndex);
        const auto frameMix = frameIndex - static_cast<float>(first);

        const auto sampleBank = [&](int bank)
        {
            const auto sampleFrame = [&](int frame)
            {
                const auto& wave = data[bank][level][frame];
                return juce::jmap(phaseMix, wave[index & tableMask], wave[(index + 1) & tableMask]);
            };
            const auto value = sampleFrame(first);
            return frameMix == 0.0f ? value : juce::jmap(frameMix, value, sampleFrame(first + 1));
        };

        if (blend == 0.0f || blend == 1.0f)
            return sampleBank(static_cast<int>(blend));
        return juce::jmap(blend, sampleBank(0), sampleBank(1));
    }

private:
    using Wave = std::array<float, tableSize>;
    using Frames = std::array<Wave, frameCount>;
    std::array<std::array<Frames, levelCount>, 2> data {};
};

namespace
{
const WavetableBank& builtInTables()
{
    static const WavetableBank bank;
    return bank;
}
}

SynthEngine::SynthEngine() : tables(builtInTables()) {}

float SynthEngine::Filter::process(float input, float g, float damping) noexcept
{
    // Topology-preserving state variable filter with shared stereo coefficients.
    const auto v1 = (integrator1 + g * (input - integrator2)) / (1.0f + g * (g + damping));
    const auto v2 = integrator2 + g * v1;
    integrator1 = 2.0f * v1 - integrator1;
    integrator2 = 2.0f * v2 - integrator2;
    return v2;
}

void SynthEngine::prepare(double sampleRate, const Settings& settings)
{
    rate = sampleRate;
    fadeSamples = std::max(1, juce::roundToInt(rate * 0.003));
    for (auto& value : smooth)
        value.reset(rate, 0.015);
    for (auto& offset : offsets)
        offset.reset(rate, 0.015);
    for (auto& weight : weights)
        weight.reset(rate, 0.015);
    for (auto& voice : voices)
        voice.envelope.setSampleRate(rate);

    reset();
    setTargets(settings, true);
}

void SynthEngine::silence(Voice& voice) noexcept
{
    voice.envelope.reset();
    voice.leftFilter = {};
    voice.rightFilter = {};
    voice.lastSample = {};
    voice.held = false;
    voice.sustained = false;
}

void SynthEngine::reset() noexcept
{
    for (auto& voice : voices)
    {
        silence(voice);
        voice.tailSamples = 0;
        voice.age = 0;
    }
    sustainPedal.fill(false);
    freePhase = 0.0;
    noteCounter = 0;
    displayPlaying.store(false, std::memory_order_relaxed);
    displayPhase.store(0.0f, std::memory_order_relaxed);
}

void SynthEngine::setTargets(Settings settings, bool immediate) noexcept
{
    settings[position] *= 0.01f;
    settings[spread] *= 0.01f;
    settings[attack] *= 0.001f;
    settings[decay] *= 0.001f;
    settings[sustain] *= 0.01f;
    settings[release] *= 0.001f;
    settings[cutoff] = std::log2(settings[cutoff]);
    settings[resonance] *= 0.01f;
    settings[positionDepth] *= 0.005f;
    settings[cutoffDepth] *= 0.04f;
    settings[output] = juce::Decibels::decibelsToGain(settings[output]);

    const auto set = [immediate](auto& smoother, float value)
    {
        if (immediate)
            smoother.setCurrentAndTargetValue(value);
        else
            smoother.setTargetValue(value);
    };
    for (std::size_t i = 0; i < smooth.size(); ++i)
        set(smooth[i], settings[i]);

    const auto count = static_cast<int>(settings[unison]);
    for (int i = 0; i < static_cast<int>(unisonCount); ++i)
    {
        const auto enabled = i < count;
        const auto offset = count > 1 && enabled ? -1.0f + 2.0f * static_cast<float>(i)
                                                       / static_cast<float>(count - 1) : 0.0f;
        // Keep the outgoing slot's tuning and pan while its gain fades out.
        if (enabled || immediate)
            set(offsets[i], offset);
        set(weights[i], enabled ? 1.0f / static_cast<float>(count) : 0.0f);
    }
}

void SynthEngine::startNote(int channel, int note, int velocity, const Settings& current) noexcept
{
    const auto priority = [](const Voice& voice)
    {
        if (!voice.envelope.isActive()) return 0;
        return voice.held || voice.sustained ? 2 : 1;
    };
    auto& voice = *std::min_element(voices.begin(), voices.end(), [&](const Voice& first, const Voice& second)
    {
        const auto firstPriority = priority(first);
        const auto secondPriority = priority(second);
        return firstPriority == secondPriority ? first.age < second.age : firstPriority < secondPriority;
    });

    if (voice.envelope.isActive())
    {
        const auto tailGain = static_cast<float>(voice.tailSamples) / static_cast<float>(fadeSamples);
        voice.stealTail = { voice.lastSample.left + voice.stealTail.left * tailGain,
                            voice.lastSample.right + voice.stealTail.right * tailGain };
        voice.tailSamples = fadeSamples;
    }

    silence(voice);
    voice.age = ++noteCounter;
    voice.note = note;
    voice.channel = channel;
    voice.held = true;
    voice.frequency = juce::MidiMessage::getMidiNoteInHertz(note);
    voice.velocity = static_cast<float>(velocity) / 127.0f;
    voice.lfoPhase = 0.0;
    for (std::size_t i = 0; i < voice.phases.size(); ++i)
        voice.phases[i] = wrap(static_cast<double>(i) * 0.38196601125);

    voice.envelope.setParameters({ current[attack], current[decay], current[sustain], current[release] });
    voice.envelope.noteOn();
}

void SynthEngine::stopNote(int channel, int note) noexcept
{
    Voice* oldest = nullptr;
    for (auto& voice : voices)
    {
        if (voice.held && voice.channel == channel && voice.note == note
            && (oldest == nullptr || voice.age < oldest->age))
            oldest = &voice;
    }

    if (oldest != nullptr)
    {
        oldest->held = false;
        oldest->sustained = sustainPedal[channel];
        if (!oldest->sustained)
            oldest->envelope.noteOff();
    }
}

void SynthEngine::handleController(int channel, int controller, int value) noexcept
{
    if (controller == 64 || controller == 121)
    {
        sustainPedal[channel] = controller == 64 && value >= 64;
        if (!sustainPedal[channel])
        {
            for (auto& voice : voices)
            {
                if (voice.channel == channel && voice.sustained)
                {
                    voice.sustained = false;
                    voice.envelope.noteOff();
                }
            }
        }
    }
    else if (controller == 120 || controller == 123)
    {
        for (auto& voice : voices)
        {
            if (voice.channel != channel)
                continue;

            if (controller == 120)
            {
                silence(voice);
                voice.tailSamples = 0;
            }
            else if (voice.held)
            {
                voice.held = false;
                voice.sustained = sustainPedal[channel];
                if (!voice.sustained)
                    voice.envelope.noteOff();
            }
        }
    }
}

void SynthEngine::handleMidi(juce::MidiMessageMetadata event, const Settings& current) noexcept
{
    if (event.numBytes != 3)
        return;

    const auto status = event.data[0] & 0xf0;
    const auto channel = event.data[0] & 0x0f;
    const auto note = event.data[1];
    const auto velocity = event.data[2];
    if (status == 0x90 && velocity != 0)
        startNote(channel, note, velocity, current);
    else if (status == 0x80 || status == 0x90)
        stopNote(channel, note);
    else if (status == 0xb0)
        handleController(channel, note, velocity);
}

void SynthEngine::process(juce::AudioBuffer<float>& audio, const juce::MidiBuffer& midi,
                          Settings settings) noexcept
{
    setTargets(settings, false);
    auto event = midi.cbegin();
    const auto end = midi.cend();
    auto* left = audio.getWritePointer(0);
    auto* right = audio.getWritePointer(1);
    const auto nyquistGuard = std::log2(static_cast<float>(rate) * 0.45f);
    Modulation display {};

    for (int sample = 0; sample < audio.getNumSamples(); ++sample)
    {
        Settings current;
        for (std::size_t i = 0; i < current.size(); ++i)
            current[i] = smooth[i].getNextValue();
        while (event != end && (*event).samplePosition <= sample)
            handleMidi(*event++, current);

        std::array<double, unisonCount> steps;
        std::array<Stereo, unisonCount> panning;
        const auto tuning = current[tune] * 100.0f + current[fine];
        for (std::size_t i = 0; i < unisonCount; ++i)
        {
            const auto offset = offsets[i].getNextValue();
            const auto weight = weights[i].getNextValue();
            const auto pan = offset * current[spread];
            steps[i] = std::exp2((tuning + offset * current[detune]) / 1200.0) / rate;
            panning[i] = { weight * (1.0f - std::max(0.0f, pan)),
                           weight * (1.0f + std::min(0.0f, pan)) };
        }

        const auto lfoStep = static_cast<double>(current[lfoRate]) / rate;
        const auto freeValue = lfo(freePhase);
        const auto damping = juce::jmap(current[resonance], 1.41421356f, 0.36f);
        Stereo mix;
        std::uint64_t newest = 0;
        const bool lastSample = sample == audio.getNumSamples() - 1;
        if (lastSample)
            display = { static_cast<float>(freePhase), current[position], std::exp2(current[cutoff]), false };

        for (auto& voice : voices)
        {
            if (voice.envelope.isActive())
            {
                const auto modulation = juce::jmap(current[lfoMode], freeValue, lfo(voice.lfoPhase));
                const auto frame = juce::jlimit(0.0f, 1.0f,
                    current[position] + modulation * current[positionDepth]);
                const auto frequency = std::exp2(juce::jlimit(4.321928f, nyquistGuard,
                    current[cutoff] + modulation * current[cutoffDepth]));
                const auto g = std::tan(pi * frequency / static_cast<float>(rate));

                Stereo oscillator;
                for (std::size_t i = 0; i < unisonCount; ++i)
                {
                    const auto step = voice.frequency * steps[i];
                    if (panning[i].left > 0.0f || panning[i].right > 0.0f)
                    {
                        const auto wave = tables.render(voice.phases[i], step, frame, current[table]);
                        oscillator.left += wave * panning[i].left;
                        oscillator.right += wave * panning[i].right;
                    }
                    voice.phases[i] = wrap(voice.phases[i] + step);
                }

                const auto amplitude = voice.envelope.getNextSample() * voice.velocity * 0.32f;
                voice.lastSample = { voice.leftFilter.process(oscillator.left, g, damping) * amplitude,
                                     voice.rightFilter.process(oscillator.right, g, damping) * amplitude };
                mix.left += voice.lastSample.left;
                mix.right += voice.lastSample.right;

                if (lastSample && voice.age > newest)
                {
                    newest = voice.age;
                    display = { static_cast<float>(current[lfoMode] >= 0.5f ? voice.lfoPhase : freePhase),
                                frame, frequency, true };
                }
                voice.lfoPhase = wrap(voice.lfoPhase + lfoStep);
            }
            if (voice.tailSamples > 0)
            {
                const auto gain = static_cast<float>(--voice.tailSamples) / static_cast<float>(fadeSamples);
                mix.left += voice.stealTail.left * gain;
                mix.right += voice.stealTail.right * gain;
            }
        }

        left[sample] = limit(mix.left * current[output]);
        right[sample] = limit(mix.right * current[output]);
        freePhase = wrap(freePhase + lfoStep);
    }

    displayPhase.store(display.phase, std::memory_order_relaxed);
    displayPosition.store(display.position, std::memory_order_relaxed);
    displayCutoff.store(display.cutoff, std::memory_order_relaxed);
    displayPlaying.store(display.playing, std::memory_order_relaxed);
}

SynthEngine::Modulation SynthEngine::modulation() const noexcept
{
    return { displayPhase.load(std::memory_order_relaxed), displayPosition.load(std::memory_order_relaxed),
             displayCutoff.load(std::memory_order_relaxed), displayPlaying.load(std::memory_order_relaxed) };
}

float SynthEngine::preview(float phase, float framePosition, int tableIndex) const noexcept
{
    return tables.sample(phase, framePosition, static_cast<float>(tableIndex));
}
}
