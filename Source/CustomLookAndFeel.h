#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * Custom LookAndFeel for TapMatrix
 * Handles PNG-based sliders and knobs with programmatic fill bars
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    
    // Load images from assets folder
    void setSliderTrackImage (const juce::Image& trackImage);
    void setSliderThumbImage (const juce::Image& thumbImage);
    void setKnobImage (const juce::Image& knobImage);
    
    // Override slider drawing
    void drawLinearSlider (juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;
    
    // Override rotary knob drawing (for filmstrip knobs later)
    void drawRotarySlider (juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

private:
    juce::Image sliderTrackImage;
    juce::Image sliderThumbImage;
    juce::Image knobImage;
    
    std::unique_ptr<juce::Drawable> sliderTrackDrawable;
    std::unique_ptr<juce::Drawable> sliderThumbDrawable;
    
    // Default fill color
    juce::Colour fillColour {0xff00a985}; /* #00a985 */
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLookAndFeel)
};
