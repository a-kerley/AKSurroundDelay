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
    static constexpr float thumbPadding = 2.0f;     // Padding at top/bottom to prevent thumb overflow
    
    //==========================================================================
    // TEXT STYLING - Adjust fonts and colors here
    //==========================================================================
    static constexpr float valueFontSize = 9.0f;              // Font size for value text in thumb
    static constexpr float labelFontSize = 10.0f;             // Font size for parameter name label
    static constexpr int valueDecimalPlaces = 2;              // Decimal precision for value display
    static inline const juce::Colour valueTextColour {0xff1a1a1a};  // Dark text on light thumb (AARRGGBB)
    static inline const juce::Colour labelTextColour {0xffaaaaaa};  // Gray label text below slider (AARRGGBB)
    
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
    
    /** Update the label text (useful when reassigning to different parameter) */
    void setLabelText (const juce::String& text);
    
    //==========================================================================
    // ACCESSORS
    //==========================================================================
    /** Access the underlying JUCE slider for advanced customization */
    juce::Slider& getSlider() { return slider; }
    
    /** Get the parameter name this slider is displaying */
    juce::String getParameterName() const { return parameterName; }
    
    /** Get the current parameter ID this slider is attached to */
    juce::String getCurrentParameterID() const { return currentParameterID; }
    
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
    juce::Colour accentColour {0xffffffff};  // Custom accent color (white default, AARRGGBB)
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    //==========================================================================
    // HELPER METHODS
    //==========================================================================
    void updateValueLabel();  // Update value label (currently unused)
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderModule)
};
