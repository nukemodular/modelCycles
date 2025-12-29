#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "OverlayDial.h"

class LfoOverlayPanel final : public juce::Component
{
public:
    LfoOverlayPanel()
    {
        setOpaque(true);

        addAndMakeVisible(multiplyDial);
        addAndMakeVisible(phaseDial);
        addAndMakeVisible(depthDial);
        addAndMakeVisible(fadeDial);

        // Ensure the dial square matches the requested visible arc size.
        setDesiredDialArcSidePx(desiredDialArcSidePx);

        // Requested: move LFO overlay dial labels (MULTIPLY/PHASE/DEPTH/FADE) down by 2px.
        for (auto* d : { &multiplyDial, &phaseDial, &depthDial, &fadeDial })
            d->setLabelYOffsetPx(StudioStyle::Sizes::overlayDialLabelYOffsetPx + 2);

        waveformLabel.setText("WAVEFORM", juce::dontSendNotification);
        waveformLabel.setJustificationType(juce::Justification::centred);
        waveformLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(waveformLabel);

        addAndMakeVisible(waveformComboBackdrop);
        waveformComboBackdrop.setInterceptsMouseClicks(false, false);

        destinationLabel.setText("DESTINATION", juce::dontSendNotification);
        destinationLabel.setJustificationType(juce::Justification::centred);
        destinationLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(destinationLabel);

        addAndMakeVisible(destinationComboBackdrop);
        destinationComboBackdrop.setInterceptsMouseClicks(false, false);

        waveformCombo.setLookAndFeel(&comboLookAndFeel);
        destinationCombo.setLookAndFeel(&comboLookAndFeel);

        addAndMakeVisible(waveformCombo);
        addAndMakeVisible(destinationCombo);

        waveformCombo.addItem("TRI", 1);
        waveformCombo.addItem("SIN", 2);
        waveformCombo.addItem("SQR", 3);
        waveformCombo.addItem("SAW", 4);
        waveformCombo.addItem("ENV", 5);
        waveformCombo.addItem("SAW-HLF", 6);
        waveformCombo.addItem("S&H", 7);
        waveformCombo.setSelectedId(1, juce::dontSendNotification);

        destinationCombo.addItem(" --- ", 1);
        destinationCombo.addItem("PTCH", 2);
        destinationCombo.addItem("FTUN", 3);
        destinationCombo.addItem("DEC", 4);
        destinationCombo.addItem("COLR", 5);
        destinationCombo.addItem("SHPE", 6);
        destinationCombo.addItem("SWEP", 7);
        destinationCombo.addItem("CONT", 8);
        destinationCombo.addItem("DELS", 9);
        destinationCombo.addItem("REVS", 10);
        destinationCombo.addItem("DIST", 11);
        destinationCombo.addItem("PAN", 12);
        destinationCombo.addItem("PAW", 13);
        destinationCombo.addItem("GATE", 14);
        destinationCombo.setSelectedId(1, juce::dontSendNotification);

        applyComboColoursAndFonts();
    }

    ~LfoOverlayPanel() override
    {
        waveformCombo.setLookAndFeel(nullptr);
        destinationCombo.setLookAndFeel(nullptr);
    }

    // Fixed size for the visible dial arc/knob square (label area is additional).
    void setDesiredDialArcSidePx(int arcSidePx)
    {
        desiredDialArcSidePx = juce::jmax(0, arcSidePx);
        for (auto* d : { &multiplyDial, &phaseDial, &depthDial, &fadeDial })
            d->setArcSidePx(desiredDialArcSidePx);
        resized();
    }

    void setDialImageFromMemory(const void* data, int dataSizeBytes)
    {
        multiplyDial.setDialImageFromMemory(data, dataSizeBytes);
        phaseDial.setDialImageFromMemory(data, dataSizeBytes);
        depthDial.setDialImageFromMemory(data, dataSizeBytes);
        fadeDial.setDialImageFromMemory(data, dataSizeBytes);
    }

    void setMiniDialUIScale(float scale)
    {
        miniDialUIScale = scale;

        multiplyDial.setUIScale(miniDialUIScale);
        phaseDial.setUIScale(miniDialUIScale);
        depthDial.setUIScale(miniDialUIScale);
        fadeDial.setUIScale(miniDialUIScale);

        applyComboColoursAndFonts();
        resized();
    }

    OverlayDial& getMultiplyDial() { return multiplyDial; }
    OverlayDial& getPhaseDial() { return phaseDial; }
    OverlayDial& getDepthDial() { return depthDial; }
    OverlayDial& getFadeDial() { return fadeDial; }

    juce::ComboBox& getWaveformCombo() { return waveformCombo; }
    juce::ComboBox& getDestinationCombo() { return destinationCombo; }

