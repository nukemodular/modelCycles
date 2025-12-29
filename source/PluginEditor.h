#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include <memory>

#include "ui_components/RotaryDial.h"
#include "ui_components/ImageToggle.h"
#include "ui_components/LayeredMenuButton.h"
#include "ui_components/TrackSelectorButton.h"
#include "ui_components/OverlayMiniToggle.h"
#include "ui_components/TextMiniToggle.h"
#include "ui_components/LfoOverlayPanel.h"
#include "ui_components/OverlayDial.h"
#include "ui_components/StudioLookAndFeel.h"

class PluginProcessor;

class PluginEditor final : public juce::AudioProcessorEditor,
                           private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    PluginProcessor& pluginProcessor;

    static constexpr int baseEditorWidthPx = 750;
    static constexpr int baseEditorHeightPx = 500;

    juce::Component uiRoot;

    static constexpr int trackRadioGroupId = 7001;

    StudioLookAndFeel lookAndFeel;

    struct PatternSelectBackdrop final : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            auto outer = getLocalBounds().toFloat();

            const auto light = StudioStyle::Colours::canvas.brighter(0.5f);
            const auto dark  = StudioStyle::Colours::canvas.darker(0.5f);

            // Diagonal blend band: shift the left side down (+10) and the right side up (-20)
            // so the transition sits towards the lower-left and upper-right corners.
            const auto darkPoint  = outer.getTopLeft().translated(0.0f, 0.0f);
            const auto lightPoint = outer.getBottomRight().translated(-40.0f,  0.0f);

            juce::ColourGradient grad(dark, darkPoint, light, lightPoint, false);
            constexpr double bandStart = 0.64;
            constexpr double bandEnd   = 0.76;
            grad.addColour(0.00, dark);
            grad.addColour(bandStart, dark);
            grad.addColour(bandEnd, light);
            grad.addColour(1.00, light);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(outer, cornerRadiusPx);

            auto inner = outer.reduced(3.0f);
            // Match the LFO combo background so the inner fill reads as one control.
            g.setColour(juce::Colour(0xFFCEE5E8));
            g.fillRoundedRectangle(inner, juce::jmax(0.0f, cornerRadiusPx - 3.0f));
        }

        float cornerRadiusPx { 7.0f };
    };

    struct PatternComboLookAndFeel final : public juce::LookAndFeel_V4
    {
        juce::Font getComboBoxFont(juce::ComboBox& box) override
        {
            juce::ignoreUnused(box);
            auto f = StudioStyle::Fonts::alphaComboTextFont();
            f.setHeight(f.getHeight() + 4.0f);
            return f;
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

            // Make the menu tighter than the JUCE default.
            constexpr int minW = 60;
            constexpr int maxW = 120;
            constexpr int padW = 34;
            idealWidth = juce::jlimit(minW, maxW, textW + padW);
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool,
                          int, int, int, int, juce::ComboBox& box) override
        {
            // Text-only: the backdrop draws the shared rounded rectangle.
            juce::ignoreUnused(g, width, height, box);
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            // Use almost the full bounds because this ComboBox is rendered text-only
            // (arrow/background/outline are intentionally hidden).
            label.setBounds(box.getLocalBounds().translated(-2, 2));
            label.setFont(getComboBoxFont(box));
            label.setJustificationType(juce::Justification::centred);
            // Avoid JUCE scaling the font horizontally to fit.
            label.setMinimumHorizontalScale(1.0f);
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            juce::ignoreUnused(width, height);
            g.fillAll(menuBg);
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

            auto r = area.reduced(6, 2);
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

    PatternSelectBackdrop patternSelectBackdrop;
    juce::ComboBox patternBankCombo;
    juce::ComboBox patternIndexCombo;
    juce::Label patternCaptionLabel;
    PatternComboLookAndFeel patternComboLookAndFeel;

    // Row 1
    RotaryDial pitchControl { "PITCH", RotaryDial::LabelPlacement::Below };

    struct PitchNoteBackdrop final : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            auto outer = getLocalBounds().toFloat();

            const auto light = StudioStyle::Colours::canvas.brighter(0.5f);
            const auto dark  = StudioStyle::Colours::canvas.darker(0.5f);

            // juce::ColourGradient grad(dark, outer.getTopLeft(), light, outer.getBottomRight(), false);
            // grad.addColour(0.00, dark);
            // grad.addColour(StudioStyle::Sizes::gradientBandStart, dark);
            // grad.addColour(StudioStyle::Sizes::gradientBandEnd, light);
            // grad.addColour(1.00, light);
            // g.setGradientFill(grad);
            // g.fillRoundedRectangle(outer, cornerRadiusPx);


            const auto darkPoint  = outer.getTopLeft().translated(0.0f, 0.0f);
            const auto lightPoint = outer.getBottomRight().translated(-40.0f,  0.0f);

            juce::ColourGradient grad(dark, darkPoint, light, lightPoint, false);
            constexpr double bandStart = 0.64;
            constexpr double bandEnd   = 0.76;
            grad.addColour(0.00, dark);
            grad.addColour(bandStart, dark);
            grad.addColour(bandEnd, light);
            grad.addColour(1.00, light);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(outer, cornerRadiusPx);

            auto inner = outer.reduced(3.0f);
            g.setColour(StudioStyle::Colours::canvas.darker(0.05f));
            g.fillRoundedRectangle(inner, juce::jmax(0.0f, cornerRadiusPx - 3.0f));
        }

        float cornerRadiusPx { 7.0f };
    };

    struct PitchNoteComboLookAndFeel final : public juce::LookAndFeel_V4
    {
        juce::Font getComboBoxFont(juce::ComboBox& box) override
        {
            juce::ignoreUnused(box);
            return StudioStyle::Fonts::comboTextFont();
        }

        juce::Font getPopupMenuFont() override
        {
            return StudioStyle::Fonts::comboMenuFont();
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

            // Keep NOTE popup menus compact.
            constexpr int minW = 74;
            constexpr int maxW = 160;
            constexpr int padW = 40;
            idealWidth = juce::jlimit(minW, maxW, textW + padW);
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool,
                          int, int, int, int, juce::ComboBox&) override
        {
            // Background is drawn by the backdrop component.
            juce::ignoreUnused(g, width, height);
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            label.setBounds(box.getLocalBounds().reduced(6, 2).translated(-5, 0));
            label.setFont(getComboBoxFont(box));
            label.setJustificationType(juce::Justification::centredLeft);
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            juce::ignoreUnused(width, height);
            g.fillAll(juce::Colour(0xFFCEE5E8));
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                               bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColourToUse) override
        {
            juce::ignoreUnused(isActive, hasSubMenu, shortcutKeyText, icon, textColourToUse);

            const auto normalFg = juce::Colour(0xFF021616);
            const auto selectedBg = juce::Colour(0xFF021616);
            const auto selectedFg = juce::Colour(0xFFCEE5E8);

            if (isSeparator)
            {
                g.setColour(normalFg.withAlpha(0.25f));
                g.fillRect(area.withHeight(1).withY(area.getCentreY()));
                return;
            }

            auto r = area.reduced(6, 2);

            if (isTicked)
            {
                // Make the selected row slightly bigger than highlight.
                auto selectedR = area.reduced(4, 1);
                g.setColour(selectedBg);
                g.fillRoundedRectangle(selectedR.toFloat(), 6.0f);
            }
            else if (isHighlighted)
            {
                g.setColour(normalFg.withAlpha(0.10f));
                g.fillRoundedRectangle(r.toFloat(), 5.0f);
            }

            g.setColour(isTicked ? selectedFg : normalFg);
            g.setFont(getPopupMenuFont());

            g.drawFittedText(text, r, juce::Justification::centred, 1);
        }
    };

    PitchNoteBackdrop pitchNoteBackdrop;
    juce::ComboBox pitchNoteCombo;
    PitchNoteComboLookAndFeel pitchNoteComboLookAndFeel;
    RotaryDial decayControl { "DECAY", RotaryDial::LabelPlacement::Below };
    RotaryDial colorControl { "COLOR", RotaryDial::LabelPlacement::Below };
    RotaryDial shapeControl { "SHAPE", RotaryDial::LabelPlacement::Below };

    struct ValueLabelComboLookAndFeel final : public juce::LookAndFeel_V4
    {
        juce::Font getComboBoxFont(juce::ComboBox& box) override
        {
            juce::ignoreUnused(box);
            // Dial labels are all-caps and read visually smaller than digits/mixed-case.
            // Use a slightly smaller size here so values like "MAJOR" and numbers match
            // the perceived height of the COLOR/SHAPE dial labels.
            return StudioStyle::Fonts::condensedBold(StudioStyle::Fonts::SizePx::dialLabel - 2.0f);
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool,
                          int, int, int, int, juce::ComboBox& box) override
        {
            // Text-only: the dial owns the background; this should read like a dial label.
            // Draw our own text so we can match the dial-label styling exactly.
            auto r = juce::Rectangle<int>(0, 0, width, height);

            g.setColour(box.findColour(juce::ComboBox::textColourId));
            g.setFont(getComboBoxFont(box));

            const auto displayText = box.getText();
            g.drawFittedText(displayText, r, juce::Justification::centred, 1);
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            // We're drawing the text in drawComboBox(); hide the internal label.
            juce::ignoreUnused(box);
            label.setVisible(false);
        }

        juce::Font getPopupMenuFont() override
        {
            return StudioStyle::Fonts::comboMenuFont();
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            juce::ignoreUnused(width, height);
            g.fillAll(menuBg);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                               bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColourToUse) override
        {
            juce::ignoreUnused(isActive, hasSubMenu, shortcutKeyText, icon, textColourToUse);

            if (isSeparator)
            {
                g.setColour(textFg.withAlpha(0.25f));
                g.fillRect(area.withHeight(1).withY(area.getCentreY()));
                return;
            }

            auto textBounds = area.reduced(6, 2);

            if (isTicked)
            {
                // Inverted selected row (no tick prefix). Slightly bigger than the text bounds.
                auto selectedBounds = area.reduced(4, 1);
                g.setColour(textFg);
                g.fillRoundedRectangle(selectedBounds.toFloat(), 6.0f);
                g.setColour(menuBg);
            }
            else
            {
                if (isHighlighted)
                {
                    g.setColour(textFg.withAlpha(0.12f));
                    g.fillRoundedRectangle(textBounds.toFloat(), 5.0f);
                }
                g.setColour(textFg);
            }

            g.setFont(getPopupMenuFont());
            g.drawFittedText(text, textBounds, juce::Justification::centred, 1);
        }

        juce::Colour menuBg { juce::Colour(0xFFCEE5E8) };
        juce::Colour textFg { juce::Colour(0xFF021616) };
    };

    ValueLabelComboLookAndFeel valueLabelComboLookAndFeel;
    juce::ComboBox toneColorValueCombo;
    juce::ComboBox chordShapeValueCombo;
    bool updatingValueLabelCombos { false };
    RotaryDial trackVolumeControl { "VOLUME", RotaryDial::LabelPlacement::Below };
    OverlayMiniToggle mainVolumeOverlayToggle;
    RotaryDial trackPanControl { "PAN", RotaryDial::LabelPlacement::Below };

    // Global MAIN VOLUME (MIX-only)
    RotaryDial mainVolumeControl { "MAIN VOLUME", RotaryDial::LabelPlacement::Below };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    ImageToggle gateToggle;
    juce::Label gateLabel;
    ImageToggle punchToggle;
    juce::Label punchLabel;
    LayeredMenuButton lfoModeButton;
    juce::Label lfoShapeLabel;

    struct SvgDecor final : public juce::Component
    {
        void setSvgFromMemory(const void* data, int size)
        {
            drawable = juce::Drawable::createFromImageData(data, (size_t) size);
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            if (drawable != nullptr)
                drawable->drawWithin(g, getLocalBounds().toFloat(), juce::RectanglePlacement::centred, 1.0f);
        }

        std::unique_ptr<juce::Drawable> drawable;
    };

    SvgDecor scalerCorner;

    std::array<TrackSelectorButton, 7> trackButtons;

    // MIX mode: footer track buttons become MUTE/UNMUTE toggles (bound to t{N}_unmuted).
    // Attachments are created only while MIX mode is active (track mode uses these buttons for selection).
    std::array<std::unique_ptr<ButtonAttachment>, 6> trackUnmutedAttachments;

    struct MixerOverlay final : public juce::Component
    {
        void paint(juce::Graphics& g) override;
    };

    MixerOverlay mixerOverlay;

    struct MixTrackDials
    {
        RotaryDial volume { "VOLUME", RotaryDial::LabelPlacement::Below };
        RotaryDial pan { "PAN", RotaryDial::LabelPlacement::Below };
        RotaryDial delaySend { "DELAY", RotaryDial::LabelPlacement::Below };
        RotaryDial reverbSend { "REVERB", RotaryDial::LabelPlacement::Below };
    };

    std::array<MixTrackDials, 6> mixTrackDials;

    std::array<std::unique_ptr<SliderAttachment>, 6> mixVolumeAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 6> mixPanAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 6> mixDelaySendAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 6> mixReverbSendAttachments;

    // MIX column mini dials (aligned above the MIX button; same size as LFO overlay dials)
    OverlayDial mixDelayFeedbackMini { "FEEDB." };
    OverlayDial mixDelayTimeMini { "TIME" };
    TextMiniToggle mixDelayTimeSyncToggle; // "S" toggle layered above the DELAY mini pair
    OverlayDial mixReverbToneMini { "TONE" };
    OverlayDial mixReverbSizeMini { "SIZE" };

    std::unique_ptr<SliderAttachment> mixDelayFeedbackAttachment;
    std::unique_ptr<SliderAttachment> mixDelayTimeAttachment;
    std::unique_ptr<SliderAttachment> mixReverbToneAttachment;
    std::unique_ptr<SliderAttachment> mixReverbSizeAttachment;

    std::unique_ptr<ButtonAttachment> mixDelayTimeSyncEnabledAttachment;
    bool mixDelayTimeUsesSyncIndex { false };

    struct TrackComboLookAndFeel final : public juce::LookAndFeel_V4
    {
        juce::Font getComboBoxFont(juce::ComboBox& box) override
        {
            juce::ignoreUnused(box);
            return StudioStyle::Fonts::alphaComboTextSmallFont();
        }

        void drawComboBox(juce::Graphics& g, int width, int height, bool,
                          int, int, int, int, juce::ComboBox& box) override
        {
            // Text-only combo (no background). Keeps the UI clean above the track buttons.
            juce::ignoreUnused(g, width, height, box);
        }

        void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
        {
            label.setBounds(box.getLocalBounds().translated(0, 1));
            label.setFont(getComboBoxFont(box));
            label.setJustificationType(juce::Justification::centred);
        }

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
        {
            juce::ignoreUnused(width, height);
            g.fillAll(menuBg);
        }

        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                               bool hasSubMenu, const juce::String& text,
                               const juce::String& shortcutKeyText, const juce::Drawable* icon,
                               const juce::Colour* textColourToUse) override
        {
            juce::ignoreUnused(isActive, hasSubMenu, shortcutKeyText, icon, textColourToUse);

            if (isSeparator)
            {
                g.setColour(textFg.withAlpha(0.25f));
                g.fillRect(area.withHeight(1).withY(area.getCentreY()));
                return;
            }

            auto textBounds = area.reduced(4, 1);

            if (isTicked)
            {
                // Inverted selected row (no tick prefix). Slightly taller than the text bounds.
                auto selectedBounds = area.reduced(4, 0);
                g.setColour(textFg);
                g.fillRoundedRectangle(selectedBounds.toFloat(), 5.0f);

                g.setColour(menuBg);
            }
            else
            {
                if (isHighlighted)
                {
                    g.setColour(textFg.withAlpha(0.12f));
                    g.fillRoundedRectangle(textBounds.toFloat(), 5.0f);
                }

                g.setColour(textFg);
            }

            g.setFont(StudioStyle::Fonts::alphaComboMenuFont());
            g.drawFittedText(text, textBounds, juce::Justification::centred, 1);
        }

        juce::Colour menuBg { juce::Colour(0xFFCEE5E8) };
        juce::Colour textFg { juce::Colour(0xFF021616) };
    };

    TrackComboLookAndFeel trackComboLookAndFeel;

    struct MachineSelectArrow final : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            g.setColour(juce::Colour(0xFF021616));

            auto r = getLocalBounds().toFloat();
            const float w = r.getWidth();
            const float h = r.getHeight();

            juce::Path p;
            p.startNewSubPath(0.0f, 0.0f);
            p.lineTo(0.0f, h);
            p.lineTo(w, h * 0.5f);
            p.closeSubPath();
            g.fillPath(p);
        }
    };

    std::array<MachineSelectArrow, 6> trackMachineArrows;
    std::array<juce::ComboBox, 6> trackMachineCombos;
    std::array<std::unique_ptr<ComboBoxAttachment>, 6> trackMachineAttachments;

    // Row 2
    RotaryDial sweepControl { "SWEEP", RotaryDial::LabelPlacement::Below };
    RotaryDial contourControl { "CONTOUR", RotaryDial::LabelPlacement::Below };
    RotaryDial delSendControl { "DELAY SEND", RotaryDial::LabelPlacement::Below };
    OverlayMiniToggle delSendOverlayToggle;
    RotaryDial revSendControl { "REVERB SEND", RotaryDial::LabelPlacement::Below };
    OverlayMiniToggle revSendOverlayToggle;
    RotaryDial revSizeControl { "REVERB SIZE", RotaryDial::LabelPlacement::Below };
    RotaryDial delayFeedbackOverlayControl { "DELAY FEEDB", RotaryDial::LabelPlacement::Below };

    // Row 3
    RotaryDial lfoSpeedControl { "LFO SPEED", RotaryDial::LabelPlacement::Below };
    OverlayMiniToggle lfoSpeedOverlayToggle;
    LfoOverlayPanel lfoOverlayPanel;
    RotaryDial volDistControl { "VOLUME+DIST", RotaryDial::LabelPlacement::Below };
    RotaryDial swingControl { "SWING", RotaryDial::LabelPlacement::Below };
    RotaryDial chanceControl { "CHANCE", RotaryDial::LabelPlacement::Below };
    RotaryDial delTimeControl { "DELAY TIME", RotaryDial::LabelPlacement::Below };
    TextMiniToggle delTimeSyncToggle;
    RotaryDial delTimeSyncControl { "DELAY TIME", RotaryDial::LabelPlacement::Below };
    RotaryDial reverbToneOverlayControl { "REVERB TONE", RotaryDial::LabelPlacement::Below };

    std::unique_ptr<SliderAttachment> delayFeedbackOverlayAttachment;
    std::unique_ptr<SliderAttachment> reverbToneOverlayAttachment;

    std::unique_ptr<SliderAttachment> mainVolumeAttachment;
    std::unique_ptr<SliderAttachment> reverbSizeAttachment;
    std::unique_ptr<SliderAttachment> delayTimeFreeAttachment;
    std::unique_ptr<SliderAttachment> delayTimeSyncIndexAttachment;

    std::unique_ptr<ComboBoxAttachment> patternBankAttachment;
    std::unique_ptr<ComboBoxAttachment> patternIndexAttachment;

    std::unique_ptr<ButtonAttachment> delayOverlayEnabledAttachment;
    std::unique_ptr<ButtonAttachment> reverbOverlayEnabledAttachment;
    std::unique_ptr<ButtonAttachment> panningOverlayEnabledAttachment;
    std::unique_ptr<ButtonAttachment> delayTimeSyncEnabledAttachment;

    // Track-dependent attachments (re-created on track switch)
    int activeTrackIndex { 0 }; // 0..5
    bool mixerMode { false };
    std::unique_ptr<ButtonAttachment> punchAttachment;
    std::unique_ptr<ButtonAttachment> gateAttachment;
    std::unique_ptr<ComboBoxAttachment> lfoModeAttachment;

    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<ComboBoxAttachment> pitchNoteAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> colorAttachment;
    std::unique_ptr<SliderAttachment> shapeAttachment;
    std::unique_ptr<SliderAttachment> sweepAttachment;
    std::unique_ptr<SliderAttachment> contourAttachment;
    std::unique_ptr<SliderAttachment> delSendAttachment;
    std::unique_ptr<SliderAttachment> revSendAttachment;
    std::unique_ptr<SliderAttachment> lfoSpeedAttachment;
    std::unique_ptr<SliderAttachment> lfoMultiplyAttachment;
    std::unique_ptr<ComboBoxAttachment> lfoWaveformAttachment;
    std::unique_ptr<SliderAttachment> lfoPhaseAttachment;
    std::unique_ptr<SliderAttachment> lfoDepthAttachment;
    std::unique_ptr<ComboBoxAttachment> lfoDestinationAttachment;
    std::unique_ptr<SliderAttachment> lfoFadeAttachment;
    std::unique_ptr<SliderAttachment> volDistAttachment;
    std::unique_ptr<SliderAttachment> swingAttachment;
    std::unique_ptr<SliderAttachment> chanceAttachment;

    // Track-mode VOLUME/PAN in row 1 (bound to the same params as MIX page)
    std::unique_ptr<SliderAttachment> trackMixVolumeAttachment;
    std::unique_ptr<SliderAttachment> trackMixPanAttachment;

    void updateDelayReverbSwapVisibility();
    void updateMainVolumeSwapVisibility();

    void updateDelayTimeSyncVisibility();
    void updateMixDelayTimeSyncBinding();
    void setMixerMode(bool shouldShowMixer);
    void updateMixerOverlayVisibility();
    void updateMachineDependentValueLabels();
    void syncToneColorValueComboToDial();
    void syncChordShapeValueComboToDial();
    void setActiveTrack(int newTrackIndex);
    void rebuildTrackAttachments();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
