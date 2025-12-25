#pragma once

#include <juce_graphics/juce_graphics.h>

// Centralised UI tokens (colours + sizes).
// Keep this header dependency-light so it can be included from components and LookAndFeel.
struct StudioStyle
{
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
        static constexpr double gradientBandStart = 0.48;
        static constexpr double gradientBandEnd = 0.52;

        // Button sizing vs dial image size
        static constexpr float buttonSizeVsDialImage = 0.55f;
    };
};
