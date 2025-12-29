#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>

#include "StudioStyle.h"

class RotaryDialSlider final : public juce::Slider
{
public:
    RotaryDialSlider()
        : juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox)
    {
    }

    bool hitTest(int x, int y) override
    {
        const auto area = getLocalBounds().toFloat().reduced(2.0f);
        const auto side = juce::jmin(area.getWidth(), area.getHeight());
        const auto squareArea = area.withSizeKeepingCentre(side, side);

        auto ringArea = squareArea.withSizeKeepingCentre(side * ringScale, side * ringScale);
        ringArea = ringArea.expanded(ringOutsetPx).getIntersection(squareArea);
        ringArea = ringArea.reduced(StudioStyle::Sizes::dialArcRadiusTrimPx);

        const auto centre = ringArea.getCentre();
        const float radius = ringArea.getWidth() * 0.5f;

        const float dx = (float) x - centre.x;
        const float dy = (float) y - centre.y;

        return (dx * dx + dy * dy) <= (radius * radius);
    }

    enum class DialMode
    {
        UnipolarRing,
        BipolarRing,
        Needle
    };

    enum class RangeDisplay
    {
        None,
        MinMax
    };

    DialMode mode { DialMode::UnipolarRing };
    RangeDisplay rangeDisplay { RangeDisplay::None };

    float ringSpanDegrees { 300.0f };
    float ringScale { 0.7f };

    // Positive values push the ring outward (increase radius) without changing the artwork bounds.
    float ringOutsetPx { 3.0f };

    // Rotation of the *centre* of the ring span, in degrees.
    // Note: JUCE arcs use 0 radians at 12 o'clock, increasing clockwise.
    // So 0° = 12 o'clock, 90° = 3 o'clock, 180° = 6 o'clock, 270° = 9 o'clock.
    float ringRotationDegrees { 0.0f };

    float ringThicknessPx { 8.0f };
    float valueFontHeightPx { 16.0f };
    juce::Image dialImage;

    // Extra padding between the ring and the dial image. Default matches existing look.
    // Reduce this to let the dial image overlap the arc slightly.
    float dialImagePaddingPx { 6.0f };

    // Optional pixel offset for centering dial artwork.
    float dialImageOffsetPx { StudioStyle::Sizes::dialImageOffsetPx };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryDialSlider)
};

class RotaryDial final : public juce::Component
{
public:
    enum class DialMode
    {
        UnipolarRing,
        BipolarRing,
        Needle
    };

    enum class LabelPlacement
    {
        None,
        Above,
        Below
    };

    enum class RangeDisplay
    {
        None,
        MinMax
    };

