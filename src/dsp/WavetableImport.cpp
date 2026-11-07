#include "WavetableImport.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace wave
{
namespace
{
constexpr int maxSamples = ImportedWavetable::frameSize * ImportedWavetable::maxFrames;

bool validLength(juce::int64 samples)
{
    return samples > 0 && samples <= maxSamples && samples % ImportedWavetable::frameSize == 0;
}

bool validSamples(const std::vector<float>& samples)
{
    return std::all_of(samples.begin(), samples.end(), [](float value)
    {
        return std::isfinite(value) && std::abs(value) <= 1.0f;
    });
}
}

std::shared_ptr<const ImportedWavetable> ImportedWavetable::load(const juce::File& file, juce::String& error)
{
    auto input = file.createInputStream();
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatReader> reader(input != nullptr ? format.createReaderFor(input.release(), true)
                                                                   : nullptr);

    if (reader == nullptr || reader->numChannels != 1 || !validLength(reader->lengthInSamples))
    {
        error = "Choose a mono WAV with 1 to 256 frames of 2,048 samples each.";
        return {};
    }

    const auto count = static_cast<int>(reader->lengthInSamples);
    std::vector<float> samples(static_cast<std::size_t>(count));
    auto* channel = samples.data();
    if (!reader->read(&channel, 1, 0, count) || !validSamples(samples))
    {
        error = "The WAV contains unreadable samples or values outside the audio range.";
        return {};
    }

    return std::shared_ptr<const ImportedWavetable>(
        new ImportedWavetable(file.getFileNameWithoutExtension(), std::move(samples)));
}

std::shared_ptr<const ImportedWavetable> ImportedWavetable::restore(juce::String name,
                                                                  const juce::String& encoded)
{
    constexpr int maxBytes = maxSamples * static_cast<int>(sizeof(float));
    constexpr auto sampleBytes = static_cast<juce::int64>(sizeof(float));
    if (encoded.length() > 4 * ((maxBytes + 2) / 3))
        return {};

    juce::MemoryOutputStream bytes;
    if (!juce::Base64::convertFromBase64(bytes, encoded))
        return {};

    const auto length = static_cast<juce::int64>(bytes.getDataSize());
    if (length % sampleBytes != 0 || !validLength(length / sampleBytes))
        return {};

    juce::MemoryInputStream stream(bytes.getData(), bytes.getDataSize(), false);
    std::vector<float> samples(static_cast<std::size_t>(length / sampleBytes));
    for (auto& value : samples)
        value = stream.readFloat();

    if (!validSamples(samples))
        return {};

    return std::shared_ptr<const ImportedWavetable>(new ImportedWavetable(std::move(name), std::move(samples)));
}

juce::String ImportedWavetable::encode() const
{
    juce::MemoryOutputStream stream(source.size() * sizeof(float));
    for (const auto value : source)
        stream.writeFloat(value);
    return juce::Base64::toBase64(stream.getData(), stream.getDataSize());
}

ImportedWavetable::ImportedWavetable(juce::String name, std::vector<float> samples)
    : displayName(name.substring(0, 128)), source(std::move(samples)), bands(source.size() * levelCount)
{
    const auto frameCount = source.size() / frameSize;
    const juce::dsp::FFT fft(11);
    using Spectrum = std::array<juce::dsp::Complex<float>, frameSize>;
    Spectrum input, spectrum, limited, cycle;
    float peak = 1.0f;

    for (std::size_t frame = 0; frame < frameCount; ++frame)
    {
        for (int i = 0; i < frameSize; ++i)
            input[i] = source[frame * frameSize + i];
        fft.perform(input.data(), spectrum.data(), false);

        for (int level = 0; level < levelCount; ++level)
        {
            limited.fill({});
            for (int harmonic = 1; harmonic <= (1 << level); ++harmonic)
            {
                limited[harmonic] = spectrum[harmonic];
                limited[frameSize - harmonic] = spectrum[frameSize - harmonic];
            }
            fft.perform(limited.data(), cycle.data(), true);
            auto* destination = bands.data() + (level * frameCount + frame) * frameSize;
            for (int i = 0; i < frameSize; ++i)
            {
                destination[i] = cycle[i].real();
                peak = std::max(peak, std::abs(destination[i]));
            }
        }
    }

    if (peak > 1.0f)
        for (auto& value : bands)
            value /= peak;
}

float ImportedWavetable::sample(double phase, float position, int level) const noexcept
{
    const auto frames = static_cast<int>(source.size() / frameSize);
    const auto framePosition = position * static_cast<float>(frames - 1);
    const auto first = static_cast<int>(framePosition);
    const auto frameMix = framePosition - static_cast<float>(first);
    const auto phasePosition = phase * frameSize;
    const auto index = static_cast<int>(phasePosition);
    const auto phaseMix = static_cast<float>(phasePosition - index);
    const auto* bank = bands.data() + level * frames * frameSize;
    const auto sampleFrame = [&](int frame)
    {
        const auto* wave = bank + frame * frameSize;
        return juce::jmap(phaseMix, wave[index & (frameSize - 1)], wave[(index + 1) & (frameSize - 1)]);
    };

    const auto value = sampleFrame(first);
    return frameMix == 0.0f ? value : juce::jmap(frameMix, value, sampleFrame(first + 1));
}

std::shared_ptr<const ImportedWavetable> WavetableImports::snapshot() const
{
    const std::scoped_lock lock(mutex);
    return current;
}

void WavetableImports::replace(std::shared_ptr<const ImportedWavetable> table)
{
    const std::scoped_lock lock(mutex);
    auto previous = std::exchange(current, std::move(table));
    published.store(current.get());
    const auto* reader = audioReader.load();
    if (previous != nullptr && previous.get() == reader)
        retired = std::move(previous);
    else if (retired.get() != reader)
        retired.reset();
}

const ImportedWavetable* WavetableImports::acquireForAudio() noexcept
{
    // Publish the hazard before checking that the table is still current.
    const auto* table = published.load();
    audioReader.store(table);
    if (published.load() == table)
        return table;

    releaseFromAudio();
    return nullptr;
}

void WavetableImports::releaseFromAudio() noexcept
{
    audioReader.store(nullptr);
}
}
