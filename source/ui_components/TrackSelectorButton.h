#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "StudioStyle.h"

class TrackSelectorButton final : public juce::ToggleButton
{
public:
    TrackSelectorButton() = default;

    void setMuted(bool shouldShowMuted)
    {
        if (muted != shouldShowMuted)
        {
            muted = shouldShowMuted;
            repaint();
        }
    }

    // Return true to consume the click (preventing the default toggle behaviour).
    void setMouseDownInterceptor(std::function<bool(const juce::MouseEvent&)> interceptor)
    {
        mouseDownInterceptor = std::move(interceptor);
    }

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

        // A: black rounded-rect background (full size)
        //g.setColour(muted ? mutedOutline : StudioStyle::Colours::buttonOutline);
        g.setColour(muted ?  StudioStyle::Colours::buttonOutline : StudioStyle::Colours::buttonOutline);
        g.fillRoundedRectangle(outer, cornerRadiusPx);

        // B: inner gradient rounded-rect (inset by outlinePx)
        auto b = outer.reduced(StudioStyle::Sizes::buttonOutlinePx);

        const auto light = StudioStyle::Colours::buttonLight.brighter(0.5f);
        const auto dark  = StudioStyle::Colours::buttonDark;

        juce::ColourGradient grad(light, b.getTopLeft(), dark, b.getBottomRight(), false);
        grad.addColour(0.00, light);
        grad.addColour(StudioStyle::Sizes::gradientBandStart, light);
        grad.addColour(StudioStyle::Sizes::gradientBandEnd, dark);
        grad.addColour(1.00, dark);

        if (isButtonDown)
            grad.multiplyOpacity(0.95f);

        g.setGradientFill(grad);
        g.fillRoundedRectangle(b, juce::jmax(0.0f, cornerRadiusPx - StudioStyle::Sizes::buttonOutlinePx));

        // C: midGrey plate (~0.7 of inner) - slightly larger for track buttons
        const auto side = juce::jmin(b.getWidth(), b.getHeight());
        constexpr float plateMul = 1.10f;
        auto plateBounds = b.withSizeKeepingCentre(side * (StudioStyle::Sizes::buttonPlateScale * plateMul),
                               side * (StudioStyle::Sizes::buttonPlateScale * plateMul));

        const float plateCorner = juce::jmin(cornerRadiusPx * 0.44f, plateBounds.getHeight() * 0.22f);
        g.setColour(StudioStyle::Colours::buttonPlate);
        g.fillRoundedRectangle(plateBounds, plateCorner);

        // Icon (same behaviour as ImageToggle: only ON/OFF artwork changes)
        if (auto* drawable = getToggleState() ? onDrawable.get() : offDrawable.get())
        {
            constexpr float iconMul = 1.10f;
            auto imageBounds = b.withSizeKeepingCentre(side * (StudioStyle::Sizes::buttonIconScale * iconMul),
                                                       side * (StudioStyle::Sizes::buttonIconScale * iconMul));
            drawable->drawWithin(g, imageBounds, juce::RectanglePlacement::centred, 1.0f);
        }

        // Label
        auto text = getButtonText();
        if (text.isNotEmpty())
        {
            auto textBounds = plateBounds.toNearestInt();
            g.setFont(StudioStyle::Fonts::trackButtonLabelFont());

            // Always white (ON/OFF is communicated only via the artwork).
            g.setColour(StudioStyle::Colours::foreground);
            g.drawFittedText(text, textBounds, juce::Justification::centred, 1);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        consumedClick = mouseDownInterceptor != nullptr && mouseDownInterceptor(e);
        if (consumedClick)
            return;

        juce::ToggleButton::mouseDown(e);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (consumedClick)
        {
            consumedClick = false;
            return;
        }

        juce::ToggleButton::mouseUp(e);
    }

private:
    static std::unique_ptr<juce::Drawable> createDrawableFromData(const void* data, int bytes)
    {
        if (data == nullptr || bytes <= 0)
            return {};

        return juce::Drawable::createFromImageData(data, (size_t) bytes);
    }

    float cornerRadiusPx { StudioStyle::Sizes::buttonCornerRadiusPx };

    bool muted { false };
    bool consumedClick { false };
    std::function<bool(const juce::MouseEvent&)> mouseDownInterceptor;

    // Slightly "reddish" outline for muted tracks.
    juce::Colour mutedOutline { StudioStyle::Colours::accent.darker(0.05f) };

    std::unique_ptr<juce::Drawable> offDrawable;
    std::unique_ptr<juce::Drawable> onDrawable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackSelectorButton)
};
