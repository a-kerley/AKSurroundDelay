#include "CustomLookAndFeel.h"
#include "SliderModule.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Set default colors
    setColour (juce::Slider::thumbColourId, juce::Colours::white);
    setColour (juce::Slider::trackColourId, fillColour);
    
    // Pre-load SVG assets for default fader style
    ensureSVGsLoadedForStyle (FaderStyle::Fader_38x170);
}

void CustomLookAndFeel::ensureSVGsLoadedForStyle (FaderStyle style)
{
    // Check if already loaded
    if (trackDrawables.find (style) != trackDrawables.end())
        return;
    
    auto assetsPath = juce::File (SliderModule::getAssetsBasePath());
    auto styleInfo = SliderModule::getStyleInfo (style);
    auto stylePath = assetsPath.getChildFile (styleInfo.folderName);
    
    auto trackFile = stylePath.getChildFile (styleInfo.folderName + "_frame.svg");
    
    if (trackFile.existsAsFile())
        trackDrawables[style] = juce::Drawable::createFromSVGFile (trackFile);
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
    // Get accent color, dimensions, and style info from parent SliderModule (if available)
    juce::Colour tintColour = juce::Colours::white;  // Default: no tint
    FaderStyle faderStyle = FaderStyle::Fader_38x170;  // Default style
    float trackWidth = 38.0f * SliderModule::uiScale;   // Fallback defaults (scaled)
    float trackHeight = 170.0f * SliderModule::uiScale;
    float thumbWidth = 34.0f * SliderModule::uiScale;   // Used for value text width
    float thumbInset = 6.5f * SliderModule::uiScale;    // Default inset
    float trackYOffset = 0.0f;                          // Default: no offset
    bool isHorizontal = false;
    
    if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
    {
        tintColour = sliderModule->getAccentColour();
        faderStyle = sliderModule->getFaderStyle();
        const auto& info = sliderModule->getStyleInfo();
        trackWidth = info.trackWidth;
        trackHeight = info.trackHeight;
        thumbWidth = info.thumbWidth;
        thumbInset = info.thumbInset;
        trackYOffset = info.trackYOffset;
        isHorizontal = info.isHorizontal;
    }
    
    // Ensure SVGs are loaded for this style
    const_cast<CustomLookAndFeel*>(this)->ensureSVGsLoadedForStyle (faderStyle);
    
    // Get the track SVG drawable for this style (thumb is rendered by spritesheet)
    juce::Drawable* trackDrawable = nullptr;
    
    auto trackIt = trackDrawables.find (faderStyle);
    if (trackIt != trackDrawables.end())
        trackDrawable = trackIt->second.get();
    
    if (isHorizontal)
    {
        // HORIZONTAL SLIDER
        // For horizontal: display width = trackHeight (travel), display height = trackWidth (short)
        float displayWidth = trackHeight;
        float displayHeight = trackWidth;
        
        // 1. Draw the track SVG - stretch to exactly fit the track bounds
        if (trackDrawable)
        {
            auto trackBounds = juce::Rectangle<float> (x + (width - displayWidth) * 0.5f, 
                                                       y + (height - displayHeight) * 0.5f, 
                                                       displayWidth, displayHeight);
            
            auto drawableCopy = trackDrawable->createCopy();
            drawableCopy->replaceColour (juce::Colours::black, tintColour);
            drawableCopy->replaceColour (juce::Colours::white, tintColour);
            drawableCopy->replaceColour (juce::Colour (0xfff2f2f7), tintColour);
            drawableCopy->drawWithin (g, trackBounds, juce::RectanglePlacement::stretchToFit, 1.0f);
        }
        
        // Note: Thumb is rendered by the spritesheet in SliderModule::paint(), not here
        
        // Draw value text for horizontal slider - positioned at the thumb location
        auto value = slider.getValue();
        if (std::isfinite (value))
        {
            juce::Colour textColour = juce::Colour (0xffcccccc);
            bool usePanDisplay = false;
            if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
            {
                textColour = sliderModule->getValueTextColour();
                usePanDisplay = sliderModule->getUsePanDisplay();
            }
            
            g.setColour (textColour);
            
            // Calculate normalized value (0-1) matching the spritesheet frame calculation
            double minValue = slider.getMinimum();
            double maxValue = slider.getMaximum();
            float normalizedValue = (float)((value - minValue) / (maxValue - minValue));
            normalizedValue = juce::jlimit (0.0f, 1.0f, normalizedValue);
            
            // Track display bounds
            float displayX = x + (width - trackHeight) * 0.5f;
            float displayY = y + (height - trackWidth) * 0.5f;
            
            // Calculate thumb X position based on normalized value (matching spritesheet)
            // For horizontal: left edge = value 0 (normalized), right edge = value 1 (normalized)
            // Use thumbInset for padding and trackYOffset as X offset for horizontal sliders
            float halfThumb = thumbWidth * 0.5f;
            float travelLeft = displayX + thumbInset + halfThumb;                      // Left limit (value = 0) - center of text
            float travelRight = displayX + trackHeight - thumbInset - halfThumb;      // Right limit (value = 1) - center of text
            float travelRange = travelRight - travelLeft;
            float thumbCenterX = travelLeft + (normalizedValue * travelRange);         // Center of text
            float thumbX = thumbCenterX - halfThumb + trackYOffset;                    // Top-left, with X offset
            
            if (usePanDisplay)
            {
                // Pan display mode: "L" or "R" above, value below - tighter spacing
                // Value is -1 to +1, display as L99 to R99
                // Use small epsilon for float comparison
                bool isCenter = std::abs(value) < 0.005f;
                bool isLeft = value < -0.005f;
                juce::String dirLabel = isLeft ? "L" : (isCenter ? "C" : "R");
                int panValue = (int)std::round (std::abs (value) * 99.0f);
                juce::String valueText = isCenter ? "" : juce::String (panValue);
                
                // For center position, use full track width for centering the "C"
                if (isCenter)
                {
                    // Center "C" horizontally in the entire track area
                    auto centerBounds = juce::Rectangle<float> (displayX, displayY, trackHeight, trackWidth);
                    g.setFont (juce::FontOptions (SliderModule::valueFontSize, juce::Font::bold));
                    g.drawText (dirLabel, centerBounds, juce::Justification::centred, false);
                }
                else
                {
                    // Use very small font size and tight spacing for L/R + value
                    float smallFontSize = 7.0f * SliderModule::uiScale;
                    float lineHeight = smallFontSize + 1.0f * SliderModule::uiScale;  // Tight line height
                    float totalHeight = lineHeight * 2.0f;
                    float startY = displayY + (trackWidth - totalHeight) * 0.5f;  // Center vertically
                    
                    // Draw direction label (L/R) on top at thumb position
                    auto dirBounds = juce::Rectangle<float> (thumbX, startY, thumbWidth, lineHeight);
                    g.setFont (juce::FontOptions (smallFontSize, juce::Font::bold));
                    g.drawText (dirLabel, dirBounds, juce::Justification::centred, false);
                    
                    // Draw value directly below with no gap
                    auto valBounds = juce::Rectangle<float> (thumbX, startY + lineHeight, thumbWidth, lineHeight);
                    g.drawText (valueText, valBounds, juce::Justification::centred, false);
                }
            }
            else
            {
                // Standard display - just show value at thumb position
                juce::String suffix;
                int decimalPlaces = 2;
                if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
                {
                    suffix = sliderModule->getValueSuffix();
                    decimalPlaces = sliderModule->getDecimalPlaces();
                }
                
                juce::String valueText;
                if (value <= 1.0f && value >= -1.0f)
                    valueText = juce::String (value, decimalPlaces) + suffix;
                else
                    valueText = juce::String ((int)value) + suffix;
                
                auto textBounds = juce::Rectangle<float> (thumbX, displayY, thumbWidth, trackWidth);
                g.setFont (juce::FontOptions (SliderModule::valueFontSize, juce::Font::bold));
                g.drawText (valueText, textBounds, juce::Justification::centred, false);
            }
        }
    }
    else
    {
        // VERTICAL SLIDER
        // 1. Draw the track SVG - stretch to exactly fit the track bounds
        if (trackDrawable)
        {
            auto trackBounds = juce::Rectangle<float> (x + (width - trackWidth) * 0.5f, 
                                                       y + (height - trackHeight) * 0.5f, 
                                                       trackWidth, trackHeight);
            
            auto drawableCopy = trackDrawable->createCopy();
            drawableCopy->replaceColour (juce::Colours::black, tintColour);
            drawableCopy->replaceColour (juce::Colours::white, tintColour);
            drawableCopy->replaceColour (juce::Colour (0xfff2f2f7), tintColour);
            drawableCopy->drawWithin (g, trackBounds, juce::RectanglePlacement::stretchToFit, 1.0f);
        }
        else
        {
            // Fallback: draw a simple track
            auto fallbackTrackWidth = 2.0f;
            auto trackX = x + width * 0.5f - fallbackTrackWidth * 0.5f;
            g.setColour (juce::Colour (0xffe0e0e0)); /* #e0e0e0 */
            g.fillRect (trackX, (float) y, fallbackTrackWidth, (float) height);
        }
        
        // 2. Draw value text directly on the fader (no SVG thumb)
        // trackYOffset allows fine-tuning the value label position independently of the track
        auto value = slider.getValue();
        if (std::isfinite (value))
        {
            // Get text styling from SliderModule
            juce::Colour textColour = juce::Colour (0xffcccccc); /* #cccccc */
            juce::String suffix;
            int decimalPlaces = 2;
            if (auto* sliderModule = dynamic_cast<SliderModule*>(slider.getParentComponent()))
            {
                textColour = sliderModule->getValueTextColour();
                suffix = sliderModule->getValueSuffix();
                decimalPlaces = sliderModule->getDecimalPlaces();
            }
            
            // Format value text
            juce::String valueText;
            if (value <= 1.0f && value >= -1.0f)
                valueText = juce::String (value, decimalPlaces) + suffix;
            else
                valueText = juce::String ((int)value) + suffix;
            
            // Calculate Y position from normalized value with per-style inset padding
            // Use trackHeight instead of height - JUCE's height includes component padding
            // Calculate track's actual Y position (centered in the component, plus any offset)
            float textHeight = SliderModule::valueFontSize + 4.0f;  // Font size + small padding
            float trackY = y + (height - trackHeight) * 0.5f + trackYOffset;  // Track center + offset
            double normValue = slider.valueToProportionOfLength (value);
            
            // For symmetric travel, we need to account for text height at BOTH ends
            // The text CENTER should be thumbInset + textHeight/2 from each track edge
            float halfText = textHeight * 0.5f;
            float travelTop = trackY + thumbInset + halfText;                    // Top limit (value = 1.0) - center of text
            float travelBottom = trackY + trackHeight - thumbInset - halfText;   // Bottom limit (value = 0.0) - center of text
            float travelRange = travelBottom - travelTop;
            float valueCenterY = travelTop + (1.0f - (float)normValue) * travelRange;  // Center of text
            float valueY = valueCenterY - halfText;  // Convert back to top-left for drawing
            
            // Draw text centered on track, at calculated Y position
            float textWidth = trackWidth + 10.0f;  // Extra width for text overflow
            float textX = x + width * 0.5f - textWidth * 0.5f;
            auto textBounds = juce::Rectangle<float> (textX, valueY, textWidth, textHeight);
            
            // Compensate for font descender space - numbers/letters appear higher than visual center
            // because JUCE centers including descender area (for letters like g, y, p)
            // Shift bounds down by ~15% of font size to visually center the text
            float descenderOffset = SliderModule::valueFontSize * 0.15f;
            textBounds = textBounds.translated (0.0f, descenderOffset);
            
            g.setColour (textColour);
            g.setFont (juce::FontOptions (SliderModule::valueFontSize, juce::Font::bold));
            g.drawText (valueText, textBounds, juce::Justification::centred, false);
        }
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


