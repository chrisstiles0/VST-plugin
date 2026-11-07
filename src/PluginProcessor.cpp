#include "SynthParameters.h"
#include "dsp/WavetableSynth.h"
#include "dsp/WavetableImport.h"
#include "ui/PluginEditor.h"

#include <cmath>

namespace
{
const juce::Identifier stateType("WavetableSynthState");
const juce::Identifier versionKey("schemaVersion");
const juce::Identifier importName("importedTableName");
const juce::Identifier importSamples("importedTableSamples");
const juce::Identifier mainImportName("osc1ImportedTableName");
const juce::Identifier mainImportSamples("osc1ImportedTableSamples");
constexpr int stateVersion = 3;

class WavetableProcessor final : public juce::AudioProcessor
{
public:
    WavetableProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          state(*this, nullptr, stateType, wave::makeParameters()),
          defaults(state.copyState())
    {
        for (std::size_t i = 0; i < values.size(); ++i)
            values[i] = state.getRawParameterValue(wave::parameterId(static_cast<wave::Parameter>(i)));
    }

    const juce::String getName() const override { return "Wavetable Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    bool hasEditor() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override
    {
        return wave::createWavetableEditor(*this, state, engine, mainImports, imports);
    }

    bool isBusesLayoutSupported(const BusesLayout& layout) const override
    {
        return layout.getMainInputChannelSet().isDisabled()
            && layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void prepareToPlay(double sampleRate, int) override
    {
        engine.prepare(sampleRate, readSettings());
    }

    void reset() override { engine.reset(); }
    void releaseResources() override { engine.reset(); }

    void processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;
        if (audio.getNumChannels() == 2 && audio.getNumSamples() > 0)
        {
            const auto settings = readSettings();
            auto tempo = 120.0;
            if (settings[wave::lfoTiming] != 0.0f)
                if (const auto* hostPlayHead = getPlayHead())
                    if (const auto position = hostPlayHead->getPosition())
                        if (const auto bpm = position->getBpm(); bpm && std::isfinite(*bpm) && *bpm > 0.0)
                            tempo = *bpm;

            engine.process(audio, midi, settings, tempo);
        }
        else
            audio.clear();

        midi.clear();
    }

    void getStateInformation(juce::MemoryBlock& destination) override
    {
        auto snapshot = state.copyState();
        snapshot.setProperty(versionKey, stateVersion, nullptr);
        const auto saveTable = [&](const wave::WavetableImports& source, const juce::Identifier& name,
                                   const juce::Identifier& samples)
        {
            if (const auto imported = source.snapshot())
            {
                snapshot.setProperty(name, imported->name(), nullptr);
                snapshot.setProperty(samples, imported->encode(), nullptr);
            }
        };
        saveTable(mainImports, mainImportName, mainImportSamples);
        saveTable(imports, importName, importSamples);
        if (const auto xml = snapshot.createXml())
            copyXmlToBinary(*xml, destination);
    }

    void setStateInformation(const void* data, int size) override
    {
        const auto xml = getXmlFromBinary(data, size);
        if (xml == nullptr)
            return;

        auto restored = juce::ValueTree::fromXml(*xml);
        const auto version = static_cast<int>(restored[versionKey]);
        if (!restored.hasType(stateType) || version < 1 || version > stateVersion)
            return;

        const auto restoreTable = [&](auto& result, const juce::Identifier& name,
                                      const juce::Identifier& samples)
        {
            if (!restored.hasProperty(samples))
                return true;
            result = wave::ImportedWavetable::restore(restored[name].toString(), restored[samples].toString());
            restored.removeProperty(name, nullptr);
            restored.removeProperty(samples, nullptr);
            return result != nullptr;
        };
        std::shared_ptr<const wave::ImportedWavetable> importedMain, importedLayer;
        if (!restoreTable(importedMain, mainImportName, mainImportSamples)
            || !restoreTable(importedLayer, importName, importSamples))
            return;

        // Earlier sessions have no oscillator 2 controls. Init keeps that layer muted.
        for (const auto& child : defaults)
            if (!restored.getChildWithProperty("id", child["id"]).isValid())
                restored.appendChild(child.createCopy(), nullptr);

        mainImports.replace(std::move(importedMain));
        imports.replace(std::move(importedLayer));
        state.replaceState(restored);
    }

private:
    wave::Settings readSettings() const noexcept
    {
        wave::Settings settings;
        for (std::size_t i = 0; i < values.size(); ++i)
            settings[i] = values[i]->load(std::memory_order_relaxed);
        return settings;
    }

    wave::ParameterState state;
    const juce::ValueTree defaults;
    std::array<const std::atomic<float>*, wave::parameterCount> values {};
    wave::WavetableImports mainImports, imports;
    wave::SynthEngine engine { mainImports, imports };
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableProcessor;
}
