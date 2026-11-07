#include "SynthParameters.h"
#include "dsp/WavetableSynth.h"
#include "ui/PluginEditor.h"

namespace
{
const juce::Identifier stateType("WavetableSynthState");
const juce::Identifier versionKey("schemaVersion");
constexpr int stateVersion = 1;

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
        return wave::createWavetableEditor(*this, state, engine);
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
            engine.process(audio, midi, readSettings());
        else
            audio.clear();

        midi.clear();
    }

    void getStateInformation(juce::MemoryBlock& destination) override
    {
        auto snapshot = state.copyState();
        snapshot.setProperty(versionKey, stateVersion, nullptr);
        if (const auto xml = snapshot.createXml())
            copyXmlToBinary(*xml, destination);
    }

    void setStateInformation(const void* data, int size) override
    {
        const auto xml = getXmlFromBinary(data, size);
        if (xml == nullptr)
            return;

        auto restored = juce::ValueTree::fromXml(*xml);
        if (!restored.hasType(stateType) || static_cast<int>(restored[versionKey]) != stateVersion)
            return;

        // The bootstrap build saved output alone. Fill missing parameters from Init.
        for (const auto& child : defaults)
            if (!restored.getChildWithProperty("id", child["id"]).isValid())
                restored.appendChild(child.createCopy(), nullptr);

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
    wave::SynthEngine engine;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableProcessor;
}
