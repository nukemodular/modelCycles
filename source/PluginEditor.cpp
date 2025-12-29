#include "PluginEditor.h"
#include "PluginProcessor.h"

#include "BinaryData.h"

#include <cmath>

#include "ui_components/StudioStyle.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), pluginProcessor (p)
{
    constexpr float dialScale = StudioStyle::Sizes::dialScale;

    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(uiRoot);

    setResizable(true, true);
    setResizeLimits(juce::roundToInt((float) baseEditorWidthPx * 0.33f),
                    juce::roundToInt((float) baseEditorHeightPx * 0.33f),
                    juce::roundToInt((float) baseEditorWidthPx * 2.0f),
                    juce::roundToInt((float) baseEditorHeightPx * 2.0f));
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio((double) baseEditorWidthPx / (double) baseEditorHeightPx);

    uiRoot.addAndMakeVisible(patternSelectBackdrop);
    uiRoot.addAndMakeVisible(patternBankCombo);
    uiRoot.addAndMakeVisible(patternIndexCombo);
    uiRoot.addAndMakeVisible(patternCaptionLabel);

    uiRoot.addAndMakeVisible(toneColorValueCombo);
    uiRoot.addAndMakeVisible(chordShapeValueCombo);

    // The MIX overlay uses an always-on-top component; keep the pattern selector above it.
    patternSelectBackdrop.setAlwaysOnTop(true);
    patternBankCombo.setAlwaysOnTop(true);
    patternIndexCombo.setAlwaysOnTop(true);
    patternCaptionLabel.setAlwaysOnTop(true);

    patternCaptionLabel.setText("PATTERN", juce::dontSendNotification);
    patternCaptionLabel.setJustificationType(juce::Justification::centred);
    patternCaptionLabel.setInterceptsMouseClicks(false, false);
    patternCaptionLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF021616));
    patternCaptionLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    // These should behave like dial labels, but must remain above overlays.
    toneColorValueCombo.setAlwaysOnTop(true);
    chordShapeValueCombo.setAlwaysOnTop(true);

    uiRoot.addAndMakeVisible(pitchNoteBackdrop);
    uiRoot.addAndMakeVisible(pitchNoteCombo);
    uiRoot.addAndMakeVisible(pitchControl);
    uiRoot.addAndMakeVisible(decayControl);
    uiRoot.addAndMakeVisible(colorControl);
    uiRoot.addAndMakeVisible(shapeControl);
    uiRoot.addAndMakeVisible(trackVolumeControl);
    uiRoot.addAndMakeVisible(mainVolumeOverlayToggle);
    uiRoot.addAndMakeVisible(trackPanControl);
    uiRoot.addAndMakeVisible(mainVolumeControl);

    uiRoot.addAndMakeVisible(gateToggle);
    uiRoot.addAndMakeVisible(punchToggle);
    uiRoot.addAndMakeVisible(lfoModeButton);

    // Button labels (match dial label styling)
    for (auto* l : { &punchLabel, &gateLabel, &lfoShapeLabel })
    {
        uiRoot.addAndMakeVisible(*l);
        l->setJustificationType(juce::Justification::centred);
        l->setInterceptsMouseClicks(false, false);
        l->setColour(juce::Label::textColourId, juce::Colours::black);
        l->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        l->setMinimumHorizontalScale(0.7f);
        l->setFont(StudioStyle::Fonts::dialLabelFont());
    }
    punchLabel.setText("PUNCH", juce::dontSendNotification);
    gateLabel.setText("GATE", juce::dontSendNotification);
    lfoShapeLabel.setText("LFO SHAPE", juce::dontSendNotification);

    uiRoot.addAndMakeVisible(scalerCorner);
    scalerCorner.setInterceptsMouseClicks(false, false);
    scalerCorner.setSvgFromMemory(BinaryData::Scaler_svg, BinaryData::Scaler_svgSize);

    uiRoot.addAndMakeVisible(mixerOverlay);
    mixerOverlay.setAlwaysOnTop(true);
    mixerOverlay.setVisible(false);
    mixerOverlay.setInterceptsMouseClicks(true, true);

    for (auto* d : { &mixDelayFeedbackMini, &mixDelayTimeMini, &mixReverbToneMini, &mixReverbSizeMini })
    {
        mixerOverlay.addAndMakeVisible(*d);
        d->setLabelYOffsetPx(StudioStyle::Sizes::overlayDialLabelYOffsetPx + 2);
    }

    mixerOverlay.addAndMakeVisible(mixDelayTimeSyncToggle);

    uiRoot.addAndMakeVisible(sweepControl);
    uiRoot.addAndMakeVisible(contourControl);
    uiRoot.addAndMakeVisible(delSendControl);
    uiRoot.addAndMakeVisible(delSendOverlayToggle);
    uiRoot.addAndMakeVisible(revSendControl);
    uiRoot.addAndMakeVisible(revSendOverlayToggle);
    uiRoot.addAndMakeVisible(revSizeControl);
    uiRoot.addAndMakeVisible(delayFeedbackOverlayControl);

    uiRoot.addAndMakeVisible(lfoSpeedControl);
    uiRoot.addAndMakeVisible(lfoSpeedOverlayToggle);
    uiRoot.addAndMakeVisible(lfoOverlayPanel);
    uiRoot.addAndMakeVisible(volDistControl);
    uiRoot.addAndMakeVisible(swingControl);
    uiRoot.addAndMakeVisible(chanceControl);
    uiRoot.addAndMakeVisible(delTimeControl);
    uiRoot.addAndMakeVisible(delTimeSyncToggle);
    uiRoot.addAndMakeVisible(delTimeSyncControl);
    uiRoot.addAndMakeVisible(reverbToneOverlayControl);

    for (auto& b : trackButtons)
        uiRoot.addAndMakeVisible(b);

    for (auto& a : trackMachineArrows)
    {
        uiRoot.addAndMakeVisible(a);
        a.setInterceptsMouseClicks(false, false);
        a.setAlwaysOnTop(true);
    }

    for (auto& c : trackMachineCombos)
        uiRoot.addAndMakeVisible(c);

    // MIX overlay dials (6 tracks x 4 dials)
    for (auto& s : mixTrackDials)
    {
        for (auto* d : { &s.volume, &s.pan, &s.delaySend, &s.reverbSend })
            mixerOverlay.addAndMakeVisible(*d);
    }

    auto initGreyDial = [&](RotaryDial& d)
    {
        d.setUIScale(dialScale);
        // Keep dial labels consistent across TRACK + MIX: raise by an extra 2px (after scaling).
        // RotaryDial applies: labelRaisePx = round(labelRaiseBasePx * uiScale)
        // so to add 2px on-screen we add (2 / uiScale) to the base.
        d.setLabelRaisePx(StudioStyle::Sizes::dialLabelRaiseBasePx
                          + (StudioStyle::Sizes::dialLabelRaiseExtraPx / juce::jmax(0.01f, d.getUIScale())));
        d.setDialMode(RotaryDial::DialMode::UnipolarRing);
        d.setRangeDisplay(RotaryDial::RangeDisplay::None);
        d.getSlider().setRange(0.0, 127.0, 1.0);
        d.getSlider().setNumDecimalPlacesToDisplay(0);
        d.getSlider().setDoubleClickReturnValue(true, 0.0);
        d.setRingThickness(StudioStyle::Sizes::dialRingThicknessPx * dialScale);
        d.setValueFontHeight(StudioStyle::Sizes::dialValueFontHeightPx * dialScale);
        d.setDialImageFromMemory(BinaryData::KnobGrey_png, BinaryData::KnobGrey_pngSize);
        d.setLabelColour(juce::Colours::black);
    };

    auto initWhiteDial = [&](RotaryDial& d)
    {
        initGreyDial(d);
        d.setDialImageFromMemory(BinaryData::KnobWhite_png, BinaryData::KnobWhite_pngSize);

        // For white knobs (used by DELAY/REVERB), render the center value in the same grey as the arc.
        d.getSlider().setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFF727676));
    };

    auto initBlueDial = [&](RotaryDial& d)
    {
        initGreyDial(d);
        d.setDialImageFromMemory(BinaryData::KnobBlue_png, BinaryData::KnobBlue_pngSize);
    };

    // PITCH: bipolar semitone control (per-track).
    pitchControl.setUIScale(dialScale);
    pitchControl.setDialMode(RotaryDial::DialMode::BipolarRing);
    pitchControl.setRangeDisplay(RotaryDial::RangeDisplay::None);
    pitchControl.getSlider().setRange(-24.0, 24.0, 1.0);
    pitchControl.getSlider().setNumDecimalPlacesToDisplay(0);
    pitchControl.getSlider().setDoubleClickReturnValue(true, 0.0);
    pitchControl.setRingThickness(StudioStyle::Sizes::dialRingThicknessPx * dialScale);
    pitchControl.setValueFontHeight(StudioStyle::Sizes::dialValueFontHeightPx * dialScale);
    pitchControl.setDialImageFromMemory(BinaryData::KnobGrey_png, BinaryData::KnobGrey_pngSize);
    pitchControl.setLabelColour(juce::Colours::black);
    pitchControl.setLabelRaisePx(StudioStyle::Sizes::dialLabelRaiseBasePx
                                 + (StudioStyle::Sizes::dialLabelRaiseExtraPx / juce::jmax(0.01f, pitchControl.getUIScale())));
    // Allow the underlying pitch-note selector to receive clicks where it sits under the dial arc.
    // The slider itself still handles interaction via its own hitTest.
    pitchControl.setInterceptsMouseClicks(false, true);

    // Pitch note selector (per-track): C0..G10.
    pitchNoteCombo.setLookAndFeel(&pitchNoteComboLookAndFeel);
    pitchNoteCombo.setEditableText(false);
    pitchNoteCombo.setJustificationType(juce::Justification::centredLeft);

    // Match the grey arc colour used by the dials.
    const auto greyArc = juce::Colour(0xFF727676);
    pitchNoteCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    pitchNoteCombo.setColour(juce::ComboBox::textColourId, greyArc);
    pitchNoteCombo.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    pitchNoteCombo.setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
    pitchNoteCombo.setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);

    pitchNoteCombo.clear(juce::dontSendNotification);
    {
        const juce::StringArray names { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int itemId = 1;
        for (int octave = 0; octave <= 10; ++octave)
        {
            const int lastIdx = (octave == 10 ? 7 : 11); // C..G for octave 10
            for (int i = 0; i <= lastIdx; ++i)
            {
                const auto n = names[i];
                const auto text = n.containsChar('#') ? (n + juce::String(octave)) : (n + " " + juce::String(octave));
                pitchNoteCombo.addItem(text, itemId++);
            }
        }
    }
    pitchNoteCombo.setSelectedId(1, juce::dontSendNotification);

    initGreyDial(decayControl);
    initGreyDial(colorControl);
    initGreyDial(shapeControl);
    initBlueDial(trackVolumeControl);
    trackVolumeControl.getSlider().setDoubleClickReturnValue(true, 100.0);

    initBlueDial(trackPanControl);
    trackPanControl.setAlwaysOnTop(true);
    trackPanControl.setVisible(false);
    trackPanControl.setDialMode(RotaryDial::DialMode::BipolarRing);
    trackPanControl.getSlider().setRange(-64.0, 63.0, 1.0);
    trackPanControl.getSlider().setNumDecimalPlacesToDisplay(0);
    trackPanControl.getSlider().setDoubleClickReturnValue(true, 0.0);

    // Global MAIN VOLUME (MIX-only)
    initBlueDial(mainVolumeControl);
    mainVolumeControl.setAlwaysOnTop(true);
    mainVolumeControl.setVisible(false);
    mainVolumeControl.getSlider().setDoubleClickReturnValue(true, 100.0);

    mainVolumeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                              "mainVolumeGlobal",
                                                              mainVolumeControl.getSlider());

    // Global pattern selection (BANK A-F + PATTERN 01-16)
    {
        const auto comboFg = juce::Colour(0xFF021616);
        const auto transparent = juce::Colours::transparentBlack;

        for (auto* c : { &patternBankCombo, &patternIndexCombo })
        {
            c->setLookAndFeel(&patternComboLookAndFeel);
            c->setEditableText(false);
            c->setJustificationType(juce::Justification::centred);
            c->setColour(juce::ComboBox::backgroundColourId, transparent);
            c->setColour(juce::ComboBox::textColourId, comboFg);
            c->setColour(juce::ComboBox::arrowColourId, transparent);
            c->setColour(juce::ComboBox::outlineColourId, transparent);
            c->setColour(juce::ComboBox::buttonColourId, transparent);
        }

        patternBankCombo.clear(juce::dontSendNotification);
        patternBankCombo.addItem("A", 1);
        patternBankCombo.addItem("B", 2);
        patternBankCombo.addItem("C", 3);
        patternBankCombo.addItem("D", 4);
        patternBankCombo.addItem("E", 5);
        patternBankCombo.addItem("F", 6);
        patternBankCombo.setSelectedId(1, juce::dontSendNotification);

        patternIndexCombo.clear(juce::dontSendNotification);
        for (int i = 1; i <= 16; ++i)
            patternIndexCombo.addItem(juce::String(i).paddedLeft('0', 2), i);
        patternIndexCombo.setSelectedId(1, juce::dontSendNotification);
    }

    patternBankAttachment = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts,
                                                                 "patternBankGlobal",
                                                                 patternBankCombo);
    patternIndexAttachment = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts,
                                                                  "patternIndexGlobal",
                                                                  patternIndexCombo);

    gateToggle.setClickingTogglesState(true);
    gateToggle.setImagesFromMemory(BinaryData::GATE_OFF_svg, BinaryData::GATE_OFF_svgSize,
                                   BinaryData::GATE_ON_svg, BinaryData::GATE_ON_svgSize);
    gateToggle.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    gateToggle.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);

    punchToggle.setClickingTogglesState(true);
    punchToggle.setImagesFromMemory(BinaryData::PUNCH_OFF_svg, BinaryData::PUNCH_OFF_svgSize,
                                    BinaryData::PUNCH_ON_svg, BinaryData::PUNCH_ON_svgSize);
    punchToggle.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    punchToggle.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);

    lfoModeButton.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    lfoModeButton.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);
    lfoModeButton.setImagesFromMemory(BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);
    lfoModeButton.clearItems();
    lfoModeButton.addItem(1, "FREE", BinaryData::LFO_FREE_svg, BinaryData::LFO_FREE_svgSize);
    lfoModeButton.addItem(2, "TRG",  BinaryData::LFO_TRG_svg,  BinaryData::LFO_TRG_svgSize);
    lfoModeButton.addItem(3, "HOLD", BinaryData::LFO_HLD_svg,  BinaryData::LFO_HLD_svgSize);
    lfoModeButton.addItem(4, "ONE",  BinaryData::LFO_ONE_svg,  BinaryData::LFO_ONE_svgSize);
    lfoModeButton.addItem(5, "HALF", BinaryData::LFO_HLF_svg,  BinaryData::LFO_HLF_svgSize);

    // Track selector buttons (footer)
    for (size_t i = 0; i < trackButtons.size(); ++i)
    {
        auto& t = trackButtons[i];
        t.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);

        t.setImagesFromMemory(BinaryData::BUTTON_OFF_svg, BinaryData::BUTTON_OFF_svgSize,
                              BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);
        t.setEnabled(true);
        t.setInterceptsMouseClicks(true, true);
        t.setVisible(true);

        if (i < 6)
        {
            // Tracks 1-6
            t.setClickingTogglesState(true);
            t.setRadioGroupId(trackRadioGroupId);
            t.setButtonText("T" + juce::String((int) i + 1));
        }
        else
        {
            // MIX (separate toggle, not in the track radio group)
            t.setClickingTogglesState(true);
            t.setRadioGroupId(0);
            t.setButtonText("MIX");
        }
    }
    trackButtons[0].setToggleState(true, juce::dontSendNotification);

    // Listen for per-track mute changes so we can update the button outline colour in TRACK mode.
    for (int i = 0; i < 6; ++i)
    {
        const auto id = juce::String("t") + juce::String(i + 1) + "_unmuted";
        pluginProcessor.apvts.addParameterListener(id, this);
    }

    // Initialise mute outlines.
    parameterChanged("t1_unmuted", 0.0f);

    // Track machine combo boxes (above track buttons)
    {
        const auto fg = juce::Colour(0xFF021616);

        for (size_t i = 0; i < trackMachineCombos.size(); ++i)
        {
            auto& c = trackMachineCombos[i];
            c.setLookAndFeel(&trackComboLookAndFeel);
            c.setEditableText(false);
            c.setJustificationType(juce::Justification::centred);
            c.setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
            c.setColour(juce::ComboBox::textColourId, fg);
            c.setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
            c.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
            c.setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);

            c.clear(juce::dontSendNotification);
            c.addItem("KICK", 1);
            c.addItem("SNARE", 2);
            c.addItem("METAL", 3);
            c.addItem("PERC", 4);
            c.addItem("TONE", 5);
            c.addItem("CHORD", 6);

            const auto paramId = juce::String("t") + juce::String((int) i + 1) + "_machine";
            trackMachineAttachments[i] = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts, paramId, c);

            c.onChange = [this, i]
            {
                if ((int) i == activeTrackIndex)
                    updateMachineDependentValueLabels();
            };
        }
    }

    // Machine-dependent value label combos (draw like labels, but open menus to set the dial value).
    {
        const auto fg = juce::Colour(0xFF021616);

        for (auto* c : { &toneColorValueCombo, &chordShapeValueCombo })
        {
            c->setLookAndFeel(&valueLabelComboLookAndFeel);
            c->setEditableText(false);
            c->setJustificationType(juce::Justification::centred);
            c->setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
            c->setColour(juce::ComboBox::textColourId, fg);
            c->setColour(juce::ComboBox::arrowColourId, juce::Colours::transparentBlack);
            c->setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
            c->setColour(juce::ComboBox::buttonColourId, juce::Colours::transparentBlack);
            c->setScrollWheelEnabled(false);
        }

        // TONE -> COLOR
        {
            static const char* const toneVals[] = {
                ".0125", "0.075", "0.125", "0.250", "0.500", "0.750", "0.900", "0.990", "1.000", "1.001", "1.010", "1.100", "1.150", "1.200", "1.250",
                "1.300", "1.400", "1.500", "1.600", "1.750", "1.800", "1.900", "1.980", "1.998", "2.000", "2.002", "2.020", "2.100", "2.150", "2.200",
                "2.250", "2.300", "2.400", "2.500", "2.600", "2.750", "2.800", "2.900", "2.970", "2.997", "3.000", "3.003", "3.030", "3.100", "3.150",
                "3.200", "3.250", "3.300", "3.400", "3.500", "3.600", "3.750", "3.800", "3.900", "3.960", "3.996", "4.000", "4.004", "4.040", "4.100",
                "4.150", "4.200", "4.250", "4.300", "4.400", "4.500", "4.600", "4.750", "4.800", "4.900", "4.950", "4.995", "5.000", "5.005", "5.050",
                "5.100", "5.150", "5.200", "5.250", "5.300", "5.400", "5.500", "5.600", "5.750", "5.800", "5.900", "5.940", "5.994", "6.000", "6.006",
                "6.060", "6.100", "6.150", "6.200", "6.250", "6.300", "6.400", "6.500", "6.600", "6.750", "6.800", "6.900", "6.930", "6.993", "7.000",
                "7.007", "7.070", "7.100", "7.150", "7.200", "7.250", "7.300", "7.400", "7.500", "7.600", "7.750", "7.800", "7.900", "7.920", "7.992",
                "8.000", "9.000", "10.00", "11.00", "12.00", "13.00", "14.00", "15.00"
            };

            toneColorValueCombo.clear(juce::dontSendNotification);
            for (int i = 0; i < (int) (sizeof(toneVals) / sizeof(toneVals[0])); ++i)
                toneColorValueCombo.addItem(toneVals[i], i + 1);
        }

        // CHORD -> SHAPE
        {
            static const char* const chordVals[] = {
                "Unison x 2", "Unison x 3", "Unison x 4", "minor", "Major", "sus2", "sus4", "m7", "M7", "mMaj7", "Maj7", "7sus4", "dim7", "madd9", "Madd9",
                "m6", "M6", "mb5", "Mb5", "m7b5", "M7b5", "M#5", "m7#5", "M7#5", "mb6", "m9no5", "M9no5", "Madd9b5", "Maj7b5", "M7b9no5",
                "sus4#5b9", "sus4add#5", "Maddb5", "M6add4no5", "Maj7/6no5", "Maj9no5", "Fourths", "Fifths"
            };

            chordShapeValueCombo.clear(juce::dontSendNotification);
            for (int i = 0; i < (int) (sizeof(chordVals) / sizeof(chordVals[0])); ++i)
                chordShapeValueCombo.addItem(chordVals[i], i + 1);
        }

        toneColorValueCombo.onChange = [this]
        {
            if (updatingValueLabelCombos)
                return;

            const int n = toneColorValueCombo.getNumItems();
            const int idx = toneColorValueCombo.getSelectedItemIndex();
            if (n <= 1 || idx < 0)
                return;

            const double v = (double) idx / (double) (n - 1) * 127.0;
            colorControl.getSlider().setValue(v, juce::sendNotificationSync);
        };

        chordShapeValueCombo.onChange = [this]
        {
            if (updatingValueLabelCombos)
                return;

            const int n = chordShapeValueCombo.getNumItems();
            const int idx = chordShapeValueCombo.getSelectedItemIndex();
            if (n <= 1 || idx < 0)
                return;

            const double v = (double) idx / (double) (n - 1) * 127.0;
            shapeControl.getSlider().setValue(v, juce::sendNotificationSync);
        };

        toneColorValueCombo.setVisible(false);
        chordShapeValueCombo.setVisible(false);
    }

    // Keep value-label combos synced when the knobs move.
    colorControl.getSlider().onValueChange = [this]
    {
        if (toneColorValueCombo.isVisible())
            syncToneColorValueComboToDial();
    };

    shapeControl.getSlider().onValueChange = [this]
    {
        if (chordShapeValueCombo.isVisible())
            syncChordShapeValueComboToDial();
    };

    updateMachineDependentValueLabels();

    // Shift+click in TRACK mode toggles mute/unmute without switching active track.
    for (int i = 0; i < 6; ++i)
    {
        trackButtons[(size_t) i].setMouseDownInterceptor([this, i](const juce::MouseEvent& e)
        {
            if (mixerMode)
                return false;

            if (! e.mods.isShiftDown())
                return false;

            const auto id = juce::String("t") + juce::String(i + 1) + "_unmuted";
            if (auto* param = pluginProcessor.apvts.getParameter(id))
            {
                const float current = param->getValue();
                const float toggled = current >= 0.5f ? 0.0f : 1.0f;
                param->beginChangeGesture();
                param->setValueNotifyingHost(toggled);
                param->endChangeGesture();
            }

            // Update outline immediately (parameterChanged is async).
            parameterChanged(id, 0.0f);
            return true;
        });
    }

    for (size_t i = 0; i < 6; ++i)
    {
        trackButtons[i].onClick = [this, i]
        {
            // In MIX mode the footer track buttons are MUTE/UNMUTE toggles and must *not* exit MIX.
            if (mixerMode)
                return;

            if (trackButtons[i].getToggleState())
                setActiveTrack((int) i);
        };
    }

    trackButtons[6].onClick = [this]
    {
        setMixerMode(trackButtons[6].getToggleState());
    };

    initGreyDial(sweepControl);
    initGreyDial(contourControl);
    initGreyDial(delSendControl);
    initGreyDial(revSendControl);
    initWhiteDial(revSizeControl);

    reverbSizeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                              "reverbSizeGlobal",
                                                              revSizeControl.getSlider());

    initWhiteDial(delayFeedbackOverlayControl);
    delayFeedbackOverlayControl.setAlwaysOnTop(true);
    delayFeedbackOverlayControl.setVisible(false);
    delayFeedbackOverlayAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                       "delayFeedbackOverlay",
                                                                       delayFeedbackOverlayControl.getSlider());

    initGreyDial(lfoSpeedControl);

    // LFO SPEED is bipolar -64..63.
    lfoSpeedControl.setDialMode(RotaryDial::DialMode::BipolarRing);
    lfoSpeedControl.getSlider().setRange(-64.0, 63.0, 1.0);
    lfoSpeedControl.getSlider().setNumDecimalPlacesToDisplay(0);
    lfoSpeedControl.getSlider().setDoubleClickReturnValue(true, 0.0);

    // Overlay mini toggles (16x16). LFO SPEED toggles the LFO panel; the others switch global overlays.
    auto initOverlayMiniToggle = [](OverlayMiniToggle& t)
    {
        t.setClickingTogglesState(true);
        t.setImagesFromMemory(BinaryData::BUTTON_OFF_svg, BinaryData::BUTTON_OFF_svgSize,
                              BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);
        t.setCornerRadius(4.0f);
        t.setAlwaysOnTop(true);
    };

    initOverlayMiniToggle(lfoSpeedOverlayToggle);
    initOverlayMiniToggle(mainVolumeOverlayToggle);
    initOverlayMiniToggle(delSendOverlayToggle);
    initOverlayMiniToggle(revSendOverlayToggle);

    // Persist global overlay toggle states.
    panningOverlayEnabledAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                                        "panningOverlayEnabled",
                                                                        mainVolumeOverlayToggle);
    delayOverlayEnabledAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                                      "delaySendOverlayEnabled",
                                                                      delSendOverlayToggle);
    reverbOverlayEnabledAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                                       "reverbSendOverlayEnabled",
                                                                       revSendOverlayToggle);

    mainVolumeOverlayToggle.onStateChange = [this]
    {
        updateMainVolumeSwapVisibility();
        resized();
    };

    // DELAY/REVERB swap mini toggles are mutually exclusive (but both can be off).
    delSendOverlayToggle.onClick = [this]
    {
        if (delSendOverlayToggle.getToggleState())
            revSendOverlayToggle.setToggleState(false, juce::sendNotificationSync);
    };

    revSendOverlayToggle.onClick = [this]
    {
        if (revSendOverlayToggle.getToggleState())
            delSendOverlayToggle.setToggleState(false, juce::sendNotificationSync);
    };

    delSendOverlayToggle.onStateChange = [this]
    {
        updateDelayReverbSwapVisibility();
        resized();
    };

    revSendOverlayToggle.onStateChange = [this]
    {
        updateDelayReverbSwapVisibility();
        resized();
    };

    lfoOverlayPanel.setAlwaysOnTop(true);
    lfoOverlayPanel.setVisible(false);
    lfoOverlayPanel.setInterceptsMouseClicks(true, true);
    lfoOverlayPanel.setMiniDialUIScale(dialScale * 0.78f);
    lfoOverlayPanel.setDialImageFromMemory(BinaryData::KnobGrey_png, BinaryData::KnobGrey_pngSize);
    lfoOverlayPanel.setDesiredDialArcSidePx(48);

    // Configure overlay dials per spec.
    {
        // MULTIPLY: unipolar with discrete labels.
        auto& s = lfoOverlayPanel.getMultiplyDial().getSlider();
        lfoOverlayPanel.getMultiplyDial().setDialMode(RotaryDial::DialMode::UnipolarRing);
        s.setRange(0.0, 23.0, 1.0);
        s.setNumDecimalPlacesToDisplay(0);

        const juce::StringArray labels {
            "x1", "x2", "x4", "x8", "x16", "x32", "x64", "x128", "x256", "x512", "x1k", "x2k",
            "1",  "2",  "4",  "8",  "16",  "32",  "64",  "128",  "256",  "512",  "1k",  "2k"
        };

        s.textFromValueFunction = [labels](double v)
        {
            const int idx = juce::jlimit(0, labels.size() - 1, (int) std::lround(v));
            return labels[idx];
        };
        s.valueFromTextFunction = [labels](const juce::String& t)
        {
            const int idx = labels.indexOf(t.trim());
            return (double) (idx >= 0 ? idx : 0);
        };
        s.setDoubleClickReturnValue(true, 0.0);

        // DEPTH + FADE: bipolar -64..63.
        for (auto* d : { &lfoOverlayPanel.getDepthDial(), &lfoOverlayPanel.getFadeDial() })
        {
            d->setDialMode(RotaryDial::DialMode::BipolarRing);
            auto& ds = d->getSlider();
            ds.setRange(-64.0, 63.0, 1.0);
            ds.setNumDecimalPlacesToDisplay(0);
            ds.setDoubleClickReturnValue(true, 0.0);
        }

        // PHASE: keep unipolar 0..127 (not specified otherwise).
        lfoOverlayPanel.getPhaseDial().setDialMode(RotaryDial::DialMode::UnipolarRing);
        lfoOverlayPanel.getPhaseDial().getSlider().setRange(0.0, 127.0, 1.0);
        lfoOverlayPanel.getPhaseDial().getSlider().setNumDecimalPlacesToDisplay(0);
        lfoOverlayPanel.getPhaseDial().getSlider().setDoubleClickReturnValue(true, 0.0);
    }

    lfoSpeedOverlayToggle.onClick = [this]
    {
        const bool on = lfoSpeedOverlayToggle.getToggleState();
        lfoOverlayPanel.setVisible(on);
        if (on)
            lfoOverlayPanel.toFront(false);
        resized();
    };

    initGreyDial(volDistControl);
    initGreyDial(swingControl);
    initGreyDial(chanceControl);
    initWhiteDial(delTimeControl);

    delayTimeFreeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                 "delayTimeFreeGlobal",
                                                                 delTimeControl.getSlider());

    initWhiteDial(delTimeSyncControl);
    delTimeSyncControl.setAlwaysOnTop(true);
    delTimeSyncControl.setVisible(false);
    {
        static constexpr std::array<int, 14> syncedValues {
            1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128
        };

        auto& s = delTimeSyncControl.getSlider();
        s.setRange(0.0, (double) (syncedValues.size() - 1), 1.0);
        s.setNumDecimalPlacesToDisplay(0);
        s.textFromValueFunction = [](double v)
        {
            static constexpr std::array<int, 14> values {
                1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128
            };
            const int i = juce::jlimit(0, (int) values.size() - 1, (int) std::lround(v));
            return juce::String(values[(size_t) i]);
        };
        s.valueFromTextFunction = [](const juce::String& t)
        {
            const auto trimmed = t.trim();
            static constexpr std::array<int, 14> values {
                1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128
            };
            for (int i = 0; i < (int) values.size(); ++i)
                if (trimmed == juce::String(values[(size_t) i]))
                    return (double) i;
            return 0.0;
        };
        s.setDoubleClickReturnValue(true, 7.0);
    }

    delayTimeSyncIndexAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                     "delayTimeSyncIndexGlobal",
                                                                     delTimeSyncControl.getSlider());

    // DELAY TIME sync toggle ("S")
    delTimeSyncToggle.setClickingTogglesState(true);
    delTimeSyncToggle.setCornerRadius(4.0f);
    delTimeSyncToggle.setAlwaysOnTop(true);
    delTimeSyncToggle.setGlyphText("S");
    delTimeSyncToggle.setGlyphColour(juce::Colours::white);
    delTimeSyncToggle.setGlyphFont(StudioStyle::Fonts::condensedBold(12.0f));
    delTimeSyncToggle.setImagesFromMemory(BinaryData::BUTTON_OFF_svg, BinaryData::BUTTON_OFF_svgSize,
                                          BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);

    delayTimeSyncEnabledAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                                       "delayTimeSyncEnabled",
                                                                       delTimeSyncToggle);

    delTimeSyncToggle.onStateChange = [this]
    {
        updateDelayTimeSyncVisibility();
        updateMixDelayTimeSyncBinding();
        resized();
    };

    initWhiteDial(reverbToneOverlayControl);
    reverbToneOverlayControl.setAlwaysOnTop(true);
    reverbToneOverlayControl.setVisible(false);
    reverbToneOverlayAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                     "reverbToneOverlay",
                                                                     reverbToneOverlayControl.getSlider());

    updateDelayReverbSwapVisibility();
    updateMainVolumeSwapVisibility();
    updateDelayTimeSyncVisibility();
    rebuildTrackAttachments();

    // MIX overlay dials: 4 rows per track aligned with the footer track buttons.
    for (int track = 0; track < 6; ++track)
    {
        auto& s = mixTrackDials[(size_t) track];

        initBlueDial(s.volume);
        s.volume.getSlider().setRange(0.0, 127.0, 1.0);
        s.volume.getSlider().setNumDecimalPlacesToDisplay(0);
        s.volume.getSlider().setDoubleClickReturnValue(true, 100.0);

        initBlueDial(s.pan);
        s.pan.setDialMode(RotaryDial::DialMode::BipolarRing);
        s.pan.getSlider().setRange(-64.0, 63.0, 1.0);
        s.pan.getSlider().setNumDecimalPlacesToDisplay(0);
        s.pan.getSlider().setDoubleClickReturnValue(true, 0.0);

        initWhiteDial(s.delaySend);
        s.delaySend.getSlider().setRange(0.0, 127.0, 1.0);
        s.delaySend.getSlider().setNumDecimalPlacesToDisplay(0);
        s.delaySend.getSlider().setDoubleClickReturnValue(true, 0.0);

        initWhiteDial(s.reverbSend);
        s.reverbSend.getSlider().setRange(0.0, 127.0, 1.0);
        s.reverbSend.getSlider().setNumDecimalPlacesToDisplay(0);
        s.reverbSend.getSlider().setDoubleClickReturnValue(true, 0.0);

        const auto tn = juce::String(track + 1);
        mixVolumeAttachments[(size_t) track] = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                                 "t" + tn + "_mixVolume",
                                                                                 s.volume.getSlider());
        mixPanAttachments[(size_t) track] = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                              "t" + tn + "_mixPan",
                                                                              s.pan.getSlider());

        // Reuse the existing per-track delay/reverb send parameters.
        mixDelaySendAttachments[(size_t) track] = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                                    "t" + tn + "_delaySend",
                                                                                    s.delaySend.getSlider());
        mixReverbSendAttachments[(size_t) track] = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                                     "t" + tn + "_reverbSend",
                                                                                     s.reverbSend.getSlider());
    }

    // MIX overlay mini dials (MIX column): same sizing/feel as LFO overlay dials.
    {
        const float miniScale = dialScale * 0.78f;
        constexpr int arcSidePx = 48;

        for (auto* d : { &mixDelayFeedbackMini, &mixDelayTimeMini, &mixReverbToneMini, &mixReverbSizeMini })
        {
            d->setUIScale(miniScale);
            d->setArcSidePx(arcSidePx);
            d->setDialMode(RotaryDial::DialMode::UnipolarRing);
            d->setDialImageFromMemory(BinaryData::KnobWhite_png, BinaryData::KnobWhite_pngSize);
            d->setLabelColour(juce::Colours::black);

            // Match the arc grey for value text on white knobs.
            d->getSlider().setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFF727676));
        }

        mixDelayFeedbackMini.getSlider().setRange(0.0, 127.0, 1.0);
        mixDelayFeedbackMini.getSlider().setNumDecimalPlacesToDisplay(0);
        mixDelayFeedbackMini.getSlider().setDoubleClickReturnValue(true, 0.0);
        mixDelayFeedbackAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                        "delayFeedbackOverlay",
                                                                        mixDelayFeedbackMini.getSlider());

        mixDelayTimeMini.getSlider().setRange(0.0, 127.0, 1.0);
        mixDelayTimeMini.getSlider().setNumDecimalPlacesToDisplay(0);
        mixDelayTimeMini.getSlider().setDoubleClickReturnValue(true, 0.0);
        // Attachment will be swapped between free time and sync index in updateMixDelayTimeSyncBinding().

        mixReverbToneMini.getSlider().setRange(0.0, 127.0, 1.0);
        mixReverbToneMini.getSlider().setNumDecimalPlacesToDisplay(0);
        mixReverbToneMini.getSlider().setDoubleClickReturnValue(true, 0.0);
        mixReverbToneAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                     "reverbToneOverlay",
                                                                     mixReverbToneMini.getSlider());

        mixReverbSizeMini.getSlider().setRange(0.0, 127.0, 1.0);
        mixReverbSizeMini.getSlider().setNumDecimalPlacesToDisplay(0);
        mixReverbSizeMini.getSlider().setDoubleClickReturnValue(true, 64.0);
        mixReverbSizeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                     "reverbSizeGlobal",
                                                                     mixReverbSizeMini.getSlider());
    }

    // MIX delay time sync toggle ("S")
    mixDelayTimeSyncToggle.setClickingTogglesState(true);
    mixDelayTimeSyncToggle.setCornerRadius(4.0f);
    mixDelayTimeSyncToggle.setAlwaysOnTop(true);
    mixDelayTimeSyncToggle.setGlyphText("S");
    mixDelayTimeSyncToggle.setGlyphColour(juce::Colours::white);
    mixDelayTimeSyncToggle.setGlyphFont(StudioStyle::Fonts::condensedBold(12.0f));
    mixDelayTimeSyncToggle.setImagesFromMemory(BinaryData::BUTTON_OFF_svg, BinaryData::BUTTON_OFF_svgSize,
                                               BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);

    // Same parameter as the normal page sync toggle.
    mixDelayTimeSyncEnabledAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                                           "delayTimeSyncEnabled",
                                                                           mixDelayTimeSyncToggle);

    // Ensure the MIX TIME dial is bound correctly for the current sync state.
    updateMixDelayTimeSyncBinding();

    mixDelayTimeSyncToggle.onStateChange = [this]
    {
        updateDelayTimeSyncVisibility();
        updateMixDelayTimeSyncBinding();
        resized();
    };

    updateMixerOverlayVisibility();

    setSize (baseEditorWidthPx, baseEditorHeightPx);
}

