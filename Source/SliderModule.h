#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
 * Reusable vertical slider module with label and value display.
 * 
 * Features:
 * - SVG-based track and thumb rendering (via CustomLookAndFeel)
 * - Parameter name label below slider
 * - Value display inside thumb (moves with slider)
 * - Customizable accent color
 * - Dynamic parameter reassignment (useful for context-switching UIs)
 * 
 * Usage:
 *   SliderModule mySlider {"GAIN"};
 *   mySlider.setAccentColour (juce::Colours::blue);
 *   mySlider.attachToParameter (apvts, "gain");
 *   // Later, reassign to different parameter:
 *   mySlider.attachToParameter (apvts, "gain_tap2");
 */
class SliderModule : public juce::Component
{
public:
    //==========================================================================
    // DIMENSIONS - Adjust these to match your SVG assets
    //==========================================================================
    static constexpr float trackWidth = 38.0f;      // Width of slider track SVG
    static constexpr float trackHeight = 170.0f;    // Height of slider track SVG
    static constexpr float thumbWidth = 34.0f;      // Width of slider thumb SVG
    static constexpr float thumbHeight = 13.0f;     // Height of slider thumb SVG
    static constexpr float thumbPadding = -3.0f;     // Padding at top/bottom to prevent thumb overflow
    
    // Fill bar spritesheet settings (170 frames at 4x scale)
    static constexpr int spritesheetTotalFrames = 170;   // Total number of frames in spritesheet
    static constexpr int spritesheetFrameWidth = 152;    // Frame width at 4x scale (38px × 4)
    static constexpr int spritesheetFrameHeight = 680;   // Frame height at 4x scale (170px × 4)
    
    //==========================================================================
    // TEXT STYLING - Adjust fonts and colors here
    //==========================================================================
    static constexpr float labelSpacing = 5.0f;               // Spacing between slider bottom and label
    static constexpr float labelHeight = 16.0f;                 // Height reserved for parameter label
    static constexpr float componentPaddingTop = 10.0f;          // Extra padding above slider
    static constexpr float componentPaddingBottom = 5.0f;       // Extra padding below label
    static constexpr float componentPaddingLeft = 10.0f;         // Extra padding left of slider
    static constexpr float componentPaddingRight = 10.0f;        // Extra padding right of slider
    static constexpr float valueFontSize = 9.0f;                // Font size for value text in thumb
    static constexpr float labelFontSize = 10.0f;             // Font size for parameter name label
    static inline const juce::Colour labelTextColour {0xffaaaaaa};  /* #aaaaaaff */ // Gray label text below slider
    
    //==========================================================================
    // CONSTRUCTOR & DESTRUCTOR
    //==========================================================================
    /** Create a slider module with the given label text (parameter name) */
    SliderModule (const juce::String& labelText);
    ~SliderModule() override;
    
    //==========================================================================
    // PARAMETER BINDING
    //==========================================================================
    /** 
     * Attach this slider to a parameter in the APVTS.
     * Can be called multiple times to reassign to different parameters.
     * Perfect for context-switching UIs (e.g., switching between taps).
     * 
     * @param apvts     The AudioProcessorValueTreeState containing parameters
     * @param parameterID  The ID of the parameter to control
     */
    void attachToParameter (juce::AudioProcessorValueTreeState& apvts, 
                           const juce::String& parameterID);
    
    /** Detach from current parameter (slider becomes inactive) */
    void detachFromParameter();
    
    //==========================================================================
    // CUSTOMIZATION
    //==========================================================================
    /** Set a custom accent color for this slider (future: could tint track/thumb) */
    void setAccentColour (juce::Colour colour) { accentColour = colour; }
    
    /** Get the current accent color */
    juce::Colour getAccentColour() const { return accentColour; }
    
    /** Set a custom text color for the value display */
    void setValueTextColour (juce::Colour colour) { valueTextColour = colour; }
    
    /** Get the current value text color */
    juce::Colour getValueTextColour() const { return valueTextColour; }
    
    /** Update the label text (useful when reassigning to different parameter) */
    void setLabelText (const juce::String& text);
    
    /** Set the unit suffix displayed after the value (e.g., "dB", "ms", "%", "Hz") */
    void setValueSuffix (const juce::String& suffix) { valueSuffix = suffix; }
    
    /** Get the current value suffix */
    juce::String getValueSuffix() const { return valueSuffix; }
    
    /** Set the number of decimal places for value display */
    void setDecimalPlaces (int places) { valueDecimalPlaces = places; }
    
    /** Get the number of decimal places for value display */
    int getDecimalPlaces() const { return valueDecimalPlaces; }
    
    /** Enable/disable debug border to visualize component bounds */
    void setShowDebugBorder (bool show) { showDebugBorder = show; repaint(); }
    
    /** 
     * Set the interval between discrete steps (0 = continuous).
     * For discrete sliders, set this to the step size.
     * Example: For a 0-9 slider, use setInterval(1.0) to snap to integers.
     */
    void setInterval (double interval) { slider.setRange (slider.getMinimum(), slider.getMaximum(), interval); }
    
    //==========================================================================
    // ACCESSORS
    //==========================================================================
    /** Access the underlying JUCE slider for advanced customization */
    juce::Slider& getSlider() { return slider; }
    
    /** Get the parameter name this slider is displaying */
    juce::String getParameterName() const { return parameterName; }
    
    /** Get the current parameter ID this slider is attached to */
    juce::String getCurrentParameterID() const { return currentParameterID; }
    
    /** Get the ideal width for this component including padding */
    static int getIdealWidth() 
    { 
        return (int)(trackWidth + componentPaddingLeft + componentPaddingRight); 
    }
    
    /** Get the ideal height for this component including padding */
    static int getIdealHeight() 
    { 
        return (int)(componentPaddingTop + trackHeight + labelSpacing + labelHeight + componentPaddingBottom); 
    }
    
    //==========================================================================
    // COMPONENT OVERRIDES
    //==========================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    //==========================================================================
    // MEMBER VARIABLES
    //==========================================================================
    juce::Slider slider;                // The underlying JUCE slider control
    juce::Label nameLabel;              // Parameter name label (below slider)
    juce::Label valueLabel;             // Value label (currently unused - value shown in thumb)
    
    juce::String parameterName;         // Display name (e.g., "GAIN")
    juce::String currentParameterID;    // Current APVTS parameter ID (e.g., "gain_tap1")
    juce::String valueSuffix;           // Unit suffix for value display (e.g., "dB", "ms", "%")
    int valueDecimalPlaces = 1;         // Decimal precision for value display (default: 2)
    juce::Colour accentColour {0xffffffff};  /* #ffffff */ // Custom accent color (white default)
    juce::Colour valueTextColour {0xffcccccc};  /* #cccccc */ // Custom text color (light grey default)
    bool showDebugBorder = false;       // Show debug border around component bounds
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    // Fill bar image (loaded once, shared by all instances)
    static juce::Image fillBarImage;
    static bool fillBarImageLoaded;
    
    // Pre-generated color variants (key = hue degree 0-360)
    static std::map<int, juce::Image> colorVariants;
    static bool colorVariantsLoaded;
    
    //==========================================================================
    // HELPER METHODS
    //==========================================================================
    void updateValueLabel();  // Update value label (currently unused)
    
    static void loadColorVariants();  // Load all pre-cached color variants from disk
    static void generateColorVariantCache();  // Generate and save color variants (call once when colors change)
    static const juce::Image& getVariantForColor (const juce::Colour& colour);  // Get nearest variant
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderModule)
};
