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
    level, table2, position2, tune2, fine2, level2, parameterCount
};

constexpr int importedTableIndex = 2;

using Settings = std::array<float, parameterCount>;
using ParameterState = juce::AudioProcessorValueTreeState;

const char* parameterId(Parameter parameter);
const char* parameterName(Parameter parameter);
ParameterState::ParameterLayout makeParameters();
juce::StringArray patchNames();
void loadPatch(ParameterState& state, int index);
}
