#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class OverlayMiniToggle final : public juce::ToggleButton
{
public:
    OverlayMiniToggle() = default;

    void setImagesFromMemory(const void* offData, int offBytes, const void* onData, int onBytes)
    {
        offDrawable = createDrawableFromData(offData, offBytes);
        onDrawable = createDrawableFromData(onData, onBytes);
        repaint();
    }

    void setCornerRadius(float newRadiusPx)
    {
        cornerRadiusPx = juce::jmax(0.0f, newRadiusPx);
        repaint();
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isMouseOverButton, isButtonDown);

        const auto outer = getLocalBounds().toFloat();

        // Black rounded rectangle background.
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(outer, cornerRadiusPx);

        // Inset SVG artwork by 2px on all sides.
        if (auto* drawable = getToggleState() ? onDrawable.get() : offDrawable.get())
        {
            auto imageBounds = outer.reduced(2.0f);
            drawable->drawWithin(g, imageBounds, juce::RectanglePlacement::centred, 1.0f);
        }
    }

private:
    static std::unique_ptr<juce::Drawable> createDrawableFromData(const void* data, int bytes)
    {
        if (data == nullptr || bytes <= 0)
            return {};

        return juce::Drawable::createFromImageData(data, (size_t) bytes);
    }

    float cornerRadiusPx { 4.0f };

    std::unique_ptr<juce::Drawable> offDrawable;
    std::unique_ptr<juce::Drawable> onDrawable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayMiniToggle)
};
