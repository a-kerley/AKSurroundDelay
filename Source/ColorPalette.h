#pragma once

#include <juce_graphics/juce_graphics.h>

//==============================================================================
/**
 * Color Palette for TapMatrix
 * Single source of truth for all UI colors
 */
namespace ColorPalette
{
    /** Color pair: background color + matching text color */
    struct ColorPair 
    { 
        juce::Colour background; 
        juce::Colour text; 
    };
    
    /** The 10 palette colors with their matching text colors */
    static const ColorPair palettePairs[] = {
        { juce::Colour (0xff3d3a5c) /* #3d3a5c */, juce::Colour (0xffd0d0d0) /* #d0d0d0 */ },  // Dark purple / Light grey
        { juce::Colour (0xff2b5876) /* #2b5876 */, juce::Colour (0xffc8c8c8) /* #c8c8c8 */ },  // Dark blue / Light grey
        { juce::Colour (0xff2a6f7f) /* #2a6f7f */, juce::Colour (0xffc0c0c0) /* #c0c0c0 */ },  // Teal / Medium-light grey
        { juce::Colour (0xff32987e) /* #32987e */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ },  // Sea green / Dark grey
        { juce::Colour (0xff54c181) /* #54c181 */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ },  // Mint / Dark grey
        { juce::Colour (0xff70b861) /* #70b861 */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ },  // Lime / Dark grey
        { juce::Colour (0xff9cae4d) /* #9cae4d */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ },  // Yellow-green / Dark grey
        { juce::Colour (0xffc1a03e) /* #c1a03e */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ },  // Gold / Dark grey
        { juce::Colour (0xffc78441) /* #c78441 */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ },  // Orange / Dark grey
        { juce::Colour (0xffb76d3a) /* #b76d3a */, juce::Colour (0xff1e1e1e) /* #1e1e1e */ }   // Rust / Dark grey
    };
    
    /** Number of colors in the palette */
    static constexpr int paletteSize = 10;
    
    /** Get just the background colors as an array (for backward compatibility) */
    inline juce::Array<juce::Colour> getBackgroundColors()
    {
        juce::Array<juce::Colour> colors;
        for (int i = 0; i < paletteSize; ++i)
            colors.add (palettePairs[i].background);
        return colors;
    }
}
