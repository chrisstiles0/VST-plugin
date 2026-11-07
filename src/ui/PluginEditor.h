#pragma once

#include "../dsp/WavetableSynth.h"

namespace wave
{
juce::AudioProcessorEditor* createWavetableEditor(juce::AudioProcessor& processor,
                                                ParameterState& state, const SynthEngine& engine);
}
