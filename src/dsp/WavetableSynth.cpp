#include "WavetableSynth.h"
#include "WavetableImport.h"

#include <algorithm>
#include <cmath>

namespace wave
{
namespace
{
constexpr float pi = juce::MathConstants<float>::pi;
constexpr double twoPi = juce::MathConstants<double>::twoPi;
constexpr int frameCount = 4;
constexpr int levelCount = ImportedWavetable::levelCount;
constexpr int tableSize = ImportedWavetable::frameSize;
constexpr int tableMask = tableSize - 1;

double wrap(double phase) noexcept
{
    return phase - std::floor(phase);
}

float detuneBlend(float cents) noexcept
{
    return std::min(cents / 3.0f, 1.0f);
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

template <typename Sample>
float renderBandlimited(double step, Sample sample) noexcept
{
    constexpr double bandwidth = 0.45;
    if (step >= bandwidth)
        return 0.0f;

    // Blend only bands whose highest partial is inside the Nyquist guard.
    int exponent = 0;
    const auto fraction = std::frexp(bandwidth / step, &exponent);
    const auto upper = std::min(exponent - 1, levelCount - 1);
    const auto mix = exponent > levelCount ? 1.0f : static_cast<float>(2.0 * fraction - 1.0);
    auto value = sample(upper);
    if (upper > 0 && mix < 1.0f)
        value = juce::jmap(mix, sample(upper - 1), value);
    return value * static_cast<float>(std::min(1.0, (bandwidth - step) / 0.05));
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
        return renderBandlimited(step, [&](int band) { return sample(phase, framePosition, blend, band); });
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

float lfoSample(double phase, float rampAmount) noexcept
{
    const auto sine = static_cast<float>(std::sin(twoPi * phase));
    const auto ramp = static_cast<float>(1.0 - 2.0 * phase);
    return juce::jmap(rampAmount, sine, ramp);
}

SynthEngine::SynthEngine(WavetableImports& main, WavetableImports& imported)
    : tables(builtInTables()), mainImports(main), imports(imported)
{
}

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
    voice.lastAmplitude = 0.0f;
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
    settings[position2] *= 0.01f;
    settings[level] *= 0.01f;
    settings[level2] *= 0.01f;
    settings[spread] *= 0.01f;
    settings[attack] *= 0.001f;
    settings[decay] *= 0.001f;
    settings[sustain] *= 0.01f;
    settings[release] *= 0.001f;
    settings[cutoff] = std::log2(settings[cutoff]);
    settings[resonance] *= 0.01f;
    settings[positionDepth] *= 0.005f;
    settings[positionDepth2] *= 0.005f;
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
        set(weights[i], enabled ? 1.0f : 0.0f);
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
        voice.stealAmplitude = voice.lastAmplitude + voice.stealAmplitude * tailGain;
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
    voice.shotPhase = 0.0;
    voice.layerPhase = 0.0;
    const auto phaseSpread = detuneBlend(current[detune]);
    for (std::size_t i = 0; i < voice.phases.size(); ++i)
        voice.phases[i] = phaseSpread * wrap(static_cast<double>(i) * 0.38196601125);

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
                          Settings settings, double tempo) noexcept
{
    setTargets(settings, false);
    auto event = midi.cbegin();
    const auto end = midi.cend();
    auto* left = audio.getWritePointer(0);
    auto* right = audio.getWritePointer(1);
    const auto nyquistGuard = std::log2(static_cast<float>(rate) * 0.45f);
    const auto* importedMain = mainImports.acquireForAudio();
    const auto* imported = imports.acquireForAudio();
    const auto beats = lfoBeats[static_cast<std::size_t>(settings[lfoTiming])];
    const auto syncedRate = beats > 0.0 ? tempo / (60.0 * beats) : 0.0;
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
        const auto layerRatio = current[level2] > 0.0f
            ? std::exp2((current[tune2] * 100.0f + current[fine2]) / 1200.0) / rate
            : 0.0;
        Stereo panSum;
        float unisonEnergy = 0.0f;
        for (std::size_t i = 0; i < unisonCount; ++i)
        {
            const auto offset = offsets[i].getNextValue();
            const auto weight = weights[i].getNextValue();
            const auto pan = juce::jlimit(-1.0f, 1.0f, offset * current[spread]);
            steps[i] = std::exp2((tuning + offset * current[detune]) / 1200.0) / rate;
            panning[i] = { weight * std::sqrt(1.0f - pan), weight * std::sqrt(1.0f + pan) };
            panSum.left += panning[i].left;
            panSum.right += panning[i].right;
            unisonEnergy += weight * weight;
        }

        // Detuned voices add in power over time. Near zero detune, allow for in-phase summing.
        const auto coherentEnergy = 0.5f * (panSum.left * panSum.left + panSum.right * panSum.right);
        const auto unisonGain = 1.0f / std::sqrt(std::max(1.0f,
            juce::jmap(detuneBlend(current[detune]), coherentEnergy, unisonEnergy)));
        const auto lfoStep = (beats > 0.0 ? syncedRate : static_cast<double>(current[lfoRate])) / rate;
        const auto shape = current[lfoShape];
        const auto freeValue = lfoSample(freePhase, shape);
        const auto damping = juce::jmap(current[resonance], 1.41421356f, 0.36f);
        Stereo mix;
        float envelopeEnergy = 0.0f;
        std::uint64_t newest = 0;
        const bool lastSample = sample == audio.getNumSamples() - 1;
        if (lastSample)
            display = { static_cast<float>(freePhase), current[position], current[position2],
                        std::exp2(current[cutoff]), false };

        for (auto& voice : voices)
        {
            float amplitude = 0.0f;
            if (voice.envelope.isActive())
            {
                auto modulation = juce::jmap(current[lfoMode], freeValue, lfoSample(voice.lfoPhase, shape));
                if (current[lfoCycle] > 0.0f)
                    modulation = juce::jmap(current[lfoCycle], modulation, lfoSample(voice.shotPhase, shape));
                const auto frame = juce::jlimit(0.0f, 1.0f,
                    current[position] + modulation * current[positionDepth]);
                const auto layerFrame = juce::jlimit(0.0f, 1.0f,
                    current[position2] + modulation * current[positionDepth2]);
                const auto frequency = std::exp2(juce::jlimit(4.321928f, nyquistGuard,
                    current[cutoff] + modulation * current[cutoffDepth]));
                const auto g = std::tan(pi * frequency / static_cast<float>(rate));

                Stereo oscillator;
                for (std::size_t i = 0; i < unisonCount; ++i)
                {
                    const auto step = voice.frequency * steps[i];
                    if (panning[i].left > 0.0f || panning[i].right > 0.0f)
                    {
                        const auto phase = voice.phases[i];
                        const auto importMix = current[useImported1];
                        float wave;
                        if (importMix == 0.0f)
                            wave = tables.render(phase, step, frame, current[table]);
                        else
                            wave = renderBandlimited(step, [&](int band)
                            {
                                const auto custom = importedMain != nullptr ? importedMain->sample(phase, frame, band) : 0.0f;
                                return importMix == 1.0f ? custom
                                    : juce::jmap(importMix, tables.sample(phase, frame, current[table], band), custom);
                            });
                        oscillator.left += wave * panning[i].left;
                        oscillator.right += wave * panning[i].right;
                    }
                    voice.phases[i] = wrap(voice.phases[i] + step);
                }

                float layer = 0.0f;
                if (layerRatio > 0.0)
                {
                    const auto step = voice.frequency * layerRatio;
                    const auto phase = voice.layerPhase;
                    const auto tableBlend = current[table2];
                    layer = renderBandlimited(step, [&](int band)
                    {
                        if (tableBlend <= 1.0f)
                            return tables.sample(phase, layerFrame, tableBlend, band);

                        const auto custom = imported != nullptr ? imported->sample(phase, layerFrame, band) : 0.0f;
                        if (tableBlend >= importedTableIndex)
                            return custom;
                        return juce::jmap(tableBlend - 1.0f, tables.sample(phase, layerFrame, 1.0f, band), custom);
                    }) * current[level2];
                    voice.layerPhase = wrap(phase + step);
                }

                amplitude = voice.envelope.getNextSample() * voice.velocity;
                voice.lastAmplitude = amplitude;
                const auto mainGain = unisonGain * current[level];
                voice.lastSample = { voice.leftFilter.process(oscillator.left * mainGain + layer, g, damping) * amplitude,
                                     voice.rightFilter.process(oscillator.right * mainGain + layer, g, damping) * amplitude };
                mix.left += voice.lastSample.left;
                mix.right += voice.lastSample.right;

                if (lastSample && voice.age > newest)
                {
                    newest = voice.age;
                    const auto loopPhase = current[lfoMode] >= 0.5f ? voice.lfoPhase : freePhase;
                    display = { static_cast<float>(current[lfoCycle] >= 0.5f ? voice.shotPhase : loopPhase),
                                frame, layerFrame, frequency, true };
                }
                voice.lfoPhase = wrap(voice.lfoPhase + lfoStep);
                voice.shotPhase = std::min(1.0, voice.shotPhase + lfoStep);
            }
            if (voice.tailSamples > 0)
            {
                const auto gain = static_cast<float>(--voice.tailSamples) / static_cast<float>(fadeSamples);
                mix.left += voice.stealTail.left * gain;
                mix.right += voice.stealTail.right * gain;
                amplitude += voice.stealAmplitude * gain;
            }
            envelopeEnergy += amplitude * amplitude;
        }

        // Leave solo notes at full gain and trim chords as their envelopes build.
        constexpr float soloGain = 0.8f;
        const auto gain = soloGain * current[output] / std::sqrt(std::max(1.0f, envelopeEnergy));
        left[sample] = limit(mix.left * gain);
        right[sample] = limit(mix.right * gain);
        freePhase = wrap(freePhase + lfoStep);
    }

    imports.releaseFromAudio();
    mainImports.releaseFromAudio();
    displayPhase.store(display.phase, std::memory_order_relaxed);
    displayPosition.store(display.position, std::memory_order_relaxed);
    displayPosition2.store(display.position2, std::memory_order_relaxed);
    displayCutoff.store(display.cutoff, std::memory_order_relaxed);
    displayPlaying.store(display.playing, std::memory_order_relaxed);
}

SynthEngine::Modulation SynthEngine::modulation() const noexcept
{
    return { displayPhase.load(std::memory_order_relaxed), displayPosition.load(std::memory_order_relaxed),
             displayPosition2.load(std::memory_order_relaxed), displayCutoff.load(std::memory_order_relaxed),
             displayPlaying.load(std::memory_order_relaxed) };
}

float SynthEngine::preview(float phase, float framePosition, int tableIndex,
                           const ImportedWavetable* imported) const noexcept
{
    if (tableIndex == importedTableIndex)
        return imported != nullptr ? imported->sample(phase, framePosition) : 0.0f;

    return tables.sample(phase, framePosition, static_cast<float>(tableIndex));
}
}
