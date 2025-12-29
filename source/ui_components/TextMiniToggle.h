#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "StudioStyle.h"

class TextMiniToggle final : public juce::ToggleButton
{
public:
    TextMiniToggle() = default;

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

    void setGlyphText(juce::String newText)
    {
        glyphText = std::move(newText);
        repaint();
    }

    void setGlyphColour(juce::Colour c)
    {
        glyphColour = c;
        repaint();
    }

    void setGlyphFont(juce::Font f)
    {
        glyphFont = std::move(f);
        repaint();
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isMouseOverButton, isButtonDown);

        const auto outer = getLocalBounds().toFloat();

        // Black rounded rectangle background.
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(outer, cornerRadiusPx);

        // Optional SVG artwork (e.g. BUTTON_OFF/BUTTON_ON) under the glyph.
        if (auto* drawable = getToggleState() ? onDrawable.get() : offDrawable.get())
        {
            auto imageBounds = outer.reduced(2.0f);
            drawable->drawWithin(g, imageBounds, juce::RectanglePlacement::centred, 1.0f);
        }

        g.setColour(glyphColour);
        g.setFont(glyphFont);

        const auto inset = getLocalBounds().reduced(1);
        g.drawFittedText(glyphText, inset, juce::Justification::centred, 1);
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

    juce::String glyphText { "S" };
    juce::Colour glyphColour { juce::Colours::white };
    juce::Font glyphFont { StudioStyle::Fonts::condensedBold(12.0f) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextMiniToggle)
};
