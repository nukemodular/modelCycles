#include "PluginEditor.h"
#include "PluginProcessor.h"

#include "BinaryData.h"

#include "ui_components/StudioStyle.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), pluginProcessor (p)
{
    constexpr float dialScale = StudioStyle::Sizes::dialScale;

    setLookAndFeel(&lookAndFeel);

    addAndMakeVisible(transposeControl);
    addAndMakeVisible(gateToggle);
    addAndMakeVisible(punchToggle);
    addAndMakeVisible(buttonToggle);
    addAndMakeVisible(lfoModeButton);

    addAndMakeVisible(reverbControl);
    addAndMakeVisible(volumeControl);

    transposeControl.setDialMode(RotaryDial::DialMode::BipolarRing);
    transposeControl.setRangeDisplay(RotaryDial::RangeDisplay::None);
    transposeControl.getSlider().setTextValueSuffix(" st");
    transposeControl.getSlider().setNumDecimalPlacesToDisplay(0);
    transposeControl.getSlider().setDoubleClickReturnValue(true, 0.0);
    transposeControl.setUIScale(dialScale);
    transposeControl.setRingThickness(StudioStyle::Sizes::dialRingThicknessPx * dialScale);
    transposeControl.setValueFontHeight(StudioStyle::Sizes::dialValueFontHeightPx * dialScale);

    // Dial graphics: drawn between the ring and the value text.
    transposeControl.setDialImageFromMemory(BinaryData::KnobGrey_png, BinaryData::KnobGrey_pngSize);

    transposeAttachment = std::make_unique<SliderAttachment>(pluginProcessor.apvts, "transpose", transposeControl.getSlider());

    gateToggle.setClickingTogglesState(true);
    gateToggle.setImagesFromMemory(BinaryData::GATE_OFF_svg, BinaryData::GATE_OFF_svgSize,
                                   BinaryData::GATE_ON_svg, BinaryData::GATE_ON_svgSize);
    gateToggle.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    gateToggle.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);
    gateAttachment = std::make_unique<ButtonAttachment>(pluginProcessor.apvts, "gate", gateToggle);

    punchToggle.setClickingTogglesState(true);
    punchToggle.setImagesFromMemory(BinaryData::PUNCH_OFF_svg, BinaryData::PUNCH_OFF_svgSize,
                                    BinaryData::PUNCH_ON_svg, BinaryData::PUNCH_ON_svgSize);
    punchToggle.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    punchToggle.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);

    buttonToggle.setClickingTogglesState(true);
    buttonToggle.setImagesFromMemory(BinaryData::BUTTON_OFF_svg, BinaryData::BUTTON_OFF_svgSize,
                                     BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);
    buttonToggle.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    buttonToggle.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);

    lfoModeButton.setCornerRadius(StudioStyle::Sizes::buttonCornerRadiusPx);
    lfoModeButton.setImageInsetScale(StudioStyle::Sizes::buttonImageInsetScale);
    lfoModeButton.setImagesFromMemory(BinaryData::BUTTON_ON_svg, BinaryData::BUTTON_ON_svgSize);
    lfoModeButton.clearItems();
    lfoModeButton.addItem(1, "TRG", BinaryData::LFO_TRG_svg, BinaryData::LFO_TRG_svgSize);
    lfoModeButton.addItem(2, "ONE", BinaryData::LFO_ONE_svg, BinaryData::LFO_ONE_svgSize);
    lfoModeButton.addItem(3, "HLF", BinaryData::LFO_HLF_svg, BinaryData::LFO_HLF_svgSize);

    reverbControl.setUIScale(dialScale);
    reverbControl.setDialMode(RotaryDial::DialMode::UnipolarRing);
    reverbControl.setRangeDisplay(RotaryDial::RangeDisplay::None);
    reverbControl.getSlider().setRange(0.0, 127.0, 1.0);
    reverbControl.getSlider().setNumDecimalPlacesToDisplay(0);
    reverbControl.getSlider().setDoubleClickReturnValue(true, 0.0);
    reverbControl.getSlider().setValue(0.0, juce::dontSendNotification);
    reverbControl.setRingThickness(StudioStyle::Sizes::dialRingThicknessPx * dialScale);
    reverbControl.setValueFontHeight(StudioStyle::Sizes::dialValueFontHeightPx * dialScale);
    reverbControl.setDialImageFromMemory(BinaryData::KnobWhite_png, BinaryData::KnobWhite_pngSize);

    volumeControl.setUIScale(dialScale);
    volumeControl.setDialMode(RotaryDial::DialMode::UnipolarRing);
    volumeControl.setRangeDisplay(RotaryDial::RangeDisplay::None);
    volumeControl.getSlider().setRange(0.0, 127.0, 1.0);
    volumeControl.getSlider().setNumDecimalPlacesToDisplay(0);
    volumeControl.getSlider().setDoubleClickReturnValue(true, 100.0);
    volumeControl.getSlider().setValue(0.0, juce::dontSendNotification);
    volumeControl.setRingThickness(StudioStyle::Sizes::dialRingThicknessPx * dialScale);
    volumeControl.setValueFontHeight(StudioStyle::Sizes::dialValueFontHeightPx * dialScale);
    volumeControl.setDialImageFromMemory(BinaryData::KnobBlue_png, BinaryData::KnobBlue_pngSize);

    setSize (500, 320);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel(nullptr);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (StudioStyle::Colours::canvas);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds().reduced(StudioStyle::Sizes::editorPaddingPx);

    constexpr float dialScale = StudioStyle::Sizes::dialScale;
    const int rowH = juce::roundToInt((float) StudioStyle::Sizes::rowHeightPx * dialScale);
    auto topRow = bounds.removeFromTop(rowH);

    const int dialSide = juce::jmin(topRow.getHeight(), topRow.getWidth());
    auto left = topRow.removeFromLeft(dialSide);
    topRow.removeFromLeft(StudioStyle::Sizes::columnGapPx);
    auto buttonsArea = topRow;

    transposeControl.setBounds(left);

    // Centre of the dial image is the centre of the dial's slider bounds.
    const auto dialSliderBoundsInEditor = transposeControl.getSlider().getBounds()
                                               .translated(transposeControl.getX(), transposeControl.getY());
    const int dialImageCentreY = dialSliderBoundsInEditor.getCentreY();

    // Match button size to the *dial image* size (not the full ring size).
    const float ringThickness = StudioStyle::Sizes::dialRingThicknessPx * dialScale;
    const float imagePadding = ringThickness + 6.0f; // keep consistent with StudioLookAndFeel imageBounds
    const float dialImageSide = (float) dialSide - 2.0f * imagePadding;
    const int buttonSide = juce::jmax(24, juce::roundToInt(dialImageSide * StudioStyle::Sizes::buttonSizeVsDialImage));

    constexpr int buttonGap = StudioStyle::Sizes::buttonGapPx;
    const int totalW = 4 * buttonSide + 3 * buttonGap;
    const int startX = buttonsArea.getX() + juce::jmax(0, (buttonsArea.getWidth() - totalW) / 2);
    const int y = dialImageCentreY - buttonSide / 2;

    gateToggle.setBounds(startX + 0 * (buttonSide + buttonGap), y, buttonSide, buttonSide);
    punchToggle.setBounds(startX + 1 * (buttonSide + buttonGap), y, buttonSide, buttonSide);
    buttonToggle.setBounds(startX + 2 * (buttonSide + buttonGap), y, buttonSide, buttonSide);
    lfoModeButton.setBounds(startX + 3 * (buttonSide + buttonGap), y, buttonSide, buttonSide);

    // Second row: two additional dials.
    auto bottomRow = bounds.removeFromTop(rowH);
    const int gap = StudioStyle::Sizes::columnGapPx;
    const int dial2Side = juce::jmin(bottomRow.getHeight(), (bottomRow.getWidth() - gap) / 2);
    auto r1 = bottomRow.removeFromLeft(dial2Side);
    bottomRow.removeFromLeft(gap);
    auto r2 = bottomRow.removeFromLeft(dial2Side);

    reverbControl.setBounds(r1);
    volumeControl.setBounds(r2);
}
