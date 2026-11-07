cmake_minimum_required(VERSION 3.22)
project(WavetableSynth VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.17-security
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(WavetableSynth
    PRODUCT_NAME "Wavetable Synth"
    COMPANY_NAME "Chris Stiles"
    BUNDLE_ID com.chrisstiles.wavetablesynth
    PLUGIN_MANUFACTURER_CODE ChSt
    PLUGIN_CODE Wt01
    FORMATS VST3
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(WavetableSynth PRIVATE
    src/PluginProcessor.cpp
    src/SynthParameters.cpp
    src/dsp/WavetableSynth.cpp
    src/ui/PluginEditor.cpp
)
target_compile_features(WavetableSynth PRIVATE cxx_std_17)
target_compile_definitions(WavetableSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(WavetableSynth
    PRIVATE juce::juce_audio_processors
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags
)
