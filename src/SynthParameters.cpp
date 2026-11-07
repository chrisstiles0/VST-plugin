#include "SynthParameters.h"

namespace wave
{
namespace
{
struct ParameterInfo
{
    const char* id;
    const char* name;
    const char* unit;
    float minimum;
    float maximum;
    float interval;
    float initial;
    float centre = 0.0f;
};

constexpr std::array<ParameterInfo, parameterCount> parameters {{
    { "table", "Table", "", 0, 1, 1, 0 },
    { "position", "Position", "%", 0, 100, 0.1f, 0 },
    { "tune", "Transpose", "st", -24, 24, 1, 0 },
    { "fine", "Fine tune", "ct", -100, 100, 0.1f, 0 },
    { "unison", "Voices", "", 1, 7, 1, 1 },
    { "detune", "Detune", "ct", 0, 50, 0.1f, 12 },
    { "spread", "Spread", "%", 0, 100, 1, 65 },
    { "attack", "Attack", "ms", 2, 5000, 1, 8, 150 },
    { "decay", "Decay", "ms", 5, 5000, 1, 250, 350 },
    { "sustain", "Sustain", "%", 0, 100, 0.1f, 100 },
    { "release", "Release", "ms", 5, 8000, 1, 90, 500 },
    { "cutoff", "Cutoff", "Hz", 20, 20000, 1, 20000, 1500 },
    { "resonance", "Resonance", "%", 0, 100, 0.1f, 0 },
    { "lfo_mode", "LFO mode", "", 0, 1, 1, 1 },
    { "lfo_rate", "LFO rate", "Hz", 0.05f, 20, 0.01f, 1, 2 },
    { "position_depth", "Position depth", "%", 0, 100, 0.1f, 0 },
    { "cutoff_depth", "Cutoff depth", "%", 0, 100, 0.1f, 0 },
    { "output", "Output", "dB", -60, 0, 0.1f, -9 },
    { "osc1_level", "Oscillator 1 level", "%", 0, 100, 0.1f, 100 },
    { "osc2_table", "Oscillator 2 table", "", 0, importedTableIndex, 1, 0 },
    { "osc2_position", "Oscillator 2 position", "%", 0, 100, 0.1f, 0 },
    { "osc2_transpose", "Oscillator 2 transpose", "st", -24, 24, 1, -12 },
    { "osc2_fine", "Oscillator 2 fine tune", "ct", -100, 100, 0.1f, 0 },
    { "osc2_level", "Oscillator 2 level", "%", 0, 100, 0.1f, 0 },
}};

void applyPatch(Settings& values, int index)
{
    switch (index)
    {
        case 1:
            values[position] = 71;
            values[unison] = 7;
            values[detune] = 18;
            values[spread] = 90;
            values[attack] = 12;
            values[decay] = 320;
            values[sustain] = 76;
            values[release] = 150;
            values[cutoff] = 8500;
            values[resonance] = 10;
            break;
        case 2:
        case 4:
            values[table] = 1;
            values[position] = 43;
            values[tune] = -12;
            values[decay] = 200;
            values[sustain] = 82;
            values[cutoff] = 360;
            values[resonance] = 34;
            values[lfoRate] = 2.2f;
            values[positionDepth] = 37;
            values[cutoffDepth] = 46;
            if (index == 4)
            {
                values[tune] = 0;
                values[level] = 72;
                values[level2] = 38;
            }
            break;
        case 3:
            values[table] = 1;
            values[position] = 52;
            values[unison] = 5;
            values[detune] = 14;
            values[spread] = 88;
            values[attack] = 630;
            values[decay] = 1800;
            values[sustain] = 74;
            values[release] = 2100;
            values[cutoff] = 1800;
            values[resonance] = 19;
            values[lfoMode] = 0;
            values[lfoRate] = 0.23f;
            values[positionDepth] = 42;
            values[cutoffDepth] = 27;
            values[output] = -7;
            break;
        default:
            break;
    }
}
}

const char* parameterId(Parameter parameter) { return parameters[parameter].id; }
const char* parameterName(Parameter parameter) { return parameters[parameter].name; }

ParameterState::ParameterLayout makeParameters()
{
    ParameterState::ParameterLayout layout;
    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        const auto& entry = parameters[i];
        const juce::ParameterID id { entry.id, i < level ? 1 : 2 };
        if (i == table || i == table2 || i == lfoMode)
        {
            auto choices = i == lfoMode ? juce::StringArray { "Free-running", "Note-triggered" }
                                        : juce::StringArray { "Classic", "Hollow" };
            if (i == table2)
                choices.add("Imported");
            layout.add(std::make_unique<juce::AudioParameterChoice>(
                id, entry.name, choices, static_cast<int>(entry.initial)));
        }
        else if (i == tune || i == tune2 || i == unison)
        {
            layout.add(std::make_unique<juce::AudioParameterInt>(
                id, entry.name, static_cast<int>(entry.minimum), static_cast<int>(entry.maximum),
                static_cast<int>(entry.initial),
                juce::AudioParameterIntAttributes().withLabel(entry.unit)));
        }
        else
        {
            juce::NormalisableRange<float> range { entry.minimum, entry.maximum, entry.interval };
            if (entry.centre > 0.0f)
                range.setSkewForCentre(entry.centre);

            layout.add(std::make_unique<juce::AudioParameterFloat>(
                id, entry.name, range, entry.initial,
                juce::AudioParameterFloatAttributes().withLabel(entry.unit)));
        }
    }
    return layout;
}

juce::StringArray patchNames()
{
    return { "Init", "Wide unison lead", "Modulated bass", "Evolving pad", "Layered bass" };
}

void loadPatch(ParameterState& state, int index)
{
    Settings values;
    for (std::size_t i = 0; i < parameters.size(); ++i)
        values[i] = parameters[i].initial;
    applyPatch(values, index);

    for (std::size_t i = 0; i < parameters.size(); ++i)
    {
        auto& parameter = *state.getParameter(parameters[i].id);
        const auto value = parameter.convertTo0to1(values[i]);
        if (parameter.getValue() != value)
        {
            parameter.beginChangeGesture();
            parameter.setValueNotifyingHost(value);
            parameter.endChangeGesture();
        }
    }
}
}
