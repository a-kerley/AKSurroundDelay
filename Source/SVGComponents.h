#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/**
 * Rotary knob component that uses SVG graphics from assets/Knob Big.svg
 */
class SVGRotaryKnob : public juce::Slider
{
public:
    SVGRotaryKnob (const juce::String& labelText = "", juce::Colour accentColor = juce::Colour (0xff00A985));
    ~SVGRotaryKnob() override = default;
    
    void paint (juce::Graphics& g) override;
    void setAccentColour (juce::Colour newColour);
    
private:
    std::unique_ptr<juce::Drawable> knobSVG;
    juce::String label;
    juce::Colour colour;
    
    // Rotation range for the knob (in radians)
    static constexpr float minAngle = juce::MathConstants<float>::pi * 1.2f;  // 7 o'clock
    static constexpr float maxAngle = juce::MathConstants<float>::pi * 2.8f;  // 5 o'clock
    
    float valueToAngle() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SVGRotaryKnob)
};

//==============================================================================
/**
 * Vertical slider component that uses SVG graphics from assets/Vertical Controller.svg
 */
class SVGVerticalSlider : public juce::Slider
{
public:
    SVGVerticalSlider (const juce::String& labelText, juce::Colour accentColor = juce::Colour (0xff00A985));
    ~SVGVerticalSlider() override = default;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
private:
    std::unique_ptr<juce::Drawable> sliderBackgroundSVG;
    
    juce::String label;
    juce::Colour colour;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SVGVerticalSlider)
};
