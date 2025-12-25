#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class LabeledSlider final : public juce::Component
{
public:
    enum class Style
    {
        Horizontal,
        Vertical
    };

    LabeledSlider(juce::String labelText, Style styleToUse = Style::Horizontal)
        : label({}, std::move(labelText)), style(styleToUse)
    {
        label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(label);

        slider.setSliderStyle(style == Style::Horizontal
                                  ? juce::Slider::LinearHorizontal
                                  : juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 22);
        addAndMakeVisible(slider);
    }

    juce::Slider& getSlider() { return slider; }
    const juce::Slider& getSlider() const { return slider; }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(6);
        auto labelWidth = juce::jmin(120, bounds.getWidth() / 3);

        label.setBounds(bounds.removeFromLeft(labelWidth));
        slider.setBounds(bounds);
    }

private:
    juce::Label label;
    juce::Slider slider;
    Style style;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabeledSlider)
};