PluginEditor::~PluginEditor()
{
    for (auto& c : trackMachineCombos)
        c.setLookAndFeel(nullptr);

    pitchNoteCombo.setLookAndFeel(nullptr);
    patternBankCombo.setLookAndFeel(nullptr);
    patternIndexCombo.setLookAndFeel(nullptr);

    for (int i = 0; i < 6; ++i)
    {
        const auto id = juce::String("t") + juce::String(i + 1) + "_unmuted";
        pluginProcessor.apvts.removeParameterListener(id, this);
    }

    setLookAndFeel(nullptr);
}

void PluginEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused(newValue);

    if (! parameterID.endsWith("_unmuted"))
        return;

    juce::MessageManager::callAsync([this]
    {
        for (int i = 0; i < 6; ++i)
        {
            const auto id = juce::String("t") + juce::String(i + 1) + "_unmuted";
            if (auto* v = pluginProcessor.apvts.getRawParameterValue(id))
            {
                const bool unmuted = v->load() >= 0.5f;
                trackButtons[(size_t) i].setMuted(! unmuted);
            }
        }
    });
}

void PluginEditor::syncToneColorValueComboToDial()
{
    const int n = toneColorValueCombo.getNumItems();
    if (n <= 0)
        return;

    const auto v = juce::jlimit(0.0, 127.0, colorControl.getSlider().getValue());
    const int idx = (int) juce::jlimit(0, n - 1, (int) std::lround((v / 127.0) * (double) (n - 1)));

    const juce::ScopedValueSetter<bool> guard(updatingValueLabelCombos, true);
    toneColorValueCombo.setSelectedItemIndex(idx, juce::dontSendNotification);
}

