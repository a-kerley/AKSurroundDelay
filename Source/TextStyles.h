#pragma once

#include <juce_graphics/juce_graphics.h>
#include "ColorPalette.h"

/**
 * TEXT STYLES
 * 
 * Predefined text styles for consistent labeling throughout the UI.
 * Use these with SliderModule::setLabelText(AttributedString) or
 * for building custom AttributedString labels.
 * 
 * All font sizes are BASE values (at 1.0x scale).
 * Multiply by scaleFactor for runtime use.
 */
namespace TextStyles
{
    //==========================================================================
    // BASE FONT SIZES (at 1.0x scale)
    //==========================================================================
    constexpr float fontSizeTiny = 8.0f;
    constexpr float fontSizeSmall = 9.0f;
    constexpr float fontSizeMedium = 10.0f;
    constexpr float fontSizeLarge = 12.0f;
    constexpr float fontSizeXLarge = 14.0f;
    constexpr float fontSizeHeading = 16.0f;
    
    //==========================================================================
    // TEXT COLORS
    //==========================================================================
    inline const juce::Colour colorPrimary {0xffffffff};      /* #ffffff - Primary/active text */
    inline const juce::Colour colorSecondary {0xffaaaaaa};    /* #aaaaaa - Secondary/label text */
    inline const juce::Colour colorDimmed {0xff888888};       /* #888888 - Dimmed/inactive text */
    inline const juce::Colour colorMuted {0xff666666};        /* #666666 - Very muted text */
    inline const juce::Colour colorAccent {0xff00aaff};       /* #00aaff - Accent/highlight */
    inline const juce::Colour colorWarning {0xffffaa00};      /* #ffaa00 - Warning/caution */
    inline const juce::Colour colorError {0xffff4444};        /* #ff4444 - Error/alert */
    
    //==========================================================================
    // FONT HELPERS (create fonts with optional scale factor)
    //==========================================================================
    
    /** Regular font at given size */
    inline juce::Font regular (float size, float scale = 1.0f)
    {
        return juce::Font (juce::FontOptions (size * scale));
    }
    
    /** Bold font at given size */
    inline juce::Font bold (float size, float scale = 1.0f)
    {
        return juce::Font (juce::FontOptions (size * scale).withStyle ("Bold"));
    }
    
    /** Italic font at given size */
    inline juce::Font italic (float size, float scale = 1.0f)
    {
        return juce::Font (juce::FontOptions (size * scale).withStyle ("Italic"));
    }
    
    /** Bold italic font at given size */
    inline juce::Font boldItalic (float size, float scale = 1.0f)
    {
        return juce::Font (juce::FontOptions (size * scale).withStyle ("Bold Italic"));
    }
    
    //==========================================================================
    // LABEL BUILDER HELPERS
    // Quick functions to create common label patterns
    //==========================================================================
    
