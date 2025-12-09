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
        { juce::Colour (0xff64629d) /* #64629d */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Dark purple / Light grey
        { juce::Colour (0xff448bbb) /* #448bbb */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Dark blue / Light grey
        { juce::Colour (0xff40a8bf) /* #40a8bf */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Teal / Medium-light grey
        { juce::Colour (0xff32987e) /* #32987e */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Sea green / Dark grey
        { juce::Colour (0xff54c181) /* #54c181 */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Mint / Dark grey
        { juce::Colour (0xff70b861) /* #70b861 */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Lime / Dark grey
        { juce::Colour (0xff9cae4d) /* #9cae4d */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Yellow-green / Dark grey
        { juce::Colour (0xffc1a03e) /* #c1a03e */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Gold / Dark grey
        { juce::Colour (0xffc78441) /* #c78441 */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ },  // Orange / Dark grey
        { juce::Colour (0xffb76d3a) /* #b76d3a */, juce::Colour (0xff0f0f0f) /* #0f0f0f */ }   // Rust / Dark grey
    };
    
    /** Number of colors in the palette */
    static constexpr int paletteSize = 10;
    
    //==============================================================================
    // PLUGIN INTERFACE COLORS
    //==============================================================================
    
    /** Main plugin background color */
    static inline const juce::Colour pluginBackground {0xff0f0f0f}; /* #0f0f0f */
    
    //==============================================================================
    // VIEW PRESET SELECTOR COLORS
    //==============================================================================
    
    /** Segmented control border */
    static inline const juce::Colour presetBorderColour {0xff4a4a4a}; /* #4a4a4a */
    
    /** Selected preset pill background */
    static inline const juce::Colour presetPillColour {0xff5a5a5a}; /* #5a5a5a */
    
    /** Dimmed/custom preset pill background */
    static inline const juce::Colour presetPillDimmedColour {0xff3a3a3a}; /* #3a3a3a */
    
    /** Preset text color (active) */
    static inline const juce::Colour presetTextColour {0xffffffff}; /* #ffffff */
    
    /** Preset text color (dimmed/inactive) */
    static inline const juce::Colour presetTextDimmedColour {0xff888888}; /* #888888 */
    
    /** Segmented control background */
    static inline const juce::Colour presetBackgroundColour {0xff2a2a2a}; /* #2a2a2a */
    
    //==============================================================================
    // 3D VIEWPORT COLORS
    //==============================================================================
    
    /** Helper to convert 8-bit hex color to normalized float (0.0-1.0) */
    constexpr float hexByteToFloat (uint8_t byte)
    {
        return static_cast<float>(byte) / 255.0f;
    }
    
    /** RGBA color struct for 3D viewport elements */
    struct ViewportColor3D
    {
        float r, g, b, a;
        
        /** Constructor from normalized RGBA values */
        constexpr ViewportColor3D (float r_, float g_, float b_, float a_) 
            : r(r_), g(g_), b(b_), a(a_) {}
        
        /** Constructor from 24-bit RGB hex + alpha byte (RGBA format) */
        constexpr ViewportColor3D (uint32_t rgba)
            : r (hexByteToFloat ((rgba >> 24) & 0xFF)),
              g (hexByteToFloat ((rgba >> 16) & 0xFF)),
              b (hexByteToFloat ((rgba >> 8) & 0xFF)),
              a (hexByteToFloat (rgba & 0xFF)) {}
    };
    
    /** 3D viewport background - dark grey (#0f0f0f @ 100% alpha) */
    static constexpr ViewportColor3D viewport3DBackground (0x0f0f0fff);  // #0f0f0f
    
    /** Room wall faces - semi-transparent dark grey (#262626 @ 60% alpha) */
    static constexpr ViewportColor3D roomWallsColour (0x26262699);  // #262626 with alpha 0x99 (~60%)
    
    /** Floor grid - medium grey (#666666 @ 100% alpha) */
    static constexpr ViewportColor3D gridColour (0x666666FF);
    
    /** Listener sphere - medium-light grey (#808080 @ 100% alpha) */
    static constexpr ViewportColor3D sphereColour (0x808080FF);
    
    /** Room edges - white wireframe (#FFFFFF @ 100% alpha) */
    static constexpr ViewportColor3D roomEdgesColour (0xFFFFFFFF);
    
    /** Room floor grid color - medium grey (#666666 @ 100% alpha) */
    static constexpr ViewportColor3D floorGridColour (0x666666FF);
    
    //==============================================================================
    // INACTIVE/DISABLED STATE COLORS
    // Used for controls that are currently unavailable (e.g., Height when no height axis)
    //==============================================================================
    
    /** Inactive slider track tint - dark grey, low contrast */
    static inline const juce::Colour inactiveTrackColour {0xff3a3a3a}; /* #3a3a3a */
    
    /** Inactive slider thumb tint - slightly lighter grey */
    static inline const juce::Colour inactiveThumbColour {0xff4a4a4a}; /* #4a4a4a */
    
    /** Inactive value text color - very dim */
    static inline const juce::Colour inactiveValueTextColour {0xff555555}; /* #555555 */
    
    /** Inactive label text color - dim grey */
    static inline const juce::Colour inactiveLabelColour {0xff555555}; /* #555555 */
    
    /** Inactive fill bar alpha multiplier (0.0-1.0) */
    static constexpr float inactiveFillBarAlpha = 0.3f;
    
    //==============================================================================
    // CONTROL GROUP COLORS
    // Used for grouped control sections (e.g., Position group)
    //==============================================================================
    
    /** Group label text color - matches default SliderModule label color (#aaaaaa) */
    static inline const juce::Colour groupLabelColour {0xffaaaaaa}; /* #aaaaaa */
    
    /** Get just the background colors as an array (for backward compatibility) */
    inline juce::Array<juce::Colour> getBackgroundColors()
    {
        juce::Array<juce::Colour> colors;
        for (int i = 0; i < paletteSize; ++i)
            colors.add (palettePairs[i].background);
        return colors;
    }
}