void PluginEditor::syncChordShapeValueComboToDial()
{
    const int n = chordShapeValueCombo.getNumItems();
    if (n <= 0)
        return;

    const auto v = juce::jlimit(0.0, 127.0, shapeControl.getSlider().getValue());
    const int idx = (int) juce::jlimit(0, n - 1, (int) std::lround((v / 127.0) * (double) (n - 1)));

    const juce::ScopedValueSetter<bool> guard(updatingValueLabelCombos, true);
    chordShapeValueCombo.setSelectedItemIndex(idx, juce::dontSendNotification);
}

void PluginEditor::updateMachineDependentValueLabels()
{
    // Machine combo items are in order: KICK, SNARE, METAL, PERC, TONE, CHORD.
    // The value is 0..5 in the APVTS.
    const int machine = trackMachineCombos[(size_t) activeTrackIndex].getSelectedItemIndex();

    if (machine < 0)
    {
        toneColorValueCombo.setVisible(false);
        chordShapeValueCombo.setVisible(false);

        colorControl.setLabelText("COLOR");
        shapeControl.setLabelText("SHAPE");
        return;
    }

    const bool isTone = (machine == 4);
    const bool isChord = (machine == 5);

    toneColorValueCombo.setVisible(isTone);
    chordShapeValueCombo.setVisible(isChord);

    // Replace the dial label text with the value-combo text when active.
    colorControl.setLabelText(isTone ? juce::String{} : juce::String{"COLOR"});
    shapeControl.setLabelText(isChord ? juce::String{} : juce::String{"SHAPE"});

    if (isTone)
        syncToneColorValueComboToDial();
    if (isChord)
        syncChordShapeValueComboToDial();

    // Ensure they stay above the MIX overlay when it is visible.
    toneColorValueCombo.toFront(false);
    chordShapeValueCombo.toFront(false);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (StudioStyle::Colours::canvas);
}