    // Column centre positions in *local* coordinates (relative to this panel).
    // If set, overlay elements will align exactly to the underlying main dials.
    void setColumnCentresX(int leftX, int midX, int rightX)
    {
        columnCentresX[0] = leftX;
        columnCentresX[1] = midX;
        columnCentresX[2] = rightX;
        haveColumnCentres = true;
        resized();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(4);
        constexpr int rowGapPx = 10;
        const int rowH = juce::jmax(1, (area.getHeight() - rowGapPx) / 2);

        auto row1 = area.removeFromTop(rowH);
        area.removeFromTop(rowGapPx);
        auto row2 = area;

        layoutRow(row1, multiplyDial, waveformLabel, waveformComboBackdrop, waveformCombo, phaseDial);
        layoutRow(row2, depthDial, destinationLabel, destinationComboBackdrop, destinationCombo, fadeDial);
    }

    void paint(juce::Graphics& g) override
    {
        // Match the editor canvas background so this reads like a "mode" overlay.
        g.fillAll(juce::Colour(0xFFAAB0B0));
    }

private:
    struct ComboBackdrop final : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            const float r = 6.0f;

            // Match the pattern selector style: gradient outer band + solid inner fill.
            const auto light = StudioStyle::Colours::canvas.brighter(0.5f);
            const auto dark = StudioStyle::Colours::canvas.darker(0.5f);

                        // Diagonal blend band: shift the left side down (+10) and the right side up (-20)
            // so the transition sits towards the lower-left and upper-right corners.
            const auto darkPoint  = bounds.getTopLeft().translated(30.0f, -5.0f);
            const auto lightPoint = bounds.getBottomRight().translated(-60.0f,  -5.0f);

