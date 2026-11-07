#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <cstddef>
#include <utility>

namespace wave
{
enum Parameter
{
    table, position, tune, fine, unison, detune, spread,
    attack, decay, sustain, release, cutoff, resonance,
    lfoMode, lfoRate, positionDepth, cutoffDepth, output,
    level, table2, position2, tune2, fine2, level2,
    useImported1, positionDepth2, lfoTiming, lfoCycle, lfoShape, parameterCount
};

constexpr int importedTableIndex = 2;
constexpr std::array<double, 6> lfoBeats { 0.0, 4.0, 2.0, 1.0, 0.5, 0.25 };

using Settings = std::array<float, parameterCount>;
using ParameterState = juce::AudioProcessorValueTreeState;

const char* parameterId(Parameter parameter);
const char* parameterName(Parameter parameter);
ParameterState::ParameterLayout makeParameters();
juce::StringArray patchNames();
void loadPatch(ParameterState& state, int index);
}