void PluginEditor::MixerOverlay::paint(juce::Graphics& g)
{
    g.fillAll(StudioStyle::Colours::canvas);
}

void PluginEditor::updateDelayReverbSwapVisibility()
{
    bool delayOn = delSendOverlayToggle.getToggleState();
    bool reverbOn = revSendOverlayToggle.getToggleState();

    // Extra safety: if both end up enabled (e.g. via automation), prefer DELAY.
    if (delayOn && reverbOn)
    {
        revSendOverlayToggle.setToggleState(false, juce::sendNotificationSync);
        reverbOn = false;
    }

    delayFeedbackOverlayControl.setVisible(delayOn);
    revSizeControl.setVisible(! delayOn);

    reverbToneOverlayControl.setVisible(reverbOn);
    updateDelayTimeSyncVisibility();

    if (delayOn)
        delayFeedbackOverlayControl.toFront(false);
    if (reverbOn)
        reverbToneOverlayControl.toFront(false);
}

void PluginEditor::updateMainVolumeSwapVisibility()
{
    const bool on = mainVolumeOverlayToggle.getToggleState();

    // Track mode: swap between TRACK VOLUME and TRACK PAN (same params as MIX page).
    // MIX mode: global MAIN VOLUME is shown separately; this toggle is hidden.
    trackPanControl.setVisible(on);
    trackVolumeControl.setVisible(! on);

    if (on)
        trackPanControl.toFront(false);

    // Keep the mini toggle clickable even when the overlay dial is front-most.
    mainVolumeOverlayToggle.toFront(false);
}