            //juce::ColourGradient grad(dark, bounds.getTopLeft(), light, bounds.getBottomRight(), false);
            juce::ColourGradient grad(dark, darkPoint, light, lightPoint, false);
            constexpr double bandStart = 0.64;
            constexpr double bandEnd   = 0.76;
            grad.addColour(0.00, dark);
            grad.addColour(bandStart, dark);
            grad.addColour(bandEnd, light);
            grad.addColour(1.00, light);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, r);

            auto inner = bounds.reduced(3.0f);
            g.setColour(base);
            g.fillRoundedRectangle(inner, juce::jmax(0.0f, r - 3.0f));
        }

        juce::Colour base { juce::Colour(0xFFCEE5E8) };
    };

    struct ComboLookAndFeel final : public juce::LookAndFeel_V4
    {
        juce::Font getComboBoxFont(juce::ComboBox& box) override
        {
            juce::ignoreUnused(box);
            return StudioStyle::Fonts::alphaComboTextFont();
        }

        juce::Font getPopupMenuFont() override
        {
            return StudioStyle::Fonts::alphaComboMenuFont();
        }

        void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                       int& idealWidth, int& idealHeight) override
        {
            juce::LookAndFeel_V4::getIdealPopupMenuItemSize(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);

            if (isSeparator)
                return;

            const auto f = getPopupMenuFont();
            juce::GlyphArrangement ga;
            ga.addLineOfText(f, text, 0.0f, 0.0f);
            const int textW = juce::roundToInt(ga.getBoundingBox(0, -1, true).getWidth());

            // Keep these menus compact (they were reading too wide).
            constexpr int minW = 84;
            constexpr int maxW = 170;
            constexpr int padW = 44;
            idealWidth = juce::jlimit(minW, maxW, textW + padW);
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool,
                          int, int, int, int, juce::ComboBox& box) override
        {
            // Backdrop draws the rounded-rectangle gradient.
            juce::ignoreUnused(g, width, height, box);
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            // Give the text more horizontal room (avoid squeezed-looking glyphs for long items
            // like "SAW-HLF") without changing the font size.
            label.setBounds(box.getLocalBounds().reduced(0, 2).translated(0, 2));
            label.setFont(getComboBoxFont(box));
            label.setJustificationType(juce::Justification::centred);
            // Never horizontally scale the font to fit; prefer truncation if it ever overflows.
            label.setMinimumHorizontalScale(1.0f);
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            g.fillAll(menuBg);
            juce::ignoreUnused(width, height);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                               bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColourToUse) override
        {
            juce::ignoreUnused(isActive, hasSubMenu, shortcutKeyText, icon, textColourToUse);

            const auto selectedBg = textFg;
            const auto selectedFg = menuBg;

            if (isSeparator)
            {
                g.setColour(textFg.withAlpha(0.25f));
                g.fillRect(area.withHeight(1).withY(area.getCentreY()));
                return;
            }

            // Make selection/highlight rows taller.
            auto r = area.reduced(4, 0);
            if (isTicked)
            {
                g.setColour(selectedBg);
                g.fillRoundedRectangle(r.toFloat(), 5.0f);
            }
            else if (isHighlighted)
            {
                g.setColour(textFg.withAlpha(0.12f));
                g.fillRoundedRectangle(r.toFloat(), 5.0f);
            }

            g.setColour(isTicked ? selectedFg : textFg);
            g.setFont(getPopupMenuFont());

            g.drawFittedText(text, r, juce::Justification::centred, 1);
        }

        juce::Colour menuBg { juce::Colour(0xFFCEE5E8) };
        juce::Colour textFg { juce::Colour(0xFF021616) };
    };

    void applyComboColoursAndFonts()
    {
        const auto bg = juce::Colour(0xFFCEE5E8);
        const auto fg = juce::Colour(0xFF021616);

        for (auto* c : { &waveformCombo, &destinationCombo })
        {
            c->setColour(juce::ComboBox::backgroundColourId, bg);
            c->setColour(juce::ComboBox::textColourId, fg);
            c->setColour(juce::ComboBox::arrowColourId, fg);
            c->setColour(juce::ComboBox::outlineColourId, fg.withAlpha(0.25f));
            c->setColour(juce::ComboBox::buttonColourId, bg);
        }

        for (auto* l : { &waveformLabel, &destinationLabel })
        {
            l->setColour(juce::Label::textColourId, fg);
            l->setFont(StudioStyle::Fonts::smallLabelFont());
        }

        // Match dial labels to the spec's dark foreground.
        multiplyDial.setLabelColour(fg);
        phaseDial.setLabelColour(fg);
        depthDial.setLabelColour(fg);
        fadeDial.setLabelColour(fg);
    }

    void layoutRow(juce::Rectangle<int> row,
                  OverlayDial& leftDial,
                  juce::Label& midLabel,
                  juce::Component& midComboBackdrop,
                  juce::ComboBox& midCombo,
                  OverlayDial& rightDial)
    {
        const int colW = row.getWidth() / 3;
        auto col1 = row.removeFromLeft(colW);
        auto col2 = row.removeFromLeft(colW);
        auto col3 = row;

        const int rowH = juce::jmax(1, row.getHeight());

        // OverlayDial compensates internally so the visible arc is desiredDialArcSidePx.
        // Width needs to account for the LookAndFeel reductions (+10).
        const int dialW = juce::jmax(20, desiredDialArcSidePx + 10);
        const int dialLabelH = (int) juce::roundToInt(18.0f * miniDialUIScale);
        const int dialGapH = (int) juce::roundToInt(4.0f * miniDialUIScale);
        const int dialH = juce::jmax(24, dialW + dialGapH + dialLabelH);

        const int dialCentreY = row.getCentreY();

        const int leftCentreX  = haveColumnCentres ? columnCentresX[0] : col1.getCentreX();
        const int midCentreX   = haveColumnCentres ? columnCentresX[1] : col2.getCentreX();
        const int rightCentreX = haveColumnCentres ? columnCentresX[2] : col3.getCentreX();

        leftDial.setBounds(juce::Rectangle<int>(0, 0, dialW, dialH).withCentre({ leftCentreX, dialCentreY }));
        rightDial.setBounds(juce::Rectangle<int>(0, 0, dialW, dialH).withCentre({ rightCentreX, dialCentreY }));

        // Combos: slightly larger/taller and much wider so long items (e.g. SAW-HALF) don't squeeze.
        const int baseComboH = juce::jlimit(20, 34, (int) std::floor((float) rowH * 0.40f));
        const int comboH = juce::jlimit(24, 46, (int) std::lround((float) baseComboH * 1.55f));
        const int gap = 4;

        auto mid = col2.reduced(4);
        const int comboW = juce::jmax(40, mid.getWidth() - 14);

        // Align WAVEFORM/DESTINATION label with the dial labels (MULTIPLY/PHASE and DEPTH/FADE).
        // OverlayDial places its label in the bottom area and then translates it up slightly.
        constexpr int dialLabelRaisePx = 11;
        const int dialLabelTop = leftDial.getY() + (dialH - dialLabelH) - dialLabelRaisePx;

        constexpr int labelDownPx = 2;

        midLabel.setBounds(juce::Rectangle<int>(0, 0, comboW, dialLabelH)
                   .withCentre({ midCentreX, dialLabelTop + dialLabelH / 2 + labelDownPx }));

        // Place the combo directly above its label.
        const int comboBottom = dialLabelTop - gap;
        const auto comboBounds = juce::Rectangle<int>(0, 0, comboW, comboH)
                         .withCentre({ midCentreX, comboBottom - comboH / 2 });

        // Requested: shrink the actual ComboBox by 3px so the backdrop reads as a ~3px outline.
        midComboBackdrop.setBounds(comboBounds);
        // Keep the backdrop frame, but allow the *text-only* ComboBox to be wider so long
        // selections (e.g. "SAW-HLF") don't ellipsize.
        auto comboInner = comboBounds.reduced(3);
        midCombo.setBounds(comboInner.expanded(10, 0));
    }

    ComboLookAndFeel comboLookAndFeel;

    OverlayDial multiplyDial { "MULTIPLY" };
    OverlayDial phaseDial { "PHASE" };
    OverlayDial depthDial { "DEPTH" };
    OverlayDial fadeDial { "FADE" };

    juce::Label waveformLabel;
    ComboBackdrop waveformComboBackdrop;
    juce::ComboBox waveformCombo;

    juce::Label destinationLabel;
    ComboBackdrop destinationComboBackdrop;
    juce::ComboBox destinationCombo;

    int desiredDialArcSidePx { 48 };

    float miniDialUIScale { 1.0f };

    bool haveColumnCentres { false };
    int columnCentresX[3] { 0, 0, 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfoOverlayPanel)
};
