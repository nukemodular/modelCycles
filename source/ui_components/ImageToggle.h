#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "StudioStyle.h"

class ImageToggle final : public juce::ToggleButton
{
public:
    ImageToggle() = default;

    void setCornerRadius(float newRadiusPx)
    {
        cornerRadiusPx = juce::jmax(0.0f, newRadiusPx);
        repaint();
    }

    void setImageInsetScale(float newScale)
    {
        imageInsetScale = juce::jlimit(0.1f, 1.0f, newScale);
        repaint();
    }

    void setGradientColours(juce::Colour newLight, juce::Colour newDark)
    {
        lightGrey = newLight;
        darkGrey = newDark;
        repaint();
    }

    void setImagesFromMemory(const void* offData, int offBytes, const void* onData, int onBytes)
    {
        offDrawable = createDrawableFromData(offData, offBytes);
        onDrawable = createDrawableFromData(onData, onBytes);
        repaint();
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::ignoreUnused(isMouseOverButton, isButtonDown);
        const auto outer = getLocalBounds().toFloat();

        // A: black rounded-rect background (full size)
        g.setColour(StudioStyle::Colours::buttonOutline);
        g.fillRoundedRectangle(outer, cornerRadiusPx);

        // B: inner gradient rounded-rect (inset by outlinePx)
        auto b = outer.reduced(StudioStyle::Sizes::buttonOutlinePx);

        const auto light = lightGrey.brighter(0.5f);

        // Background: diagonal gradient (top-left -> bottom-right) with a narrow blend region.
        juce::ColourGradient grad(light, b.getTopLeft(), darkGrey, b.getBottomRight(), false);
        grad.addColour(0.00, light);
        grad.addColour(StudioStyle::Sizes::gradientBandStart, light);
        grad.addColour(StudioStyle::Sizes::gradientBandEnd, darkGrey);
        grad.addColour(1.00, darkGrey);

        g.setGradientFill(grad);
        g.fillRoundedRectangle(b, juce::jmax(0.0f, cornerRadiusPx - StudioStyle::Sizes::buttonOutlinePx));

        // C: midGrey plate (~0.7 of inner)
        const auto side = juce::jmin(b.getWidth(), b.getHeight());
        auto plateBounds = b.withSizeKeepingCentre(side * StudioStyle::Sizes::buttonPlateScale,
                                                   side * StudioStyle::Sizes::buttonPlateScale);

        const float plateCorner = juce::jmin(cornerRadiusPx * 0.44f, plateBounds.getHeight() * 0.22f);
        g.setColour(midGrey);
        g.fillRoundedRectangle(plateBounds, plateCorner);

        // D: icon (~0.6 of inner)
        auto* drawable = getToggleState() ? onDrawable.get() : offDrawable.get();
        if (drawable != nullptr)
        {
            auto imageBounds = b.withSizeKeepingCentre(side * StudioStyle::Sizes::buttonIconScale,
                                                       side * StudioStyle::Sizes::buttonIconScale);
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

    float cornerRadiusPx { StudioStyle::Sizes::buttonCornerRadiusPx };
    float imageInsetScale { StudioStyle::Sizes::buttonImageInsetScale };

    juce::Colour lightGrey { StudioStyle::Colours::buttonLight };
    juce::Colour darkGrey  { StudioStyle::Colours::buttonDark };
    juce::Colour midGrey   { StudioStyle::Colours::buttonPlate };

    std::unique_ptr<juce::Drawable> offDrawable;
    std::unique_ptr<juce::Drawable> onDrawable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImageToggle)
};