void PluginEditor::updateDelayTimeSyncVisibility()
{
    const bool reverbOn = revSendOverlayToggle.getToggleState();
    const bool syncOn = delTimeSyncToggle.getToggleState();

    // When the REVERB overlay is enabled, DELAY TIME is swapped out entirely.
    const bool allowDelayTime = ! reverbOn;

    delTimeControl.setVisible(allowDelayTime && ! syncOn);
    delTimeSyncControl.setVisible(allowDelayTime && syncOn);
    delTimeSyncToggle.setVisible(allowDelayTime);

    if (allowDelayTime && syncOn)
        delTimeSyncControl.toFront(false);

    // Keep the sync toggle clickable even when the swapped dial is front-most.
    delTimeSyncToggle.toFront(false);
}

void PluginEditor::updateMixDelayTimeSyncBinding()
{
    const bool syncOn = delTimeSyncToggle.getToggleState();

    if (syncOn == mixDelayTimeUsesSyncIndex && mixDelayTimeAttachment)
        return;

    // Rebind the MIX TIME mini dial to the correct parameter.
    mixDelayTimeAttachment.reset();

    auto& s = mixDelayTimeMini.getSlider();

    if (syncOn)
    {
        // Match the main synced TIME dial behaviour: index 0..13 with musical labels.
        static constexpr std::array<int, 14> values { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 };

        s.setRange(0.0, (double) (values.size() - 1), 1.0);
        s.setNumDecimalPlacesToDisplay(0);
        s.textFromValueFunction = [](double v)
        {
            static constexpr std::array<int, 14> vls { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 };
            const int i = juce::jlimit(0, (int) vls.size() - 1, (int) std::lround(v));
            return juce::String(vls[(size_t) i]);
        };
        s.valueFromTextFunction = [](const juce::String& t)
        {
            const auto trimmed = t.trim();
            static constexpr std::array<int, 14> vls { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 };
            for (int i = 0; i < (int) vls.size(); ++i)
                if (trimmed == juce::String(vls[(size_t) i]))
                    return (double) i;
            return 0.0;
        };
        s.setDoubleClickReturnValue(true, 7.0);

        mixDelayTimeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                    "delayTimeSyncIndexGlobal",
                                                                    s);
    }
    else
    {
        // Free time: 0..127.
        s.setRange(0.0, 127.0, 1.0);
        s.setNumDecimalPlacesToDisplay(0);
        s.textFromValueFunction = {};
        s.valueFromTextFunction = {};
        s.setDoubleClickReturnValue(true, 0.0);

        mixDelayTimeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                    "delayTimeFreeGlobal",
                                                                    s);
    }

    mixDelayTimeUsesSyncIndex = syncOn;
}

void PluginEditor::setMixerMode(bool shouldShowMixer)
{
    mixerMode = shouldShowMixer;

    if (mixerMode)
    {
        // Disable radio grouping so T1-T6 become independent toggles.
        for (int i = 0; i < 6; ++i)
            trackButtons[(size_t) i].setRadioGroupId(0);

        // Bind T1-T6 to per-track UNMUTED parameters.
        for (int i = 0; i < 6; ++i)
        {
            const auto id = juce::String("t") + juce::String(i + 1) + "_unmuted";
            trackUnmutedAttachments[(size_t) i] = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                                                     id,
                                                                                     trackButtons[(size_t) i]);
        }

        // Ensure MIX button is lit while MIX is active.
        trackButtons[6].setToggleState(true, juce::dontSendNotification);
    }
    else
    {
        // Drop mute/unmute bindings so track buttons can be used for track selection again.
        for (auto& a : trackUnmutedAttachments)
            a.reset();

        // Restore radio grouping and the selected-track visual.
        for (int i = 0; i < 6; ++i)
            trackButtons[(size_t) i].setRadioGroupId(trackRadioGroupId);

        trackButtons[6].setToggleState(false, juce::dontSendNotification);
        trackButtons[(size_t) activeTrackIndex].setToggleState(true, juce::dontSendNotification);
    }

    // Mixer page UI will be defined later.
    updateMixerOverlayVisibility();
    resized();
}

void PluginEditor::updateMixerOverlayVisibility()
{
    const bool on = mixerMode;

    mixerOverlay.setVisible(on);
    if (on)
        mixerOverlay.toFront(false);

    // Pattern selector is visible in both modes and must stay above the MIX overlay.
    patternSelectBackdrop.setVisible(true);
    patternBankCombo.setVisible(true);
    patternIndexCombo.setVisible(true);
    patternCaptionLabel.setVisible(true);
    if (on)
    {
        patternSelectBackdrop.toFront(false);
        patternBankCombo.toFront(false);
        patternIndexCombo.toFront(false);
        patternCaptionLabel.toFront(false);
    }

    // Machine selectors are hidden in MIX view.
    for (auto& c : trackMachineCombos)
        c.setVisible(! on);

    for (auto& a : trackMachineArrows)
        a.setVisible(! on);

    // Normal page controls are hidden in MIX view.
    const bool showNormal = ! on;
    for (auto* comp : {
             (juce::Component*) &pitchControl,
             (juce::Component*) &pitchNoteBackdrop,
             (juce::Component*) &pitchNoteCombo,
             (juce::Component*) &decayControl,
             (juce::Component*) &colorControl,
             (juce::Component*) &shapeControl,
             (juce::Component*) &trackVolumeControl,
             (juce::Component*) &trackPanControl,
             (juce::Component*) &sweepControl,
             (juce::Component*) &contourControl,
             (juce::Component*) &delSendControl,
             (juce::Component*) &revSendControl,
             (juce::Component*) &revSizeControl,
             (juce::Component*) &delayFeedbackOverlayControl,
             (juce::Component*) &lfoSpeedControl,
             (juce::Component*) &volDistControl,
             (juce::Component*) &swingControl,
             (juce::Component*) &chanceControl,
             (juce::Component*) &delTimeControl,
             (juce::Component*) &delTimeSyncToggle,
             (juce::Component*) &delTimeSyncControl,
             (juce::Component*) &reverbToneOverlayControl,
             (juce::Component*) &gateToggle,
             (juce::Component*) &punchToggle,
             (juce::Component*) &lfoModeButton,
             (juce::Component*) &lfoSpeedOverlayToggle,
             (juce::Component*) &mainVolumeOverlayToggle,
             (juce::Component*) &delSendOverlayToggle,
             (juce::Component*) &revSendOverlayToggle,
             (juce::Component*) &lfoOverlayPanel,
         })
    {
        comp->setVisible(showNormal);
    }

    // Machine-dependent value labels only apply in TRACK mode.
    if (on)
    {
        toneColorValueCombo.setVisible(false);
        chordShapeValueCombo.setVisible(false);
    }
    else
    {
        updateMachineDependentValueLabels();
    }

    // (pattern selector visibility handled above)

    // Global MAIN VOLUME is MIX-only and lives in the row-1 col-5 slot.
    mainVolumeControl.setVisible(on);
    if (on)
        mainVolumeControl.toFront(false);

    if (showNormal)
    {
        updateDelayReverbSwapVisibility();
        updateMainVolumeSwapVisibility();
        updateDelayTimeSyncVisibility();
        lfoOverlayPanel.setVisible(lfoSpeedOverlayToggle.getToggleState());
    }
    else
    {
        lfoOverlayPanel.setVisible(false);
    }
}

void PluginEditor::setActiveTrack(int newTrackIndex)
{
    newTrackIndex = juce::jlimit(0, 5, newTrackIndex);
    const bool wasMixer = mixerMode;
    if (activeTrackIndex == newTrackIndex && ! wasMixer)
        return;

    activeTrackIndex = newTrackIndex;

    // Safety: if something triggers a track change while MIX is active,
    // ensure we drop MIX bindings and return to normal track selection mode.
    if (mixerMode)
    {
        for (auto& a : trackUnmutedAttachments)
            a.reset();
        for (int i = 0; i < 6; ++i)
            trackButtons[(size_t) i].setRadioGroupId(trackRadioGroupId);
    }

    mixerMode = false;
    trackButtons[6].setToggleState(false, juce::dontSendNotification);
    updateMixerOverlayVisibility();

    // Update the track selector UI (buttons 0-5 are T1-T6).
    for (int i = 0; i < 6; ++i)
        trackButtons[(size_t) i].setToggleState(i == activeTrackIndex, juce::dontSendNotification);

    rebuildTrackAttachments();
}

