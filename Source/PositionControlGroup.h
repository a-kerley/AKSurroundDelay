#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "SliderModule.h"
#include "ColorPalette.h"

//==============================================================================
/**
 * PositionControlGroup
 * 
 * A grouped control panel containing 3D positioning controls for a delay tap:
 *   - LEFT / RIGHT: Horizontal fader for stereo panning (Fader 28 x 84 Horizontal)
 *   - FRONT / BACK: Vertical fader for depth positioning (Fader 32 x 129 Front-Back)
 *   - HEIGHT: Vertical fader for vertical positioning (Fader 32 x 129) - may be inactive
 * 
 * LAYOUT (at 1.0x scale):
 * ┌─────────────────────────────────────────────────────────────────────┐
 * │  LEFT / RIGHT label (14px high)                                     │
 * │  ┌─────────────────────────────────────────────────────────┐       │
 * │  │  Horizontal L/R fader (28px tall, 84px wide)            │       │
 * │  └─────────────────────────────────────────────────────────┘       │
 * │                                                                     │
 * │  ┌───────────────────────┐  gap  ┌───────────────────────┐         │
 * │  │                       │       │                       │         │
 * │  │  Front/Back fader     │       │  Height fader         │         │
 * │  │  (32x129)             │       │  (32x129) - inactive  │         │
 * │  │                       │       │                       │         │
 * │  └───────────────────────┘       └───────────────────────┘         │
 * │  FRONT / BACK label              HEIGHT label                       │
 * └─────────────────────────────────────────────────────────────────────┘
 *   POSITION (group label)
 * 
 * DIMENSIONS at 1.0x scale:
 *   - Total control area height: 170px (matches full-height fader)
 *   - Group label below: ~14px additional
 *   - Total width: determined by child faders + internal gaps
 * 
 * The Height fader can be enabled/disabled based on whether the current
 * channel configuration supports height positioning (e.g., 7.1.4 Atmos).
 */
class PositionControlGroup : public juce::Component
{
public:
    //==========================================================================
    // LAYOUT CONSTANTS (base values at 1.0x scale)
    // Adjust these to tweak the internal positioning
    //==========================================================================
    
    // Total height from top of L/R fader to bottom of F/B and Height faders
    // This matches the height of a Fader_38x170 for visual consistency
    static constexpr float baseTotalControlHeight = 170.0f;
    
    // Spacing between L/R fader and the two vertical faders below
    // ADJUST THIS: Negative values move vertical faders UP (overlap with L/R label area)
    static constexpr float baseVerticalGap = 3.0f;
    
    // Horizontal gap between the two vertical faders (F/B and Height)
    static constexpr float baseHorizontalGap = 5.0f;
    
    // Group label settings
    static constexpr float baseGroupLabelHeight = 14.0f;
    static constexpr float baseGroupLabelSpacing = 17.0f;  // Gap above the group label
    static constexpr float baseGroupLabelFontSize = 10.0f;
    
    // Individual fader label height (rendered by SliderModule)
    static constexpr float baseFaderLabelHeight = 14.0f;
    
    // L/R fader dimensions (from Fader_28x84_HorizontalLeftRight)
    static constexpr float baseLRFaderWidth = 84.0f;   // Travel direction (horizontal)
    static constexpr float baseLRFaderHeight = 28.0f;  // Short dimension (vertical)
    
    // Vertical fader dimensions (from Fader_32x129)
    static constexpr float baseVerticalFaderWidth = 32.0f;
    static constexpr float baseVerticalFaderHeight = 129.0f;
    
    //==========================================================================
    // CONSTRUCTOR & DESTRUCTOR
    //==========================================================================
    
    /**
     * Create a position control group for the specified tap.
     * 
     * @param tapIndex  The tap index (0-7) for parameter naming
     * @param apvts     Reference to the AudioProcessorValueTreeState
     */
    PositionControlGroup (int tapIndex, juce::AudioProcessorValueTreeState& apvts);
    ~PositionControlGroup() override = default;
    
    //==========================================================================
    // COMPONENT OVERRIDES
    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    //==========================================================================
    // SCALING
    //==========================================================================
    
    /** Set the UI scale factor (1.0 to 3.0) */
    void setScaleFactor (float scale);
    
    /** Get the current scale factor */
    float getScaleFactor() const { return currentScaleFactor; }
    
    //==========================================================================
    // DIMENSIONS (scaled)
    //==========================================================================
    
    /** Get the preferred width at current scale */
    int getPreferredWidth() const;
    
    /** Get the preferred height at current scale (includes group label) */
    int getPreferredHeight() const;
    
    //==========================================================================
    // HEIGHT CONTROL ENABLE/DISABLE
    //==========================================================================
    
    /**
     * Enable or disable the Height fader.
     * When disabled, the Height fader is greyed out and non-interactive.
     * Use this when the current channel format doesn't support height.
     */
    void setHeightEnabled (bool enabled);
    
    /** Check if Height control is currently enabled */
    bool isHeightEnabled() const { return heightEnabled; }
    
    //==========================================================================
    // ACCENT COLOR
    //==========================================================================
    
    /** Set the accent color for all faders in this group (track tinting) */
    void setAccentColour (juce::Colour colour);
    
    /** Set the value text color for all faders in this group */
    void setValueTextColour (juce::Colour colour);
    
    /** Get the current accent color */
    juce::Colour getAccentColour() const { return accentColour; }
    
    //==========================================================================
    // LOOK AND FEEL
    //==========================================================================
    
    /** Set the custom LookAndFeel for the slider modules */
    void setSliderLookAndFeel (juce::LookAndFeel* lf);
    
private:
    //==========================================================================
    // MEMBER VARIABLES
    //==========================================================================
    
    int tapIndex;                          // Which tap this group belongs to (0-7)
    float currentScaleFactor = 1.0f;       // Current UI scale
    bool heightEnabled = true;             // Whether Height fader is interactive
    juce::Colour accentColour {0xffffffff}; // Current accent color
    
    //==========================================================================
    // CHILD COMPONENTS
    //==========================================================================
    
    // The three position faders
    SliderModule leftRightFader   {"LEFT / RIGHT", FaderStyle::Fader_28x84_HorizontalLeftRight};
    SliderModule frontBackFader   {"FRONT / BACK", FaderStyle::Fader_32x129_FrontBack};
    SliderModule heightFader      {"HEIGHT",       FaderStyle::Fader_32x129};
    
    //==========================================================================
    // HELPER METHODS
    //==========================================================================
    
    /** Calculate scaled value from base */
    float scaled (float baseValue) const { return baseValue * currentScaleFactor; }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionControlGroup)
};
