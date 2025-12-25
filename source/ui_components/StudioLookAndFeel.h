#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "RotaryDial.h"
#include "StudioStyle.h"

// Centralised styling (fonts + colours) for the plugin UI.
class StudioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    StudioLookAndFeel()
    {
        // Enforce a consistent sans-serif base.
        setDefaultSansSerifTypefaceName("Arial");

        setColour(juce::Label::textColourId, StudioStyle::Colours::foreground);
        setColour(juce::Slider::rotarySliderFillColourId, StudioStyle::Colours::accent);
        setColour(juce::Slider::rotarySliderOutlineColourId, StudioStyle::Colours::background);
        setColour(juce::PopupMenu::textColourId, StudioStyle::Colours::foreground);
        setColour(juce::PopupMenu::backgroundColourId, StudioStyle::Colours::background);
    }

    juce::Font getLabelFont(juce::Label& label) override
    {
        const auto h = (float) label.getHeight();
        const float size = juce::jlimit(10.0f, 18.0f, h * 0.8f);
        return makeArialBold(size);
    }

    juce::Font getSliderPopupFont(juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);
        return makeArialBold(14.0f);
    }

    juce::Font getPopupMenuFont() override
    {
        return makeArialBold(14.0f);
    }

    // Custom rotary dial drawing for our RotaryDialSlider.
    void drawRotarySlider(juce::Graphics& g,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& s) override;

private:
    juce::Font makeArialBold(float heightPx) const
    {
        // Use FontOptions so metrics are consistent with JUCE 8 portable metrics.
        return withDefaultMetrics(juce::FontOptions { "Arial", heightPx, juce::Font::bold });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StudioLookAndFeel)
};