void PluginEditor::rebuildTrackAttachments()
{
    auto trackParamId = [this](const juce::String& suffix)
    {
        return juce::String("t") + juce::String(activeTrackIndex + 1) + "_" + suffix;
    };

    // Destroy old attachments first to ensure we can rebind safely.
    punchAttachment.reset();
    gateAttachment.reset();
        updateMachineDependentValueLabels();
    lfoModeAttachment.reset();

    pitchAttachment.reset();
    pitchNoteAttachment.reset();
    decayAttachment.reset();
    colorAttachment.reset();
    shapeAttachment.reset();
    sweepAttachment.reset();
    contourAttachment.reset();
    delSendAttachment.reset();
    revSendAttachment.reset();
    lfoSpeedAttachment.reset();
    lfoMultiplyAttachment.reset();
    lfoWaveformAttachment.reset();
    lfoPhaseAttachment.reset();
    lfoDepthAttachment.reset();
    lfoDestinationAttachment.reset();
    lfoFadeAttachment.reset();
    volDistAttachment.reset();
    swingAttachment.reset();
    chanceAttachment.reset();

    trackMixVolumeAttachment.reset();
    trackMixPanAttachment.reset();

    punchAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                         trackParamId("punch"),
                                                         punchToggle);

    pitchAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                         trackParamId("pitch"),
                                                         pitchControl.getSlider());

    pitchNoteAttachment = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts,
                                                               trackParamId("pitchNote"),
                                                               pitchNoteCombo);

    decayAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                         trackParamId("decay"),
                                                         decayControl.getSlider());

    colorAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                         trackParamId("color"),
                                                         colorControl.getSlider());

    shapeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                         trackParamId("shape"),
                                                         shapeControl.getSlider());

    gateAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts,
                                                        trackParamId("gate"),
                                                        gateToggle);

    sweepAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                         trackParamId("sweep"),
                                                         sweepControl.getSlider());

    contourAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                           trackParamId("contour"),
                                                           contourControl.getSlider());

    delSendAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                           trackParamId("delaySend"),
                                                           delSendControl.getSlider());

    revSendAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                           trackParamId("reverbSend"),
                                                           revSendControl.getSlider());

    lfoModeAttachment = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts,
                                                             trackParamId("lfoMode"),
                                                             lfoModeButton.getComboBox());

    lfoSpeedAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                            trackParamId("lfoSpeed"),
                                                            lfoSpeedControl.getSlider());

    // LFO overlay panel controls (track-dependent)
    lfoMultiplyAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                               trackParamId("lfoMultiply"),
                                                               lfoOverlayPanel.getMultiplyDial().getSlider());

    lfoWaveformAttachment = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts,
                                                                 trackParamId("lfoWaveform"),
                                                                 lfoOverlayPanel.getWaveformCombo());

    lfoPhaseAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                            trackParamId("lfoPhase"),
                                                            lfoOverlayPanel.getPhaseDial().getSlider());

    lfoDepthAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                            trackParamId("lfoDepth"),
                                                            lfoOverlayPanel.getDepthDial().getSlider());

    lfoDestinationAttachment = std::make_unique<ComboBoxAttachment>(pluginProcessor.apvts,
                                                                    trackParamId("lfoDestination"),
                                                                    lfoOverlayPanel.getDestinationCombo());

    lfoFadeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                           trackParamId("lfoFade"),
                                                           lfoOverlayPanel.getFadeDial().getSlider());

    volDistAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                           trackParamId("volDist"),
                                                           volDistControl.getSlider());

    swingAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                         trackParamId("swing"),
                                                         swingControl.getSlider());

    chanceAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                          trackParamId("chance"),
                                                          chanceControl.getSlider());

    // Track-mode VOLUME/PAN are the same parameters as the MIX page per-track controls.
    trackMixVolumeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                                  trackParamId("mixVolume"),
                                                                  trackVolumeControl.getSlider());

    trackMixPanAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts,
                                                               trackParamId("mixPan"),
                                                               trackPanControl.getSlider());
}

