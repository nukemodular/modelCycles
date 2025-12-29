#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "StudioStyle.h"

class LayeredMenuButton final : public juce::Component
{
public:
    LayeredMenuButton()
    {
        addChildComponent(combo);

        combo.setLookAndFeel(&popupLookAndFeel);

        combo.onChange = [this]
        {
            selectedIndex = juce::jmax(0, combo.getSelectedItemIndex());
            repaint();
        };

        setWantsKeyboardFocus(true);
    }

    ~LayeredMenuButton() override
    {
        combo.setLookAndFeel(nullptr);
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

        // Also show the overlay SVG in the ComboBox popup menu next to the item.
        if (auto* d = overlayDrawables.back().get())
        {
            if (auto* menu = combo.getRootMenu())
            {
                for (juce::PopupMenu::MenuItemIterator it(*menu, true); it.next();)
                {
                    auto& item = it.getItem();
                    if (item.itemID == itemId)
                    {
                        item.setImage(d->createCopy());
                        break;
                    }
                }
            }
        }

        if (combo.getNumItems() == 1)
            combo.setSelectedItemIndex(0, juce::sendNotification);
        else
            repaint();
    }

    int getSelectedItemId() const { return combo.getSelectedId(); }

    juce::ComboBox& getComboBox() { return combo; }
    const juce::ComboBox& getComboBox() const { return combo; }

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

                    if (auto recoloured = recolourDrawableForButton(overlay, juce::Colours::white))
                        recoloured->drawWithin(g, overlayBounds, juce::RectanglePlacement::centred, 1.0f);
                    else
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

    static std::unique_ptr<juce::Drawable> recolourDrawableForButton(const juce::Drawable* source, juce::Colour target)
    {
        if (source == nullptr)
            return {};

        auto copy = source->createCopy();
        if (copy == nullptr)
            return {};

        // The shipped SVGs are monochrome; replace common "foreground" colours.
        copy->replaceColour(juce::Colours::black, target);
        copy->replaceColour(juce::Colour(0xFF021616), target);
        return copy;
    }

    juce::ComboBox combo;

    struct PopupLookAndFeel final : public juce::LookAndFeel_V4
    {
        static std::unique_ptr<juce::Drawable> recolourIconForMenu(const juce::Drawable* source, juce::Colour target)
        {
            if (source == nullptr)
                return {};

            auto copy = source->createCopy();
            if (copy == nullptr)
                return {};

            // The shipped SVGs are monochrome; replace common "foreground" colours.
            copy->replaceColour(juce::Colours::black, target);
            copy->replaceColour(juce::Colour(0xFF021616), target);
            return copy;
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            juce::ignoreUnused(width, height);
            g.fillAll(juce::Colour(0xFFCEE5E8));
        }

        void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                       int& idealWidth, int& idealHeight) override
        {
            juce::LookAndFeel_V4::getIdealPopupMenuItemSize(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);

            if (isSeparator)
                return;

            auto f = StudioStyle::Fonts::alphaComboMenuFont();
            f.setHeight(juce::jmax(10.0f, f.getHeight() - 2.0f));

            const int iconSide = 20;
            juce::GlyphArrangement ga;
            ga.addLineOfText(f, text, 0.0f, 0.0f);
            const int textW = juce::roundToInt(ga.getBoundingBox(0, -1, true).getWidth());

            // Tighten menu width: icon + gap + text + margins.
            constexpr int minW = 96;
            constexpr int maxW = 150;
            constexpr int padW = 36;
            idealWidth = juce::jlimit(minW, maxW, iconSide + 8 + textW + padW);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                               bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColourToUse) override
        {
            juce::ignoreUnused(isActive, hasSubMenu, shortcutKeyText, textColourToUse);

            const auto normalFg = juce::Colour(0xFF021616);
            const auto selectedBg = juce::Colour(0xFF021616);
            const auto selectedFg = juce::Colour(0xFFCEE5E8);

            if (isSeparator)
            {
                g.setColour(normalFg.withAlpha(0.25f));
                g.fillRect(area.withHeight(1).withY(area.getCentreY()));
                return;
            }

            auto r = area.reduced(4);

            if (isTicked)
            {
                g.setColour(selectedBg);
                g.fillRoundedRectangle(r.toFloat(), 4.0f);
            }
            else if (isHighlighted)
            {
                g.setColour(normalFg.withAlpha(0.10f));
                g.fillRoundedRectangle(r.toFloat(), 4.0f);
            }

            const int iconSide = 20;
            const int iconX = r.getX();
            const int iconY = r.getCentreY() - iconSide / 2;

            // Draw per-item SVG icon (FREE/TRG/HOLD/ONE/HALF etc.), coloured like the menu font.
            if (icon != nullptr)
            {
                auto iconBounds = juce::Rectangle<int>(iconX, iconY, iconSide, iconSide).toFloat();
                const auto iconColour = isTicked ? selectedFg : normalFg;
                if (auto recoloured = recolourIconForMenu(icon, iconColour))
                    recoloured->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
                else
                    icon->drawWithin(g, iconBounds, juce::RectanglePlacement::centred, 1.0f);
            }

            auto textArea = r.withTrimmedLeft(iconSide + 8);
            g.setColour(isTicked ? selectedFg : normalFg);
            {
                auto f = StudioStyle::Fonts::alphaComboMenuFont();
                f.setHeight(juce::jmax(10.0f, f.getHeight() - 2.0f));
                g.setFont(f);
            }
            g.drawFittedText(text, textArea, juce::Justification::centredLeft, 1);
        }
    };

    PopupLookAndFeel popupLookAndFeel;

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
