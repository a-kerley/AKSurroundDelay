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
            // Fallback: draw a simple track
            auto fallbackTrackWidth = 2.0f;
            auto trackX = x + width * 0.5f - fallbackTrackWidth * 0.5f;
            g.setColour (juce::Colour (0xffe0e0e0)); /* #e0e0e0 */
            g.fillRect (trackX, (float) y, fallbackTrackWidth, (float) height);
        }
        
        // 2. Draw the thumb with padding
        // Get padding value from parent SliderModule and remap thumb position
        float padding = 0.0f;
        if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
            padding = SliderModule::thumbPadding;
        
        // Remap sliderPos to account for padding
        float proportion = (sliderPos - y) / height;
        float paddedHeight = height - (padding * 2);
        float paddedPos = y + padding + (proportion * paddedHeight);
        
        auto thumbX = x + width * 0.5f - SliderModule::thumbWidth * 0.5f;
        auto thumbY = paddedPos - SliderModule::thumbHeight * 0.5f;
        auto thumbBounds = juce::Rectangle<float> (thumbX, thumbY, SliderModule::thumbWidth, SliderModule::thumbHeight);
        
        if (sliderThumbDrawable)
        {
            // Clone and recolor the thumb SVG - make it transparent
            auto thumbCopy = sliderThumbDrawable->createCopy();
            thumbCopy->replaceColour (juce::Colours::black, juce::Colours::transparentBlack);
            thumbCopy->replaceColour (juce::Colours::white, juce::Colours::transparentBlack);
            thumbCopy->replaceColour (juce::Colour (0xfff2f2f7), juce::Colours::transparentBlack);
            
            thumbCopy->drawWithin (g, thumbBounds, juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            // Fallback: draw a simple transparent thumb
            g.setColour (juce::Colours::transparentBlack);
            g.fillRoundedRectangle (thumbBounds, 2.0f);
        }
        
        // Draw value text in the center of the thumb
        auto value = slider.getValue();
        if (std::isfinite (value))
        {
            // Get accent color from parent SliderModule to calculate contrasting text color
            juce::Colour accentColour = juce::Colours::white;  // Default
            juce::Colour textColour = juce::Colour (0xffcccccc);  // Default light grey
            if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
            {
                accentColour = sliderModule->getAccentColour();
                textColour = sliderModule->getValueTextColour();
            }
            
            // Use the fixed text color from SliderModule
            g.setColour (textColour);
            g.setFont (juce::FontOptions (SliderModule::valueFontSize, juce::Font::bold));
            
            // Get the value suffix and decimal places from parent SliderModule (if available)
            juce::String suffix;
            int decimalPlaces = 2;  // Default
            if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
            {
                suffix = sliderModule->getValueSuffix();
                decimalPlaces = sliderModule->getDecimalPlaces();
            }
            
            // Format value based on range (0-1 shows decimals, larger shows integers)
            juce::String valueText;
            if (value <= 1.0f && value >= 0.0f)
                valueText = juce::String (value, decimalPlaces) + suffix;
            else
                valueText = juce::String ((int)value) + suffix;  // Integer for larger values
            
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


