#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "StudioStyle.h"

class LayeredMenuButton final : public juce::Component
{
public:
    LayeredMenuButton()
    {
        addChildComponent(combo);

        combo.onChange = [this]
        {
            selectedIndex = juce::jmax(0, combo.getSelectedItemIndex());
            repaint();
        };

        setWantsKeyboardFocus(true);
    }

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

    void setPlateColour(juce::Colour newColour)
    {
        midGrey = newColour.brighter(0.4f);
        repaint();
    }

    void setImagesFromMemory(const void* baseData, int baseBytes)
    {
        baseDrawable = createDrawableFromData(baseData, baseBytes);
        repaint();
    }

    void clearItems()
    {
        combo.clear(juce::dontSendNotification);
        overlayDrawables.clear();
        selectedIndex = 0;
        repaint();
    }

    void addItem(int itemId, juce::String text, const void* overlayData, int overlayBytes)
    {
        combo.addItem(text, itemId);
        overlayDrawables.emplace_back(createDrawableFromData(overlayData, overlayBytes));

        if (combo.getNumItems() == 1)
            combo.setSelectedItemIndex(0, juce::sendNotification);
        else
            repaint();
    }

    int getSelectedItemId() const { return combo.getSelectedId(); }

    void paint(juce::Graphics& g) override
    {
        const auto outer = getLocalBounds().toFloat();

        // A: black rounded-rect background (full size)
        g.setColour(StudioStyle::Colours::buttonOutline);
        g.fillRoundedRectangle(outer, cornerRadiusPx);

        // B: inner gradient rounded-rect (inset by outlinePx)
        auto b = outer.reduced(StudioStyle::Sizes::buttonOutlinePx);


        const auto light = lightGrey.brighter(0.5f);
        juce::ColourGradient grad(light, b.getTopLeft(), darkGrey, b.getBottomRight(), false);
        grad.addColour(0.00, light);
        grad.addColour(StudioStyle::Sizes::gradientBandStart, light);
        grad.addColour(StudioStyle::Sizes::gradientBandEnd, darkGrey);
        grad.addColour(1.00, darkGrey);

        g.setGradientFill(grad);
        g.fillRoundedRectangle(b, juce::jmax(0.0f, cornerRadiusPx - StudioStyle::Sizes::buttonOutlinePx));

        const auto side = juce::jmin(b.getWidth(), b.getHeight());

        // C: midGrey plate (~0.7 of inner)
        auto plateBounds = b.withSizeKeepingCentre(side * StudioStyle::Sizes::buttonPlateScale,
                                                   side * StudioStyle::Sizes::buttonPlateScale);

        const float plateCorner = juce::jmin(cornerRadiusPx * 0.45f, plateBounds.getHeight() * 0.12f);
        g.setColour(midGrey);
        g.fillRoundedRectangle(plateBounds, plateCorner);

        // D: base icon (~0.6 of inner)
        if (baseDrawable != nullptr)
        {
            auto baseBounds = b.withSizeKeepingCentre(side * StudioStyle::Sizes::buttonIconScale,
                                                      side * StudioStyle::Sizes::buttonIconScale);
            baseDrawable->drawWithin(g, baseBounds, juce::RectanglePlacement::centred, 1.0f);

            if (selectedIndex >= 0 && selectedIndex < (int) overlayDrawables.size())
            {
                if (auto* overlay = overlayDrawables[(size_t) selectedIndex].get())
                {
                    auto overlayBounds = baseBounds.withSizeKeepingCentre(baseBounds.getWidth() * StudioStyle::Sizes::buttonOverlayScale,
                                                                          baseBounds.getHeight() * StudioStyle::Sizes::buttonOverlayScale);
                    overlay->drawWithin(g, overlayBounds, juce::RectanglePlacement::centred, 1.0f);
                }
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        juce::ignoreUnused(e);
        combo.showPopup();
    }

    void resized() override
    {
        combo.setBounds(getLocalBounds());
    }

private:
    static std::unique_ptr<juce::Drawable> createDrawableFromData(const void* data, int bytes)
    {
        if (data == nullptr || bytes <= 0)
            return {};

        return juce::Drawable::createFromImageData(data, (size_t) bytes);
    }

    juce::ComboBox combo;

    float cornerRadiusPx { StudioStyle::Sizes::buttonCornerRadiusPx };
    float imageInsetScale { StudioStyle::Sizes::buttonImageInsetScale };

    juce::Colour lightGrey { StudioStyle::Colours::buttonLight };
    juce::Colour darkGrey  { StudioStyle::Colours::buttonDark };
    juce::Colour midGrey   { StudioStyle::Colours::buttonPlate };

    std::unique_ptr<juce::Drawable> baseDrawable;
    std::vector<std::unique_ptr<juce::Drawable>> overlayDrawables;
    int selectedIndex { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayeredMenuButton)
};
