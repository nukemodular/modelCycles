#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "RotaryDial.h"

// A compact dial component intended for overlays.
// Key differences vs RotaryDial:
// - The slider (arc/knob) square has an explicit size (e.g. 48x48)
// - Internal ring/image scaling is tuned so the artwork remains readable at small sizes
// - Label is always below and does not intrude into the dial square
class OverlayDial final : public juce::Component
{
public:
    explicit OverlayDial(juce::String labelText = {})
        : label({}, labelText.toUpperCase())
    {
        label.setJustificationType(juce::Justification::centred);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);

        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupDisplayEnabled(false, false, this);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFFD5252));
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);

        // Tune the internal geometry so it reads well at small sizes.
        slider.ringScale = 1.0f;
        // Reduce arc scale by 2px (shrink ring area slightly).
        slider.ringOutsetPx = -2.0f;
        slider.ringThicknessPx = 6.0f;
        slider.valueFontHeightPx = 14.0f;
        // Recenter artwork and let it overlap the arc slightly.
        slider.dialImageOffsetPx = 0.0f;
        // Increase inner image by +8px total (reduce padding by 4px on each side).
        slider.dialImagePaddingPx = 1.0f;

        addAndMakeVisible(slider);
    }

    juce::Slider& getSlider() { return slider; }
    const juce::Slider& getSlider() const { return slider; }

    void setLabelText(juce::String newText)
    {
        label.setText(newText.toUpperCase(), juce::dontSendNotification);
        resized();
    }

    void setLabelColour(juce::Colour c)
    {
        label.setColour(juce::Label::textColourId, c);
        label.repaint();
    }

    void setUIScale(float newScale)
    {
        uiScale = juce::jlimit(0.5f, 2.0f, newScale);
        resized();
        repaint();
    }

    float getUIScale() const { return uiScale; }

    void setDialMode(RotaryDial::DialMode newMode)
    {
        slider.mode = static_cast<RotaryDialSlider::DialMode>(newMode);
        slider.repaint();
    }

    void setRingSpanDegrees(float degrees)
    {
        slider.ringSpanDegrees = juce::jlimit(10.0f, 359.9f, degrees);
        slider.repaint();
    }

    void setRingRotationDegrees(float degrees)
    {
        slider.ringRotationDegrees = degrees;
        slider.repaint();
    }

    void setRingThickness(float px)
    {
        slider.ringThicknessPx = juce::jmax(1.0f, px);
        slider.repaint();
    }

    void setValueFontHeight(float px)
    {
        slider.valueFontHeightPx = juce::jmax(6.0f, px);
        slider.repaint();
    }

    void setDialImageFromMemory(const void* data, int dataSizeBytes)
    {
        if (data == nullptr || dataSizeBytes <= 0)
        {
            slider.dialImage = {};
            slider.repaint();
            return;
        }

        slider.dialImage = juce::ImageCache::getFromMemory(data, dataSizeBytes);
        slider.repaint();
    }

    // Size of the *visible* dial arc/knob box. Internally we compensate for the
    // StudioLookAndFeel reductions so the arc actually reads at this size.
    void setArcSidePx(int sidePx)
    {
        arcSidePx = juce::jmax(12, sidePx);
        resized();
    }

    int getArcSidePx() const { return arcSidePx; }

    void setLabelYOffsetPx(int newYOffsetPx)
    {
        labelYOffsetPx = newYOffsetPx;
        resized();
        repaint();
    }

    void resized() override
    {
        auto b = getLocalBounds();

        const int labelH = (int) juce::roundToInt(StudioStyle::Sizes::overlayDialLabelAreaHeightPx * uiScale);
        const int gap = (int) juce::roundToInt(StudioStyle::Sizes::overlayDialLabelGapPx * uiScale);

        auto labelArea = b.removeFromBottom(labelH);
        b.removeFromBottom(gap);

        // The StudioLookAndFeel shrinks the drawable square by:
        // - area.reduced(2)  => -4 px
        // - ringArea.reduced(trim=3) => -6 px
        // Total: -10 px.
        // So to get a visible arc box of ~arcSidePx, we allocate +10 px here.
        const int sliderSide = arcSidePx + 10;

        const int side = juce::jmin(sliderSide, b.getWidth(), b.getHeight());
        auto dialArea = b.withSizeKeepingCentre(side, side);
        slider.setBounds(dialArea);

        // Move label up a bit (overlay only), but keep slightly more distance to the dial.
        label.setBounds(labelArea.translated(0, labelYOffsetPx));
        label.setFont(StudioStyle::Fonts::overlayDialLabelFont());
    }

private:
    juce::Label label;
    RotaryDialSlider slider;

    float uiScale { 1.0f };
    int arcSidePx { 48 };
    int labelYOffsetPx { StudioStyle::Sizes::overlayDialLabelYOffsetPx };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OverlayDial)
};