    /**
     * Create a simple single-style label
     * Example: label("DELAY") → "DELAY" in secondary color
     */
    inline juce::AttributedString label (const juce::String& text, float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (text, bold (fontSizeSmall, scale), colorSecondary);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a label with prefix in dimmed color and main text in primary color
     * Example: prefixLabel("TAP ", "1") → "TAP " (dimmed) + "1" (primary)
     */
    inline juce::AttributedString prefixLabel (const juce::String& prefix, 
                                                const juce::String& main,
                                                float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (prefix, regular (fontSizeSmall, scale), colorDimmed);
        as.append (main, bold (fontSizeSmall, scale), colorPrimary);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a label with main text in primary color and suffix in dimmed color
     * Example: suffixLabel("DELAY", " ms") → "DELAY" (primary) + " ms" (dimmed)
     */
    inline juce::AttributedString suffixLabel (const juce::String& main,
                                                const juce::String& suffix,
                                                float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (main, bold (fontSizeSmall, scale), colorPrimary);
        as.append (suffix, regular (fontSizeSmall, scale), colorDimmed);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a two-part label with custom colors
     * Example: twoTone("L", "EVEL", colorAccent, colorSecondary) → "L" (accent) + "EVEL" (secondary)
     */
    inline juce::AttributedString twoTone (const juce::String& part1,
                                            const juce::String& part2,
                                            juce::Colour color1,
                                            juce::Colour color2,
                                            float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (part1, bold (fontSizeSmall, scale), color1);
        as.append (part2, bold (fontSizeSmall, scale), color2);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a label with accent-colored main text
     * Example: accentLabel("ACTIVE") → "ACTIVE" in accent color
     */
    inline juce::AttributedString accentLabel (const juce::String& text, float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (text, bold (fontSizeSmall, scale), colorAccent);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a numbered item label (e.g., "TAP 1", "CH 2")
     * Example: numberedLabel("TAP", 1) → "TAP " (secondary) + "1" (primary, larger)
     */
    inline juce::AttributedString numberedLabel (const juce::String& prefix,
                                                  int number,
                                                  float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (prefix + " ", regular (fontSizeSmall, scale), colorSecondary);
        as.append (juce::String (number), bold (fontSizeMedium, scale), colorPrimary);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a channel label (e.g., "L", "R", "C", "LFE")
     * Example: channelLabel("L") → "L" in accent color, bold
     */
    inline juce::AttributedString channelLabel (const juce::String& channel, float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (channel, bold (fontSizeMedium, scale), colorAccent);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    /**
     * Create a parameter label with units
     * Example: paramLabel("FREQ", "Hz") → "FREQ" (secondary) + " Hz" (dimmed, smaller)
     */
    inline juce::AttributedString paramLabel (const juce::String& param,
                                               const juce::String& unit,
                                               float scale = 1.0f)
    {
        juce::AttributedString as;
        as.append (param, bold (fontSizeSmall, scale), colorSecondary);
        as.append (" " + unit, regular (fontSizeTiny, scale), colorDimmed);
        as.setJustification (juce::Justification::centred);
        as.setWordWrap (juce::AttributedString::none);
        return as;
    }
    
    //==========================================================================
    // BUILDER CLASS
    // For more complex labels, use the builder pattern
    //==========================================================================
    
    /**
     * Fluent builder for creating custom AttributedString labels
     * 
     * Example:
     *   auto label = TextStyles::Builder()
     *       .text("TAP ", TextStyles::colorDimmed)
     *       .bold("1", TextStyles::colorAccent)
     *       .text(" DELAY", TextStyles::colorSecondary)
     *       .centered()
     *       .build();
     */
    class Builder
    {
    public:
        Builder (float scale = 1.0f) : scaleFactor (scale) {}
        
        /** Append regular text */
        Builder& text (const juce::String& str, juce::Colour color = colorSecondary)
        {
            result.append (str, regular (fontSizeSmall, scaleFactor), color);
            return *this;
        }
        
        /** Append bold text */
        Builder& bold (const juce::String& str, juce::Colour color = colorPrimary)
        {
            result.append (str, TextStyles::bold (fontSizeSmall, scaleFactor), color);
            return *this;
        }
        
        /** Append italic text */
        Builder& italic (const juce::String& str, juce::Colour color = colorSecondary)
        {
            result.append (str, TextStyles::italic (fontSizeSmall, scaleFactor), color);
            return *this;
        }
        
        /** Append text with custom font size */
        Builder& sized (const juce::String& str, float fontSize, juce::Colour color = colorSecondary, bool isBold = false)
        {
            auto font = isBold ? TextStyles::bold (fontSize, scaleFactor) : regular (fontSize, scaleFactor);
            result.append (str, font, color);
            return *this;
        }
        
        /** Set centered justification */
        Builder& centered()
        {
            result.setJustification (juce::Justification::centred);
            return *this;
        }
        
        /** Set left justification */
        Builder& left()
        {
            result.setJustification (juce::Justification::left);
            return *this;
        }
        
        /** Set right justification */
        Builder& right()
        {
            result.setJustification (juce::Justification::right);
            return *this;
        }
        
        /** Build and return the AttributedString */
        juce::AttributedString build()
        {
            result.setWordWrap (juce::AttributedString::none);
            return result;
        }
        
    private:
        juce::AttributedString result;
        float scaleFactor;
    };
    
} // namespace TextStyles