inline void StudioLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                                int x,
                                                int y,
                                                int width,
                                                int height,
                                                float sliderPosProportional,
                                                float rotaryStartAngle,
                                                float rotaryEndAngle,
                                                juce::Slider& s)
{
    auto* dial = dynamic_cast<RotaryDialSlider*>(&s);
    if (dial == nullptr)
    {
        juce::LookAndFeel_V4::drawRotarySlider(g, x, y, width, height,
                                              sliderPosProportional, rotaryStartAngle, rotaryEndAngle, s);
        return;
    }

    const auto area = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(2.0f);
    const auto side = juce::jmin(area.getWidth(), area.getHeight());
    const auto squareArea = area.withSizeKeepingCentre(side, side);

    auto ringArea = squareArea.withSizeKeepingCentre(side * dial->ringScale, side * dial->ringScale);
    ringArea = ringArea.expanded(dial->ringOutsetPx).getIntersection(squareArea);
    ringArea = ringArea.reduced(StudioStyle::Sizes::dialArcRadiusTrimPx);

    const auto ringColour = s.findColour(juce::Slider::rotarySliderOutlineColourId);
    const auto valueColour = s.findColour(juce::Slider::rotarySliderFillColourId);

    // Path::addCentredArc: 0 at 12 o'clock, increasing clockwise.
    const float spanRad = juce::degreesToRadians(dial->ringSpanDegrees);
    const float midAngle = juce::degreesToRadians(dial->ringRotationDegrees);
    const float startAngle = midAngle - spanRad * 0.5f;
    const float endAngle = startAngle + spanRad;

    const auto toCosSinAngle = [](float arcAngle)
    {
        // Convert arc angle (0 at 12) -> cos/sin angle (0 at 3)
        return arcAngle - juce::MathConstants<float>::halfPi;
    };

    const auto centre = ringArea.getCentre();
    const float radius = ringArea.getWidth() * 0.5f;
    const float ringThickness = juce::jmin(dial->ringThicknessPx, radius - 2.0f);
    const auto ringBounds = ringArea.reduced(ringThickness * 0.5f);

    // Background ring
    {
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centre.x, centre.y,
                                    ringBounds.getWidth() * 0.5f,
                                    ringBounds.getHeight() * 0.5f,
                                    0.0f, startAngle, endAngle, true);
        g.setColour(ringColour);
        g.strokePath(backgroundArc, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Value indicator
    if (dial->mode == RotaryDialSlider::DialMode::UnipolarRing)
    {
        const float valueAngle = startAngle + sliderPosProportional * spanRad;
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y,
                               ringBounds.getWidth() * 0.5f,
                               ringBounds.getHeight() * 0.5f,
                               0.0f, startAngle, valueAngle, true);
        g.setColour(valueColour);
        g.strokePath(valueArc, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (dial->mode == RotaryDialSlider::DialMode::BipolarRing)
    {
        const float a0 = midAngle;
        const float a1 = startAngle + sliderPosProportional * spanRad;

        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y,
                               ringBounds.getWidth() * 0.5f,
                               ringBounds.getHeight() * 0.5f,
                               0.0f, juce::jmin(a0, a1), juce::jmax(a0, a1), true);
        g.setColour(valueColour);
        g.strokePath(valueArc, juce::PathStrokeType(ringThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (dial->mode == RotaryDialSlider::DialMode::Needle)
    {
        const float valueAngle = startAngle + sliderPosProportional * spanRad;
        const auto vp = toCosSinAngle(valueAngle);

        g.setColour(valueColour);
        const float needleLen = radius * 0.42f;
        const float needleStart = radius * 0.16f;

        const auto p0 = centre + juce::Point<float>(std::cos(vp), std::sin(vp)) * needleStart;
        const auto p1 = centre + juce::Point<float>(std::cos(vp), std::sin(vp)) * (needleStart + needleLen);
        g.drawLine({ p0.x, p0.y, p1.x, p1.y }, 2.0f);
    }

    // Range text
    if (dial->rangeDisplay == RotaryDialSlider::RangeDisplay::MinMax)
    {
        const auto minText = s.getTextFromValue(s.getMinimum());
        const auto maxText = s.getTextFromValue(s.getMaximum());

        const auto textRadius = radius - ringThickness - 10.0f;
        const auto startP = toCosSinAngle(startAngle);
        const auto endP = toCosSinAngle(endAngle);
        const auto minPos = centre + juce::Point<float>(std::cos(startP), std::sin(startP)) * textRadius;
        const auto maxPos = centre + juce::Point<float>(std::cos(endP), std::sin(endP)) * textRadius;

        g.setColour(StudioStyle::Colours::foreground.withAlpha(0.75f));
        g.setFont(makeArialBold(10.5f));

        const auto rMin = juce::Rectangle<float>(0, 0, 48, 16).withCentre(minPos);
        const auto rMax = juce::Rectangle<float>(0, 0, 48, 16).withCentre(maxPos);
        g.drawFittedText(minText, rMin.toNearestInt(), juce::Justification::centred, 1);
        g.drawFittedText(maxText, rMax.toNearestInt(), juce::Justification::centred, 1);
    }

    // Dial image (between ring and value text)
    if (dial->dialImage.isValid())
    {
        auto imageBounds = squareArea.reduced(ringThickness + 6.0f);
        imageBounds = imageBounds.translated(StudioStyle::Sizes::dialImageOffsetPx, StudioStyle::Sizes::dialImageOffsetPx);
        g.setOpacity(1.0f);
        g.drawImageWithin(dial->dialImage,
                          (int) imageBounds.getX(),
                          (int) imageBounds.getY(),
                          (int) imageBounds.getWidth(),
                          (int) imageBounds.getHeight(),
                          juce::RectanglePlacement::centred);
    }

    // Center value text
    {
        const auto valueBounds = squareArea.reduced(ringThickness + 10.0f);
        g.setColour(StudioStyle::Colours::foreground);
        g.setFont(makeArialBold(dial->valueFontHeightPx));

        const auto text = s.getTextFromValue(s.getValue());
        g.drawFittedText(text, valueBounds.toNearestInt(), juce::Justification::centred, 1);
    }
}
