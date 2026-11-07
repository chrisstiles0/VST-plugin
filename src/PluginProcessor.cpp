#include <juce_audio_processors/juce_audio_processors.h>

namespace
{
class WavetableProcessor final : public juce::AudioProcessor
{
public:
    WavetableProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          state(*this, nullptr, "WavetableSynthState", makeParameters())
    {
    }

    const juce::String getName() const override { return "Wavetable Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override
    {
        return new juce::GenericAudioProcessorEditor(*this);
    }

    bool isBusesLayoutSupported(const BusesLayout& layout) const override
    {
        return layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void prepareToPlay(double sampleRate, int) override
    {
        rate = sampleRate;
        phase = level = target = 0.0;
    }

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;
        auto event = midi.cbegin();
        const auto end = midi.cend();
        auto* left = audio.getWritePointer(0);
        auto* right = audio.getWritePointer(1);
        const auto gain = juce::Decibels::decibelsToGain(output->load(std::memory_order_relaxed));

        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
        {
            while (event != end && (*event).samplePosition <= sample)
            {
                const auto message = *event++;
                if (message.numBytes == 3)
                {
                    const auto status = message.data[0] & 0xf0;
                    if (status == 0x90 && message.data[2] != 0)
                    {
                        note = message.data[1];
                        frequency = juce::MidiMessage::getMidiNoteInHertz(note);
                        target = message.data[2] / 127.0;
                    }
                    else if ((status == 0x80 || status == 0x90) && message.data[1] == note)
                    {
                        target = 0.0;
                    }
                }
            }

            level += (target - level) * 0.003;
            left[sample] = right[sample] = static_cast<float>(
                std::sin(phase * juce::MathConstants<double>::twoPi) * level * gain * 0.3);
            phase += frequency / rate;
            phase -= std::floor(phase);
        }
        midi.clear();
    }

    void getStateInformation(juce::MemoryBlock& destination) override
    {
        auto snapshot = state.copyState();
        snapshot.setProperty("schemaVersion", 1, nullptr);
        if (auto xml = snapshot.createXml())
            copyXmlToBinary(*xml, destination);
    }

    void setStateInformation(const void* data, int size) override
    {
        if (auto xml = getXmlFromBinary(data, size))
        {
            auto restored = juce::ValueTree::fromXml(*xml);
            if (restored.hasType(state.state.getType()) && int(restored["schemaVersion"]) == 1)
                state.replaceState(restored);
        }
    }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "output", 1 }, "Output",
            juce::NormalisableRange<float> { -60.0f, 0.0f, 0.1f }, -9.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));
        return layout;
    }

    juce::AudioProcessorValueTreeState state;
    const std::atomic<float>* output = state.getRawParameterValue("output");
    double rate = 44100.0;
    double frequency = 0.0;
    double phase = 0.0;
    double level = 0.0;
    double target = 0.0;
    int note = -1;
};
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WavetableProcessor;
}
