cmake_minimum_required(VERSION 3.22)

project(JuceCMakeStarter VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE is included as a git submodule at ./JUCE
add_subdirectory(JUCE)

# Embed UI assets (e.g. dial graphics) into the plugin binary.
juce_add_binary_data(JuceCMakeStarterAssets
    SOURCES
        assets/KnobBlue.png
        assets/KnobGrey.png
        assets/KnobWhite.png
    assets/GATE_OFF.svg
    assets/GATE_ON.svg
    assets/PUNCH_OFF.svg
    assets/PUNCH_ON.svg
    assets/BUTTON_OFF.svg
    assets/BUTTON_ON.svg
    assets/LFO_TRG.svg
    assets/LFO_ONE.svg
    assets/LFO_HLF.svg
)

juce_add_plugin(JuceCMakeStarter
    COMPANY_NAME "Studio"
    BUNDLE_ID "com.studio.jucecmakestarter"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    MICROPHONE_PERMISSION_ENABLED FALSE
    CAMERA_PERMISSION_ENABLED FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE Stdo
    PLUGIN_CODE JcSt

    VST3_CATEGORIES Fx Tools

    FORMATS VST3 Standalone

    PRODUCT_NAME "JuceCMakeStarter"
)

target_sources(JuceCMakeStarter
    PRIVATE
        source/PluginProcessor.cpp
        source/PluginEditor.cpp
        source/PluginProcessor.h
        source/PluginEditor.h
    source/ui_components/LabeledSlider.h
    source/ui_components/RotaryDial.h
    source/ui_components/StudioLookAndFeel.h
    source/ui_components/ImageToggle.h
    source/ui_components/LayeredMenuButton.h
    source/ui_components/StudioStyle.h
)

target_compile_definitions(JuceCMakeStarter
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(JuceCMakeStarter
    PRIVATE
        JuceCMakeStarterAssets
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
