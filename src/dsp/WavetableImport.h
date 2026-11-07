#pragma once

#include <juce_core/juce_core.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace wave
{
class ImportedWavetable
{
public:
    static constexpr int frameSize = 2048;
    static constexpr int levelCount = 9;
    static constexpr int maxFrames = 256;

    static std::shared_ptr<const ImportedWavetable> load(const juce::File& file, juce::String& error);
    static std::shared_ptr<const ImportedWavetable> restore(juce::String name, const juce::String& encoded);

    const juce::String& name() const noexcept { return displayName; }
    juce::String encode() const;
    float sample(double phase, float position, int level = levelCount - 1) const noexcept;

private:
    ImportedWavetable(juce::String name, std::vector<float> samples);

    juce::String displayName;
    std::vector<float> source;
    std::vector<float> bands;
};

class WavetableImports
{
public:
    std::shared_ptr<const ImportedWavetable> snapshot() const;
    void replace(std::shared_ptr<const ImportedWavetable> table);

    const ImportedWavetable* acquireForAudio() noexcept;
    void releaseFromAudio() noexcept;

private:
    mutable std::mutex mutex;
    std::shared_ptr<const ImportedWavetable> current;
    std::shared_ptr<const ImportedWavetable> retired;
    std::atomic<const ImportedWavetable*> published { nullptr };
    std::atomic<const ImportedWavetable*> audioReader { nullptr };
};
}
