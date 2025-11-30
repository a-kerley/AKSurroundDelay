#include "CustomLookAndFeel.h"
#include "SliderModule.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Set default colors
    setColour (juce::Slider::thumbColourId, juce::Colours::white);
    setColour (juce::Slider::trackColourId, fillColour);
    
    // Load SVG assets - use absolute path to workspace
    auto assetsPath = juce::File ("/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets");
    
    auto trackFile = assetsPath.getChildFile ("SliderTrack.svg");
    auto thumbFile = assetsPath.getChildFile ("SliderThumb.svg");
    
    if (trackFile.existsAsFile())
    {
        sliderTrackDrawable = juce::Drawable::createFromSVGFile (trackFile);
        DBG ("Loaded SliderTrack.svg");
    }
    else
        DBG ("SliderTrack.svg NOT FOUND at: " + trackFile.getFullPathName());
    
    if (thumbFile.existsAsFile())
    {
        sliderThumbDrawable = juce::Drawable::createFromSVGFile (thumbFile);
        DBG ("Loaded SliderThumb.svg");
    }
    else
        DBG ("SliderThumb.svg NOT FOUND at: " + thumbFile.getFullPathName());
}

void CustomLookAndFeel::setSliderTrackImage (const juce::Image& trackImage)
{
    sliderTrackImage = trackImage;
}

void CustomLookAndFeel::setSliderThumbImage (const juce::Image& thumbImage)
{
    sliderThumbImage = thumbImage;
}

void CustomLookAndFeel::setKnobImage (const juce::Image& knobImage)
{
    this->knobImage = knobImage;
}

void CustomLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPos,
                                          float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style,
                                          juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        // Get accent color from parent SliderModule (if available)
        juce::Colour tintColour = juce::Colours::white;  // Default: no tint
        if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
        {
            tintColour = sliderModule->getAccentColour();
            DBG ("Drawing slider with tint: " + tintColour.toDisplayString (true));
        }
        else
        {
            DBG ("No SliderModule parent found!");
        }
        
        // Draw using SVG assets or fallback
        
        // 1. Draw the track at original SVG size with color tint
        if (sliderTrackDrawable)
        {
            auto trackBounds = juce::Rectangle<float> (x + (width - SliderModule::trackWidth) * 0.5f, 
                                                       y + (height - SliderModule::trackHeight) * 0.5f, 
                                                       SliderModule::trackWidth, SliderModule::trackHeight);
            
            // Clone the drawable so we don't modify the original
            auto drawableCopy = sliderTrackDrawable->createCopy();
            
            // Replace all colors in the SVG with the tint color
            drawableCopy->replaceColour (juce::Colours::black, tintColour);
            drawableCopy->replaceColour (juce::Colours::white, tintColour);
            drawableCopy->replaceColour (juce::Colour (0xfff2f2f7), tintColour);  // Your SVG stroke color
            
            // Draw the recolored SVG
            drawableCopy->drawWithin (g, trackBounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            // Fallback: draw a simple track (light gray, AARRGGBB)
            auto fallbackTrackWidth = 2.0f;
            auto trackX = x + width * 0.5f - fallbackTrackWidth * 0.5f;
            g.setColour (juce::Colour (0xffe0e0e0));
            g.fillRect (trackX, (float) y, fallbackTrackWidth, (float) height);
        }
        
        // 2. Draw the thumb with constrained positioning
        // Constrain thumb position to stay within track bounds
        float topLimit = y + SliderModule::thumbPadding;
        float bottomLimit = y + height - SliderModule::thumbPadding;
        float constrainedPos = juce::jlimit (topLimit, bottomLimit, sliderPos);
        
        auto thumbX = x + width * 0.5f - SliderModule::thumbWidth * 0.5f;
        auto thumbY = constrainedPos - SliderModule::thumbHeight * 0.5f;
        auto thumbBounds = juce::Rectangle<float> (thumbX, thumbY, SliderModule::thumbWidth, SliderModule::thumbHeight);
        
        if (sliderThumbDrawable)
        {
            // Clone and recolor the thumb SVG
            auto thumbCopy = sliderThumbDrawable->createCopy();
            thumbCopy->replaceColour (juce::Colours::black, tintColour);
            thumbCopy->replaceColour (juce::Colours::white, tintColour);
            thumbCopy->replaceColour (juce::Colour (0xfff2f2f7), tintColour);
            
            thumbCopy->drawWithin (g, thumbBounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            // Fallback: draw a simple thumb with tint color
            g.setColour (tintColour);
            g.fillRoundedRectangle (thumbBounds, 2.0f);
        }
        
        // Draw value text in the center of the thumb
        auto value = slider.getValue();
        if (std::isfinite (value))
        {
            g.setColour (SliderModule::valueTextColour);
            g.setFont (juce::FontOptions (SliderModule::valueFontSize, juce::Font::bold));
            
            // Format value based on range (0-1 shows decimals, larger shows integers)
            juce::String valueText;
            if (value <= 1.0f && value >= 0.0f)
                valueText = juce::String (value, SliderModule::valueDecimalPlaces);
            else
                valueText = juce::String ((int)value);  // Integer for larger values
            
            g.drawText (valueText, thumbBounds, juce::Justification::centred, false);
        }
    }
    else
    {
        // For horizontal or other styles, use default behavior
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height,
                                         sliderPos, minSliderPos, maxSliderPos,
                                         style, slider);
    }
}

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    // For now, use default rotary drawing
    // We'll implement filmstrip knobs later when you have the PNG
    LookAndFeel_V4::drawRotarySlider (g, x, y, width, height,
                                      sliderPosProportional,
                                      rotaryStartAngle, rotaryEndAngle,
                                      slider);
}
