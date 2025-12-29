#pragma once

#include <juce_graphics/juce_graphics.h>

#include <initializer_list>

// Centralised UI tokens (colours + sizes).
// Keep this header dependency-light so it can be included from components and LookAndFeel.
struct StudioStyle
{
    struct Fonts
    {
        static const juce::String& alphaSmartFamilyTypefaceName()
        {
            static const juce::String name = []
            {
                const auto names = juce::Font::findAllTypefaceNames();
                const auto pick = [&](std::initializer_list<const char*> candidates) -> juce::String
                {
                    for (auto* c : candidates)
                        if (names.contains(juce::String(c), true))
                            return c;
                    return {};
                };

                if (auto s = pick({ "AlphaSmart 3000", "AlphaSmart" }); s.isNotEmpty())
                    return s;

                // Fallback: default sans if AlphaSmart isn't installed.
                return juce::Font::getDefaultSansSerifFontName();
            }();

            return name;
        }

        static const juce::String& condensedFamilyTypefaceName()
        {
            static const juce::String name = []
            {
                const auto names = juce::Font::findAllTypefaceNames();

                const auto pick = [&](std::initializer_list<const char*> candidates) -> juce::String
                {
                    for (auto* c : candidates)
                        if (names.contains(juce::String(c), true))
                            return c;
                    return {};
                };

                // Prefer an actually-condensed face.
                // User request: Arial Narrow + juce::Font::bold.
                if (auto s = pick({ "Arial Narrow", "ArialNarrow", "Arial Narrow Bold", "Arial Bold" }); s.isNotEmpty())
                    return s;

                return juce::Font::getDefaultSansSerifFontName();
            }();

            return name;
        }

        static juce::FontOptions condensedBoldOptions(float heightPx)
        {
            return juce::FontOptions { condensedFamilyTypefaceName(), heightPx, juce::Font::bold };
        }

        static juce::Font condensedBold(float heightPx)
        {
            return juce::Font(condensedBoldOptions(heightPx));
        }

        static juce::FontOptions alphaSmartPlainOptions(float heightPx)
        {
            return juce::FontOptions { alphaSmartFamilyTypefaceName(), heightPx, juce::Font::plain };
        }

        static juce::Font alphaSmartPlain(float heightPx)
        {
            return juce::Font(alphaSmartPlainOptions(heightPx));
        }

        // Centralized *fixed* font sizes (no size derived from component height).
        // If you want a visual adjustment, change these constants only.
        struct SizePx
        {
            static constexpr float smallLabel = 14.0f;
            static constexpr float uiLabel = 14.0f;
            static constexpr float dialLabel = 16.0f;
            static constexpr float overlayDialLabel = 14.0f;

            static constexpr float trackButtonLabel = 24.0f;

            static constexpr float comboText = 20.0f;
            static constexpr float comboTextSmall = 14.0f;
            static constexpr float comboMenu = 20.0f;

            // AlphaSmart-styled combos/menus (Pattern, LFO overlay, machine selectors, LFO mode menu)
            static constexpr float alphaComboText = 20.0f;
            static constexpr float alphaComboTextSmall = 14.0f;
            static constexpr float alphaComboMenu = 20.0f;

            static constexpr float sliderPopup = 20.0f;
            static constexpr float popupMenu = 20.0f;

            static constexpr float dialRangeText = 14.0f;
        };

        static juce::Font smallLabelFont() { return condensedBold(SizePx::smallLabel); }
        static juce::Font uiLabelFont() { return condensedBold(SizePx::uiLabel); }
        static juce::Font dialLabelFont() { return condensedBold(SizePx::dialLabel); }
        static juce::Font overlayDialLabelFont() { return condensedBold(SizePx::overlayDialLabel); }

        static juce::Font comboTextFont() { return condensedBold(SizePx::comboText); }
        static juce::Font comboTextSmallFont() { return condensedBold(SizePx::comboTextSmall); }
        static juce::Font comboMenuFont() { return condensedBold(SizePx::comboMenu); }

        static juce::Font alphaComboTextFont() { return alphaSmartPlain(SizePx::alphaComboText); }
        static juce::Font alphaComboTextSmallFont() { return alphaSmartPlain(SizePx::alphaComboTextSmall); }
        static juce::Font alphaComboMenuFont() { return alphaSmartPlain(SizePx::alphaComboMenu); }

        static juce::Font trackButtonLabelFont() { return condensedBold(SizePx::trackButtonLabel); }
    };

    struct Colours
    {
        static inline const juce::Colour accent { juce::Colour(0xFFFD5252) };
        static inline const juce::Colour foreground { juce::Colour(0xFFFFFFFF) };
        static inline const juce::Colour background { juce::Colour(0xFF000000) };
        static inline const juce::Colour canvas { juce::Colour(0xFFAAB0B0) };

        // Explicit ARGB to avoid any theme/LAF ambiguity.
        static inline const juce::Colour buttonOutline { juce::Colour(0xFF000000) };
        static inline const juce::Colour buttonLight { juce::Colour(0xFF6E6E6E) };
        // Slightly lighter so the 3px black rim reads clearly.
        static inline const juce::Colour buttonDark { juce::Colour(0xFF404040) };
        static inline const juce::Colour buttonPlate { juce::Colour(0xFF4A4A4A) };
    };

    struct Sizes
    {
        // Dials
        static constexpr float dialScale = 0.8f;
        static constexpr float dialRingThicknessPx = 9.0f;
        static constexpr float dialValueFontHeightPx = 18.0f;
        static constexpr float dialImageOffsetPx = 1.0f;
        static constexpr float dialArcRadiusTrimPx = 3.0f;

        // Dial label layout
        static constexpr float dialLabelAreaHeightPx = 26.0f;
        static constexpr float dialLabelGapPx = 6.0f;
        static constexpr int dialLabelYOffsetPx = 2;
        static constexpr float dialLabelRaiseBasePx = 30.0f;
        static constexpr float dialLabelRaiseExtraPx = 2.0f;

        // Overlay dial label layout
        static constexpr float overlayDialLabelAreaHeightPx = 20.0f;
        static constexpr float overlayDialLabelGapPx = 4.0f;
        static constexpr int overlayDialLabelYOffsetPx = -11;

        // Layout
        static constexpr int editorPaddingPx = 16;
        static constexpr int rowHeightPx = 160;
        static constexpr int columnGapPx = 12;

        // Buttons
        static constexpr float buttonCornerRadiusPx = 10.0f;
        static constexpr float buttonOutlinePx = 3.0f;
        // Relative sizes inside the button (based on the inner gradient rect)
        static constexpr float buttonPlateScale = 0.76f;
        static constexpr float buttonIconScale = 0.66f;

        // Backwards-compat: used by existing setters, maps to icon scale.
        static constexpr float buttonImageInsetScale = buttonIconScale;
        static constexpr float buttonIconInsetPx = 2.0f;
        static constexpr float buttonPlateExpandPx = 2.0f;
        static constexpr float buttonOverlayScale = 0.66f;
        static constexpr int buttonGapPx = 10;

        // Gradient transition band (narrow blend)
        static constexpr double gradientBandStart = 0.47;
        static constexpr double gradientBandEnd = 0.53;

        // Button sizing vs dial image size
        static constexpr float buttonSizeVsDialImage = 0.55f;
    };
};