    explicit RotaryDial(juce::String labelText = {},
                       LabelPlacement labelPlacementToUse = LabelPlacement::Below)
        : label({}, labelText.toUpperCase()), labelPlacement(labelPlacementToUse)
    {
        label.setJustificationType(juce::Justification::centred);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupDisplayEnabled(false, false, this);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFFD5252)); // 0xFD5252 
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
        addAndMakeVisible(slider);

        setLabelPlacement(labelPlacementToUse);
    }

    ~RotaryDial() override = default;

    juce::Slider& getSlider() { return slider; }
    const juce::Slider& getSlider() const { return slider; }

    void setDialMode(DialMode newMode)
    {
        slider.mode = static_cast<RotaryDialSlider::DialMode>(newMode);
        slider.repaint();
    }

    DialMode getDialMode() const { return static_cast<DialMode>(slider.mode); }

    void setLabelText(juce::String newText)
    {
        label.setText(newText.toUpperCase(), juce::dontSendNotification);
        resized();
    }

    void setLabelColour(juce::Colour newColour)
    {
        label.setColour(juce::Label::textColourId, newColour);
        label.repaint();
    }

    void setLabelPlacement(LabelPlacement placement)
    {
        labelPlacement = placement;
        label.setVisible(labelPlacement != LabelPlacement::None);
        resized();
    }

    // How far the label is pulled upwards (into the dial area). Default matches the original look.
    void setLabelRaisePx(float newRaisePx)
    {
        labelRaiseBasePx = juce::jmax(0.0f, newRaisePx);
        resized();
    }

    void setUIScale(float newScale)
    {
        uiScale = juce::jlimit(0.5f, 2.0f, newScale);
        resized();
        repaint();
    }

    float getUIScale() const { return uiScale; }

    LabelPlacement getLabelPlacement() const { return labelPlacement; }

    void setRangeDisplay(RangeDisplay display)
    {
        slider.rangeDisplay = static_cast<RotaryDialSlider::RangeDisplay>(display);
        slider.repaint();
    }

    RangeDisplay getRangeDisplay() const { return static_cast<RangeDisplay>(slider.rangeDisplay); }

    void setRingSpanDegrees(float degrees)
    {
        slider.ringSpanDegrees = juce::jlimit(10.0f, 359.9f, degrees);
        slider.repaint();
    }

    void setRingScale(float scale)
    {
        slider.ringScale = juce::jlimit(0.2f, 1.0f, scale);
        slider.repaint();
    }

    void setRingOutset(float outsetPx)
    {
        slider.ringOutsetPx = juce::jmax(0.0f, outsetPx);
        slider.repaint();
    }

    void setRingRotationDegrees(float degrees)
    {
        slider.ringRotationDegrees = degrees;
        slider.repaint();
    }

    void setRingThickness(float newThicknessPx)
    {
        slider.ringThicknessPx = juce::jmax(1.0f, newThicknessPx);
        slider.repaint();
    }

    void setValueFontHeight(float newFontHeightPx)
    {
        slider.valueFontHeightPx = juce::jmax(6.0f, newFontHeightPx);
        slider.repaint();
    }

    void setDialImage(const juce::Image& newImage)
    {
        slider.dialImage = newImage;
        slider.repaint();
    }

    void setDialImagePadding(float paddingPx)
    {
        slider.dialImagePaddingPx = juce::jmax(0.0f, paddingPx);
        slider.repaint();
    }

    void setDialImageOffset(float offsetPx)
    {
        slider.dialImageOffsetPx = offsetPx;
        slider.repaint();
    }

    void setDialImageFromMemory(const void* data, int dataSizeBytes)
    {
        if (data == nullptr || dataSizeBytes <= 0)
        {
            clearDialImage();
            return;
        }

        setDialImage(juce::ImageCache::getFromMemory(data, dataSizeBytes));
    }

    bool setDialImageFromFile(const juce::File& file)
    {
        if (! file.existsAsFile())
            return false;

        std::unique_ptr<juce::ImageFileFormat> format(juce::ImageFileFormat::findImageFormatForFileExtension(file));
        if (format == nullptr)
            return false;

        std::unique_ptr<juce::InputStream> stream(file.createInputStream());
        if (stream == nullptr)
            return false;

        auto image = format->decodeImage(*stream);
        if (! image.isValid())
            return false;

        setDialImage(image);
        return true;
    }

    void clearDialImage()
    {
        slider.dialImage = {};
        slider.repaint();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        constexpr int labelYOffsetPx = StudioStyle::Sizes::dialLabelYOffsetPx;

        const int labelHeight = (labelPlacement == LabelPlacement::None)
                        ? 0
                        : (int) juce::roundToInt(StudioStyle::Sizes::dialLabelAreaHeightPx * uiScale);
        const int gap = (labelPlacement == LabelPlacement::None)
                    ? 0
                    : (int) juce::roundToInt(StudioStyle::Sizes::dialLabelGapPx * uiScale);
        const int labelRaisePx = (int) juce::roundToInt(labelRaiseBasePx * uiScale);

        if (labelPlacement == LabelPlacement::Above)
        {
            auto labelArea = bounds.removeFromTop(labelHeight);
            bounds.removeFromTop(gap);
            label.setBounds(labelArea.translated(0, labelYOffsetPx));

            label.setFont(StudioStyle::Fonts::dialLabelFont());

            const int side = juce::jmin(bounds.getWidth(), bounds.getHeight());
            auto dialArea = bounds.removeFromTop(side).withSizeKeepingCentre(side, side);
            slider.setBounds(dialArea.reduced(2));
            return;
        }

        if (labelPlacement == LabelPlacement::Below)
        {
            // Reserve label area at the bottom, and keep the dial as a square above it.
            auto labelArea = bounds.removeFromBottom(labelHeight);
            bounds.removeFromBottom(gap);

            const int side = juce::jmin(bounds.getWidth(), bounds.getHeight());
            auto dialArea = bounds.removeFromTop(side).withSizeKeepingCentre(side, side);
            slider.setBounds(dialArea.reduced(2));

            label.setBounds(labelArea.translated(0, -labelRaisePx + labelYOffsetPx));

            label.setFont(StudioStyle::Fonts::dialLabelFont());
            return;
        }

        // No label: just a square dial.
        const int side = juce::jmin(bounds.getWidth(), bounds.getHeight());
        slider.setBounds(bounds.withSizeKeepingCentre(side, side).reduced(2));
    }

private:
    juce::Label label;
    RotaryDialSlider slider;
    LabelPlacement labelPlacement { LabelPlacement::Below };
    float uiScale { 1.0f };
    float labelRaiseBasePx { StudioStyle::Sizes::dialLabelRaiseBasePx };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryDial)
};
