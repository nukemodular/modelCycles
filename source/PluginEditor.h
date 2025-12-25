#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>

#include "ui_components/RotaryDial.h"
#include "ui_components/ImageToggle.h"
#include "ui_components/LayeredMenuButton.h"
#include "ui_components/StudioLookAndFeel.h"

class PluginProcessor;

class PluginEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& pluginProcessor;

    StudioLookAndFeel lookAndFeel;

    RotaryDial transposeControl { "TRANSPOSE", RotaryDial::LabelPlacement::Below };
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> transposeAttachment;

    ImageToggle gateToggle;
    ImageToggle punchToggle;
    ImageToggle buttonToggle;
    LayeredMenuButton lfoModeButton;

    RotaryDial reverbControl { "REVERB", RotaryDial::LabelPlacement::Below };
    RotaryDial volumeControl { "VOLUME", RotaryDial::LabelPlacement::Below };

    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> gateAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