void PluginEditor::resized()
{
    // Layout at a fixed design size, then scale the whole UI uniformly.
    uiRoot.setBounds(0, 0, baseEditorWidthPx, baseEditorHeightPx);
    const float sx = (float) getWidth()  / (float) baseEditorWidthPx;
    const float sy = (float) getHeight() / (float) baseEditorHeightPx;
    const float scale = juce::jmax(0.01f, juce::jmin(sx, sy));
    const float offsetX = ((float) getWidth()  - (float) baseEditorWidthPx  * scale) * 0.5f;
    const float offsetY = ((float) getHeight() - (float) baseEditorHeightPx * scale) * 0.5f;
    uiRoot.setTransform(juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    auto bounds = uiRoot.getLocalBounds().reduced(StudioStyle::Sizes::editorPaddingPx);

    constexpr float dialScale = StudioStyle::Sizes::dialScale;
    const int preferredRowH = juce::roundToInt((float) StudioStyle::Sizes::rowHeightPx * dialScale);

    const int gap = StudioStyle::Sizes::columnGapPx;

    // Compute 4-row layout based on available height so rows 1-3 are always equal.
    // Keep things a bit tighter than the old preferred sizing.
    const int totalH = bounds.getHeight();
    const int trackRowH = juce::jlimit(50, preferredRowH, juce::roundToInt((float) totalH * 0.19f));
    const int machineRowHForSizing = juce::jlimit(18, 30, juce::roundToInt((float) trackRowH * 0.36f));
    const int controlsHWithMachine = juce::jmax(0, totalH - trackRowH - machineRowHForSizing - 2 * gap);
    const int rowH2 = juce::jmax(40, controlsHWithMachine / 3);
    // Keep row 1–3 equal by adding +10px per row band.
    const int rowHControls = juce::jmax(40, rowH2 - 20 + 10);

    // Row 4: track selectors (7 columns): T1-T6, then MIX.
    constexpr int trackCount = 7;

    auto trackRowArea = bounds.removeFromBottom(trackRowH);

    // Extra band between row 3 and track buttons for per-track machine selectors.
    auto machineRowArea = bounds.removeFromBottom(machineRowHForSizing);

    // Remaining area is the main page (rows 1-3) and also the MIX overlay.
    const auto controlsArea = bounds;

    // Full-width 7-slot spacing.
    const float slotW = (float) trackRowArea.getWidth() / (float) trackCount;
    std::array<int, trackCount> trackCentresX {};
    for (int i = 0; i < trackCount; ++i)
    {
        const float centreX = (float) trackRowArea.getX() + slotW * ((float) i + 0.5f);
        trackCentresX[(size_t) i] = juce::roundToInt(centreX);
    }

    // Rows 1-3 (controls)
    auto row1 = bounds.removeFromTop(rowHControls);
    bounds.removeFromTop(gap);
    auto row2 = bounds.removeFromTop(rowHControls);
    bounds.removeFromTop(gap);
    auto row3 = bounds.removeFromTop(rowHControls);

    // Row 1-3 alignment rules:
    // - Each row has 6 columns.
    // - Column 1 centre aligns with track column 1 centre.
    // - Column 6 centre aligns with track column 7 centre.
    // - Columns 2-5 are evenly spaced between.
    const float x1 = (float) trackCentresX[0];
    const float x6 = (float) trackCentresX[6];
    const float step = (x6 - x1) / 5.0f;

    // Ensure track buttons are bigger than the small toggle buttons (PUNCH/GATE/LFO).
    // Derive small toggle size from the same dial sizing math used below.
    constexpr float minXGap = 6.0f;
    const int dialSideForSizing = juce::jmin(preferredRowH, juce::jmax(30, (int) std::floor(step - minXGap)));
    const float ringThicknessForSizing = StudioStyle::Sizes::dialRingThicknessPx * dialScale;
    const float imagePaddingForSizing = ringThicknessForSizing + 6.0f;
    const float dialImageSideForSizing = (float) dialSideForSizing - 2.0f * imagePaddingForSizing;
    const int smallButtonSideForSizing = juce::jmax(24, juce::roundToInt(dialImageSideForSizing * StudioStyle::Sizes::buttonSizeVsDialImage));

    // (Overlay sizing is tied to the *visible dial arc*; see further down after row 3 is laid out.)

    const int maxTrackSideFromSlots = juce::jmax(26, (int) std::floor(slotW * 0.92f));
    const int minTrackSide = juce::jmin(maxTrackSideFromSlots, smallButtonSideForSizing + 2);

    int trackSide = juce::roundToInt((float) trackRowH * 0.56f * 1.3f);
    trackSide = juce::jlimit(minTrackSide, maxTrackSideFromSlots, trackSide);

    // Keep the track row slightly lower within its band.
    const int trackRowY = trackRowArea.getY()
                          + juce::roundToInt((float) (trackRowArea.getHeight() - trackSide) * 0.8f);

    for (int i = 0; i < trackCount; ++i)
        trackButtons[(size_t) i].setBounds(trackCentresX[(size_t) i] - trackSide / 2, trackRowY, trackSide, trackSide);

    // Track machine combos: match track button width, smaller height/font, tucked close above buttons.
    {
        const int comboW = trackSide;
        const int comboH = juce::jlimit(16, 22, machineRowArea.getHeight());
        constexpr int gapToButtons = 4;

        constexpr int arrowW = 4;
        constexpr int arrowH = 6;

        // Place combos relative to the *actual* track button Y so the visual gap is consistent
        // and much tighter than anchoring to the track-row band.
        const int desiredY = trackRowY - gapToButtons - comboH;
        const int comboY = juce::jmax(machineRowArea.getY(), desiredY);

        for (int i = 0; i < 6; ++i)
        {
            const int cx = trackCentresX[(size_t) i];
            trackMachineCombos[(size_t) i].setBounds(cx - comboW / 2, comboY, comboW, comboH);

            // Small arrow at the left side of the machine combo (visual affordance).
            // Left edge aligned with the track button; vertically centred to the combo.
            const int x = trackButtons[(size_t) i].getX() + 3;
            const int y = trackMachineCombos[(size_t) i].getBounds().getCentreY() - arrowH / 2;
            trackMachineArrows[(size_t) i].setBounds(x, y, arrowW, arrowH);
            trackMachineArrows[(size_t) i].toFront(false);
        }
    }

    auto getColCentreX = [&](int colIndex0To5) -> int
    {
        return juce::roundToInt(x1 + (float) colIndex0To5 * step);
    };

    auto layoutRow = [&](juce::Component& rowToggle,
                         RotaryDial& d1, RotaryDial& d2, RotaryDial& d3, RotaryDial& d4, RotaryDial& d5,
                         juce::Rectangle<int> rowBounds)
    {
        // Allow slightly larger dials by using a smaller minimum horizontal gap.
        const int maxDialSideFromStep = juce::jmax(30, (int) std::floor(step - minXGap));
        // Keep content size independent of row height (row height only affects Y positioning).
        const int dialSide = juce::jmin(preferredRowH, maxDialSideFromStep);

        const float ringThickness = StudioStyle::Sizes::dialRingThicknessPx * dialScale;
        const float imagePadding = ringThickness + 6.0f;
        const float dialImageSide = (float) dialSide - 2.0f * imagePadding;
        const int buttonSide = juce::jmax(24, juce::roundToInt(dialImageSide * StudioStyle::Sizes::buttonSizeVsDialImage));

        const int dialY = rowBounds.getY() + (rowBounds.getHeight() - dialSide) / 2;
        const int buttonY = rowBounds.getY() + (rowBounds.getHeight() - buttonSide) / 2;

        const int c1x = getColCentreX(0);
        const int c2x = getColCentreX(1);
        const int c3x = getColCentreX(2);
        const int c4x = getColCentreX(3);
        const int c5x = getColCentreX(4);
        const int c6x = getColCentreX(5);

        rowToggle.setBounds(c1x - buttonSide / 2, buttonY, buttonSide, buttonSide);

        d1.setBounds(c2x - dialSide / 2, dialY, dialSide, dialSide);
        d2.setBounds(c3x - dialSide / 2, dialY, dialSide, dialSide);
        d3.setBounds(c4x - dialSide / 2, dialY, dialSide, dialSide);
        d4.setBounds(c5x - dialSide / 2, dialY, dialSide, dialSide);
        d5.setBounds(c6x - dialSide / 2, dialY, dialSide, dialSide);

        // Lower dials so the *dial image centre* lines up with the toggle button centre.
        // The dial image is centred within the slider bounds (which excludes the bottom label area).
        const int desiredCentreY = rowToggle.getBounds().getCentreY();
        const int imageCentreY = d1.getSlider().getBounds().translated(d1.getX(), d1.getY()).getCentreY();
        const int deltaY = desiredCentreY - imageCentreY;
        if (deltaY != 0)
        {
            d1.setTopLeftPosition(d1.getX(), d1.getY() + deltaY);
            d2.setTopLeftPosition(d2.getX(), d2.getY() + deltaY);
            d3.setTopLeftPosition(d3.getX(), d3.getY() + deltaY);
            d4.setTopLeftPosition(d4.getX(), d4.getY() + deltaY);
            d5.setTopLeftPosition(d5.getX(), d5.getY() + deltaY);
        }
    };

    // Row 1: PUNCH toggle + 4 grey dials + 1 blue dial (MAIN VOLUME)
    layoutRow(punchToggle,
              pitchControl, decayControl, colorControl, shapeControl, trackVolumeControl,
              row1);

    // Pitch note selector: placed left of the PITCH dial and vertically centred to the dial image.
    {
        // Centre Y: align to the PUNCH button centre.
        const auto dialImage = pitchControl.getSlider().getBounds().translated(pitchControl.getX(), pitchControl.getY());

        const int baseH = juce::jlimit(18, 28, (int) std::lround((float) dialImage.getHeight() * 0.22f));
        const int h = baseH + 10;
        const int w = juce::jlimit(54, 92, (int) std::lround((float) dialImage.getWidth() * 0.62f));
        const int overlap = juce::jlimit(6, 14, (int) std::lround((float) dialImage.getWidth() * 0.10f));

        // Right edge slightly under the PITCH dial arc.
        const int rightEdge = dialImage.getX() + overlap;
        const int x = (rightEdge - w) + 20;
        const int y = punchToggle.getBounds().getCentreY() - h / 2;

        pitchNoteBackdrop.setBounds(x, y, w, h);
        pitchNoteCombo.setBounds(pitchNoteBackdrop.getBounds().reduced(3));

        // Ensure the PITCH dial draws over the right edge of the note selector.
        pitchControl.toFront(false);
    }

    // Machine-dependent value labels: overlay the dial label area for COLOR/SHAPE.
    {
        auto placeOverDialLabel = [](RotaryDial& dial, juce::ComboBox& combo)
        {
            const auto dialBounds = dial.getBounds();
            const float uiScale = dial.getUIScale();

            // Mirror RotaryDial::resized() label geometry for LabelPlacement::Below.
            // This ensures the combo text sits exactly where the dial label text (COLOR/SHAPE) sits.
            const int labelHeight = (int) juce::roundToInt((float) StudioStyle::Sizes::dialLabelAreaHeightPx * uiScale);
            const int labelRaisePx = (int) juce::roundToInt(
                (StudioStyle::Sizes::dialLabelRaiseBasePx
                 + (StudioStyle::Sizes::dialLabelRaiseExtraPx / juce::jmax(0.01f, uiScale)))
                * uiScale);

            auto labelArea = dialBounds.withY(dialBounds.getBottom() - labelHeight).withHeight(labelHeight);
            auto labelBounds = labelArea.translated(0, -labelRaisePx + StudioStyle::Sizes::dialLabelYOffsetPx);

            combo.setBounds(labelBounds);
            combo.toFront(false);
        };

        placeOverDialLabel(colorControl, toneColorValueCombo);
        placeOverDialLabel(shapeControl, chordShapeValueCombo);
    }

    // Track VOLUME/PAN swap overlay (does not affect layout).
    trackPanControl.setBounds(trackVolumeControl.getBounds());
    updateMainVolumeSwapVisibility();

    // Global MAIN VOLUME is MIX-only, but uses the same slot bounds.
    mainVolumeControl.setBounds(trackVolumeControl.getBounds());

    // Row 2: GATE toggle + 4 grey dials + 1 white dial (REV SIZE)
    layoutRow(gateToggle,
              sweepControl, contourControl, delSendControl, revSendControl, revSizeControl,
              row2);

    // Row 3: LFO mode + 4 grey dials + 1 white dial (DEL TIME)
    layoutRow(lfoModeButton,
              lfoSpeedControl, volDistControl, swingControl, chanceControl, delTimeControl,
              row3);

    // Labels under PUNCH / GATE / LFO SHAPE: match dial label height/baseline.
    {
        auto placeButtonLabelLikeDialLabel = [](juce::Component& button, juce::Label& label, RotaryDial& referenceDial)
        {
            const auto dialBounds = referenceDial.getBounds();
            const float uiScale = referenceDial.getUIScale();

            const int labelHeight = (int) juce::roundToInt((float) StudioStyle::Sizes::dialLabelAreaHeightPx * uiScale);
            const int labelRaisePx = (int) juce::roundToInt((float) StudioStyle::Sizes::dialLabelRaiseBasePx * uiScale
                                                           + (float) StudioStyle::Sizes::dialLabelRaiseExtraPx);

            auto labelArea = dialBounds.withY(dialBounds.getBottom() - labelHeight).withHeight(labelHeight);
            auto labelBounds = labelArea.translated(0, -labelRaisePx + StudioStyle::Sizes::dialLabelYOffsetPx);

            label.setBounds(button.getX(), labelBounds.getY(), button.getWidth(), labelBounds.getHeight());
        };

        placeButtonLabelLikeDialLabel(punchToggle, punchLabel, pitchControl);
        placeButtonLabelLikeDialLabel(gateToggle, gateLabel, sweepControl);
        placeButtonLabelLikeDialLabel(lfoModeButton, lfoShapeLabel, lfoSpeedControl);

        // Widen LFO SHAPE label beyond the icon button width so the text isn't clipped.
        {
            const auto current = lfoShapeLabel.getBounds();
            juce::GlyphArrangement ga;
            ga.addLineOfText(lfoShapeLabel.getFont(), lfoShapeLabel.getText(), 0.0f, 0.0f);
            const int textW = (int) std::ceil(ga.getBoundingBox(0, -1, true).getWidth());
            const int desiredW = juce::jmax(current.getWidth(), textW + 12);
            const int clampedW = juce::jmin(desiredW, controlsArea.getWidth());

            int x = lfoModeButton.getBounds().getCentreX() - clampedW / 2;
            x = juce::jlimit(controlsArea.getX(), controlsArea.getRight() - clampedW, x);
            lfoShapeLabel.setBounds(x, current.getY(), clampedW, current.getHeight());
        }
    }

    // Bottom-right scaler decoration.
    {
        constexpr int scalerSize = 18;
        constexpr int scalerMargin = 8;
        constexpr int scalerNudge = 4;
        scalerCorner.setBounds(uiRoot.getWidth() - scalerSize - scalerMargin,
                               uiRoot.getHeight() - scalerSize - scalerMargin,
                               scalerSize,
                               scalerSize);
        scalerCorner.setTopLeftPosition(scalerCorner.getX() + scalerNudge, scalerCorner.getY() + scalerNudge);
        scalerCorner.toFront(false);
    }

    // DELAY TIME sync swap (does not affect layout).
    delTimeSyncControl.setBounds(delTimeControl.getBounds());
    updateDelayTimeSyncVisibility();

    // Swap overlays (do not affect layout):
    // - DELAY toggle swaps REVERB SIZE -> DELAY FEEDB
    // - REVERB toggle swaps DELAY TIME -> REVERB TONE
    delayFeedbackOverlayControl.setBounds(revSizeControl.getBounds());
    reverbToneOverlayControl.setBounds(delTimeControl.getBounds());
    updateDelayReverbSwapVisibility();

    // Overlay dial sizing is fixed; no per-resize reference needed.

    // Overlay mini toggle: upper-left of the LFO SPEED dial (does not affect layout).
    {
        constexpr int overlaySide = 16;
        constexpr int overlayMargin = 4;
        constexpr int overlayDownPx = 10;

        auto placeMiniToggle = [&](juce::Component& toggle, juce::Rectangle<int> dialBounds)
        {
            const int x = dialBounds.getX() + overlayMargin + 5;
            const int y = dialBounds.getY() + overlayMargin + overlayDownPx;
            toggle.setBounds(x, y, overlaySide, overlaySide);
            toggle.toFront(false);
        };

        placeMiniToggle(lfoSpeedOverlayToggle, lfoSpeedControl.getBounds());
        placeMiniToggle(mainVolumeOverlayToggle, trackVolumeControl.getBounds());
        placeMiniToggle(delSendOverlayToggle, delSendControl.getBounds());
        placeMiniToggle(revSendOverlayToggle, revSendControl.getBounds());
        placeMiniToggle(delTimeSyncToggle, delTimeControl.getBounds());
    }

    // Overlay panel: covers VOLUME+DIST, SWING, CHANCE (3 columns) without changing layout.
    {
        const bool on = lfoSpeedOverlayToggle.getToggleState();
        lfoOverlayPanel.setVisible(on);

        if (on)
        {
            auto overlayBounds = volDistControl.getBounds();
            overlayBounds = overlayBounds.getUnion(swingControl.getBounds());
            overlayBounds = overlayBounds.getUnion(chanceControl.getBounds());

            lfoOverlayPanel.setBounds(overlayBounds.expanded(2));
            {
                const auto b = lfoOverlayPanel.getBounds();
                lfoOverlayPanel.setColumnCentresX(volDistControl.getBounds().getCentreX() - b.getX(),
                                                  swingControl.getBounds().getCentreX() - b.getX(),
                                                  chanceControl.getBounds().getCentreX() - b.getX());
            }
            lfoOverlayPanel.toFront(false);
        }
    }

    // MIX overlay: 4 rows (VOLUME, PAN, DELAY, REVERB) x 6 tracks, aligned to T1..T6.
    // In MIX mode we can also use the machine-row band as extra vertical space (machine combos are hidden).
    const auto mixOverlayArea = controlsArea.getUnion(machineRowArea);
    mixerOverlay.setBounds(mixOverlayArea);
    if (mixerMode)
    {
        constexpr int rows = 4;
        constexpr int mixRowsGlobalOffsetPx = 4; // requested: move all MIX big dials down by 4px

        // Use the same dial sizing as the normal (track) page, even though MIX has 4 rows.
        // We'll fit the extra row by tightening vertical spacing between rows.
        const int maxDialSideFromStep = juce::jmax(30, (int) std::floor(step - minXGap));
        const int dialSide = juce::jmin(preferredRowH, maxDialSideFromStep);

        const int availableH = mixOverlayArea.getHeight();
        const int maxSpan = juce::jmax(0, availableH - dialSide);

        // Make the PAN->DELAY gap bigger than the others.
        // We compute step sizes so the last row still fits exactly within the overlay area.
        const int desiredBigDelta = 18;
        constexpr int extraGapPx = 6;
        const int spanForSteps = juce::jmax(0, maxSpan - 3 * extraGapPx);
        const int bigDelta = juce::jlimit(0, spanForSteps, desiredBigDelta);
        const int smallStep = (spanForSteps - bigDelta) / 3;
        const int bigStep = smallStep + bigDelta;

        const int y0 = 0;
        constexpr int lowerRowsOffsetPx = 6; // PAN/DELAY/REVERB requested y +6
        const int y1 = smallStep + extraGapPx;
        const int y2 = y1 + bigStep + extraGapPx;
        const int y3 = y2 + smallStep + extraGapPx;
        const std::array<int, rows> rowY { y0, y1, y2, y3 };

        auto layoutMixRow = [&](int rowIndex, auto dialGetter)
        {
            const int baseY = rowY[(size_t) juce::jlimit(0, rows - 1, rowIndex)];
            // Shift PAN/DELAY down by +offset, and REVERB down by +2*offset.
            // This makes the DELAY->REVERB gap match the (shifted) VOLUME->PAN gap.
            const int extraOffset = (rowIndex == 3 ? (2 * lowerRowsOffsetPx)
                                                   : (rowIndex > 0 ? lowerRowsOffsetPx : 0));
            const int y = baseY + extraOffset + mixRowsGlobalOffsetPx;

            for (int t = 0; t < 6; ++t)
            {
                const int cxLocal = trackCentresX[(size_t) t] - mixOverlayArea.getX();
                auto& d = dialGetter(mixTrackDials[(size_t) t]);
                d.setBounds(cxLocal - dialSide / 2, y, dialSide, dialSide);
            }
        };

        layoutMixRow(0, [](MixTrackDials& s) -> RotaryDial& { return s.volume; });
        layoutMixRow(1, [](MixTrackDials& s) -> RotaryDial& { return s.pan; });
        layoutMixRow(2, [](MixTrackDials& s) -> RotaryDial& { return s.delaySend; });
        layoutMixRow(3, [](MixTrackDials& s) -> RotaryDial& { return s.reverbSend; });

        // MIX column mini dials: two per row (side-by-side) in the MIX slot.
        {
            const float miniScale = dialScale * 0.78f;
            constexpr int arcSidePx = 48;
            const int miniW = arcSidePx + 10;
            const int miniLabelH = (int) juce::roundToInt(20.0f * miniScale);
            const int miniGap = (int) juce::roundToInt(4.0f * miniScale);
            const int miniH = miniW + miniLabelH + miniGap;
            // Tighten the gap between the two mini dials (requested: ~12px closer).
            constexpr int miniPairGap = -4;

            const int mixCxLocal = trackCentresX[(size_t) 6] - mixOverlayArea.getX();
            const int pairW = 2 * miniW + miniPairGap;
            const int leftX = mixCxLocal - pairW / 2;
            const int rightX = leftX + miniW + miniPairGap;

            // Align the mini dial arc-square centre with the big dial's inner image/arc centre.
            // For OverlayDial, the arc-square is at the top of the component with height = miniW.
            const auto getBigDialArcCentreY = [](RotaryDial& d) -> int
            {
                return d.getSlider().getBounds().translated(d.getX(), d.getY()).getCentreY();
            };

            const auto clampMiniY = [&](int desiredTop) -> int
            {
                const int maxTop = juce::jmax(0, mixOverlayArea.getHeight() - miniH);
                return juce::jlimit(0, maxTop, desiredTop);
            };

            const int delayArcCentreY = getBigDialArcCentreY(mixTrackDials[0].delaySend);
            const int reverbArcCentreY = getBigDialArcCentreY(mixTrackDials[0].reverbSend);

            const int delayY = clampMiniY(delayArcCentreY - miniW / 2);
            const int reverbY = clampMiniY(reverbArcCentreY - miniW / 2);

            mixDelayFeedbackMini.setBounds(leftX, delayY, miniW, miniH);
            mixDelayTimeMini.setBounds(rightX, delayY, miniW, miniH);
            mixReverbToneMini.setBounds(leftX, reverbY, miniW, miniH);
            mixReverbSizeMini.setBounds(rightX, reverbY, miniW, miniH);

            // MIX delay sync toggle ("S"): overlay only, does not move any dials.
            // Requested placement: near top-left of TIME dial, centred between FEEDBACK and TIME.
            constexpr int toggleSide = 16;
            const int gapLeft = mixDelayFeedbackMini.getRight();
            const int gapRight = mixDelayTimeMini.getX();
            const int centreX = (gapLeft + gapRight) / 2;
            const int x = centreX - toggleSide / 2;

            // Align the toggle vertically with the DELAY dial label row.
            // (Match RotaryDial label layout; include the extra +2px raise used in initGreyDial.)
            const auto delayDialBounds = mixTrackDials[0].delaySend.getBounds();
            const float delayUiScale = mixTrackDials[0].delaySend.getUIScale();

            const int labelHeight = (int) juce::roundToInt(StudioStyle::Sizes::dialLabelAreaHeightPx * delayUiScale);
            const int labelRaisePx = (int) juce::roundToInt((StudioStyle::Sizes::dialLabelRaiseBasePx
                                                            + (StudioStyle::Sizes::dialLabelRaiseExtraPx / juce::jmax(0.01f, delayUiScale)))
                                                           * delayUiScale);
            constexpr int labelYOffsetPx = StudioStyle::Sizes::dialLabelYOffsetPx;

            const auto labelArea = delayDialBounds.withY(delayDialBounds.getBottom() - labelHeight).withHeight(labelHeight);
            const auto labelBounds = labelArea.translated(0, -labelRaisePx + labelYOffsetPx);

            const int y = labelBounds.getCentreY() - toggleSide / 2  - 62;
            mixDelayTimeSyncToggle.setBounds(x, y, toggleSide, toggleSide);
            mixDelayTimeSyncToggle.toFront(false);
        }

        mixerOverlay.toFront(false);
    }

    // Ensure MIX mode visibility rules win over normal overlay visibility logic above.
    // Global pattern selection: overlay. In MIX mode, position it after MIX dials are laid out.
    {
        const int h = 34 + 10; // user request: PATTERN bank & number height +10
        const int w = trackButtons[0].getWidth();

        constexpr int normalExtraUpPx = 10; // user request: normal mode up by 10px (combos + label)
        constexpr int mixerCombosDownPx = 0; // keep centred on PAN dial inner image

        constexpr int yLiftPx = 20 + normalExtraUpPx;

        int x = 0;
        int y = 0;

        if (! mixerMode)
        {
            // TRACK mode: sit above the MIX button.
            const int cx = trackCentresX[6];
            x = cx - w / 2;

            const int desiredY = trackButtons[6].getY() - h - 6;
            y = juce::jmax(0, desiredY);
        }
        else
        {
            // MIX mode: placed below MAIN VOLUME and aligned to the PAN row.
            const int cx = mainVolumeControl.getBounds().getCentreX();
            x = cx - w / 2;

            const auto panImageLocal = mixTrackDials[0].pan.getSlider().getBounds()
                                           .translated(mixTrackDials[0].pan.getX(), mixTrackDials[0].pan.getY());
            const int panCentreYGlobal = mixOverlayArea.getY() + panImageLocal.getCentreY();
            y = panCentreYGlobal - h / 2;
        }

        if (! mixerMode)
            y -= yLiftPx;
        else
            y += mixerCombosDownPx;

        // TRACK mode: nudge the whole selector down (background + combos + caption).
        if (! mixerMode)
            y += 7;

        patternSelectBackdrop.setBounds(x, y, w, h);

        auto inner = patternSelectBackdrop.getBounds().reduced(3, 4);
        const int innerGap = 0;
        const int bankW = 30;

        auto bank = inner.removeFromLeft(bankW);
        inner.removeFromLeft(innerGap);
        auto idx = inner;

        // Give the text more room (overlap is allowed) so we don't get "..." truncation.
        auto bankBounds = bank.expanded(6, 0);
        auto idxBounds = idx.expanded(10, 0).translated(-4, 0);

        patternBankCombo.setBounds(bankBounds);
        patternIndexCombo.setBounds(idxBounds);

        // Caption: "PATTERN" below the PATTERN combo.
        {
            const float uiScale = mixerMode ? mixTrackDials[0].pan.getUIScale()
                                            : StudioStyle::Sizes::dialScale;

            // Match RotaryDial label space (see RotaryDial.h) so the caption reads the same size.
            int labelH = (int) juce::roundToInt(26.0f * uiScale);

            int labelY = patternSelectBackdrop.getBottom() + 2;

            if (mixerMode)
            {
                // Align to PAN dial label Y.
                const auto panDialBounds = mixTrackDials[0].pan.getBounds().translated(mixerOverlay.getX(), mixerOverlay.getY());
                const float dialUiScale = mixTrackDials[0].pan.getUIScale();

                // Match the actual RotaryDial label layout (see RotaryDial.h)
                const int dialLabelHeight = (int) juce::roundToInt(StudioStyle::Sizes::dialLabelAreaHeightPx * dialUiScale);
                const int dialLabelRaise = (int) juce::roundToInt(StudioStyle::Sizes::dialLabelRaiseBasePx * dialUiScale);
                constexpr int dialLabelYOffsetPx = StudioStyle::Sizes::dialLabelYOffsetPx;

                const auto dialLabelArea = panDialBounds.withY(panDialBounds.getBottom() - dialLabelHeight).withHeight(dialLabelHeight);
                const auto dialLabelBounds = dialLabelArea.translated(0, -dialLabelRaise + dialLabelYOffsetPx);

                labelY = dialLabelBounds.getY() - 2;
                labelH = dialLabelBounds.getHeight();
            }

            patternCaptionLabel.setBounds(patternSelectBackdrop.getX(), labelY, patternSelectBackdrop.getWidth(), labelH);

            // Make the caption read a bit bolder/larger (+2px) without changing global label styling.
            {
                // StudioLookAndFeel label font roughly uses (h * 0.8 + 2). RotaryDial then adds +2.
                const float targetSize = juce::jlimit(10.0f, 28.0f, (float) labelH * 0.8f + 4.0f);
                patternCaptionLabel.setFont(StudioStyle::Fonts::condensedBold(targetSize));
            }
        }

        // Keep selector clickable above the MIX overlay.
        patternSelectBackdrop.toFront(false);
        patternBankCombo.toFront(false);
        patternIndexCombo.toFront(false);
        patternCaptionLabel.toFront(false);
    }

    updateMixerOverlayVisibility();
}
