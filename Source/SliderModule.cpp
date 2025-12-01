#include "SliderModule.h"
#include "ColorPalette.h"

// Static member initialization
juce::Image SliderModule::fillBarImage;
bool SliderModule::fillBarImageLoaded = false;
std::map<int, juce::Image> SliderModule::colorVariants;
bool SliderModule::colorVariantsLoaded = false;

SliderModule::SliderModule (const juce::String& labelText)
{
    // Setup slider
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);  // No text box - value shown in thumb
    slider.setColour (juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::trackColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff2a2a2a)); /* #2a2a2a */
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setRange (0.0, 1.0, 0.001); // Finer step for smoother animation (was 0.01)
    slider.setValue (0.5, juce::dontSendNotification); // Set default value
    slider.setDoubleClickReturnValue (true, 0.5);  // Cmd+click resets to default (0.5)
    addAndMakeVisible (slider);
    
    // Setup name label (below slider)
    parameterName = labelText;
    if (labelText.isNotEmpty())
    {
        nameLabel.setText (labelText, juce::dontSendNotification);
        nameLabel.setFont (juce::FontOptions (labelFontSize));
        nameLabel.setJustificationType (juce::Justification::centred);
        nameLabel.setColour (juce::Label::textColourId, labelTextColour);
        addAndMakeVisible (nameLabel);
    }
    
    // Listen to slider changes to repaint fill bar
    slider.onValueChange = [this]() 
    { 
        repaint();  // Repaint to update fill bar position
    };
    
    // Load fill bar spritesheet once (shared by all SliderModule instances)
    if (!fillBarImageLoaded)
    {
        auto assetsPath = juce::File ("/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets");
        auto fillBarFile = assetsPath.getChildFile ("SliderFill_spritesheet_withthumb.png");
        
        if (fillBarFile.existsAsFile())
        {
            fillBarImage = juce::ImageCache::getFromFile (fillBarFile);
            if (!fillBarImage.isNull())
            {
                fillBarImageLoaded = true;
                
                // Load color variants
                loadColorVariants();
            }
        }
    }
}

SliderModule::~SliderModule()
{
}

void SliderModule::attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& parameterID)
{
    // Store the parameter ID for reference
    currentParameterID = parameterID;
    
    // Create or recreate the attachment
    // This automatically detaches from any previous parameter
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);
}

void SliderModule::detachFromParameter()
{
    // Reset attachment to detach from parameter
    attachment.reset();
    currentParameterID.clear();
    slider.setValue (0.5, juce::dontSendNotification);
}

void SliderModule::setLabelText (const juce::String& text)
{
    parameterName = text;
    nameLabel.setText (text, juce::dontSendNotification);
}

void SliderModule::loadColorVariants()
{
    if (colorVariantsLoaded)
        return;
    
    auto assetsPath = juce::File ("/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets");
    
    // Check if cache files exist - if not, generate them first
    bool cacheExists = true;
    for (int i = 0; i < 10; ++i)
    {
        auto variantFile = assetsPath.getChildFile ("SliderFill_color" + juce::String(i) + ".png");
        if (!variantFile.existsAsFile())
        {
            cacheExists = false;
            break;
        }
    }
    
    if (!cacheExists)
        generateColorVariantCache();
    
    // Load each pre-cached color variant PNG file
    for (int i = 0; i < 10; ++i)
    {
        auto variantFile = assetsPath.getChildFile ("SliderFill_color" + juce::String(i) + ".png");
        
        if (variantFile.existsAsFile())
        {
            auto variantImage = juce::ImageCache::getFromFile (variantFile);
            if (!variantImage.isNull())
                colorVariants[i] = variantImage;
        }
    }
    
    colorVariantsLoaded = true;
}

void SliderModule::generateColorVariantCache()
{
    // Get palette colors from central definition
    auto paletteColors = ColorPalette::getBackgroundColors();
    
    // Ensure fill bar is loaded
    if (fillBarImage.isNull())
    {
        auto assetsPath = juce::File ("/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets");
        auto fillBarFile = assetsPath.getChildFile ("SliderFill_spritesheet_withthumb.png");
        if (fillBarFile.existsAsFile())
            fillBarImage = juce::ImageCache::getFromFile (fillBarFile);
    }
    
    if (!fillBarImage.isValid())
    {
        DBG ("ERROR: Cannot generate color variants - source image not found");
        return;
    }
    
    auto assetsPath = juce::File ("/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets");
    
    // Generate tinted variants for each palette color and save as PNG
    for (int i = 0; i < paletteColors.size(); ++i)
    {
        auto colour = paletteColors[i];
        
        // Clone and tint the spritesheet
        auto tintedImage = fillBarImage.createCopy();
        
        // Fast color multiplication using BitmapData
        juce::Image::BitmapData bitmapData (tintedImage, juce::Image::BitmapData::readWrite);
        
        float tintR = colour.getFloatRed();
        float tintG = colour.getFloatGreen();
        float tintB = colour.getFloatBlue();
        
        for (int y = 0; y < bitmapData.height; ++y)
        {
            for (int x = 0; x < bitmapData.width; ++x)
            {
                auto pixel = bitmapData.getPixelColour (x, y);
                auto tinted = juce::Colour::fromFloatRGBA (
                    pixel.getFloatRed() * tintR,
                    pixel.getFloatGreen() * tintG,
                    pixel.getFloatBlue() * tintB,
                    pixel.getFloatAlpha()
                );
                bitmapData.setPixelColour (x, y, tinted);
            }
        }
        
        // Save as PNG file
        auto outputFile = assetsPath.getChildFile ("SliderFill_color" + juce::String(i) + ".png");
        juce::FileOutputStream outputStream (outputFile);
        
        if (outputStream.openedOk())
        {
            juce::PNGImageFormat pngFormat;
            pngFormat.writeImageToStream (tintedImage, outputStream);
        }
    }
}

const juce::Image& SliderModule::getVariantForColor (const juce::Colour& colour)
{
    // Get palette colors from central definition
    auto paletteColors = ColorPalette::getBackgroundColors();
    
    // Find the closest matching palette color
    int closestIndex = 0;
    float minDistance = std::numeric_limits<float>::max();
    
    for (int i = 0; i < paletteColors.size(); ++i)
    {
        // Simple color distance using RGB euclidean distance
        float dr = colour.getFloatRed() - paletteColors[i].getFloatRed();
        float dg = colour.getFloatGreen() - paletteColors[i].getFloatGreen();
        float db = colour.getFloatBlue() - paletteColors[i].getFloatBlue();
        float distance = dr*dr + dg*dg + db*db;
        
        if (distance < minDistance)
        {
            minDistance = distance;
            closestIndex = i;
        }
    }
    
    // Look up the variant
    auto it = colorVariants.find (closestIndex);
    if (it != colorVariants.end())
        return it->second;
    
    // Fallback to original grayscale if variant not found
    return fillBarImage;
}

void SliderModule::paint (juce::Graphics& g)
{
    // Draw fill bar behind the slider using spritesheet frames
    if (!fillBarImage.isNull())
    {
        // Get slider bounds
        auto sliderBounds = slider.getBounds();
        
        // Calculate which frame to display based on slider value
        double sliderValue = slider.getValue();
        double minValue = slider.getMinimum();
        double maxValue = slider.getMaximum();
        
        // Normalize value to 0-1 range (works for any slider range)
        float normalizedValue = (float)((sliderValue - minValue) / (maxValue - minValue));
        
        // Safety check - clamp to valid range
        if (!std::isfinite(normalizedValue))
            return;  // Skip drawing if value is invalid
        
        normalizedValue = juce::jlimit (0.0f, 1.0f, normalizedValue);
        
        // Calculate frame index (0 to 169)
        // Spritesheet is reversed: Frame 0 = full (value 1.0), Frame 169 = empty (value 0.0)
        // So we need to invert the index
        // Use roundToInt() instead of (int) cast for smoother frame selection
        int frameIndex = juce::jlimit (0, spritesheetTotalFrames - 1, 
                                       juce::roundToInt ((1.0f - normalizedValue) * (spritesheetTotalFrames - 1)));
        
        // Calculate source Y position in the spritesheet
        // Frames are stacked vertically from top (frame 0) to bottom (frame 169)
        int srcY = frameIndex * spritesheetFrameHeight;
        
        // Destination bounds (scale down from 4x to 1x)
        float fillX = sliderBounds.getX();
        float fillY = sliderBounds.getY();
        
        // Enable high-quality resampling for smooth 4x downscaling
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        
        // Get the pre-tinted variant for this slider's accent color (instant lookup!)
        const auto& spritesheetToUse = getVariantForColor (accentColour);
        
        // Draw the pre-tinted spritesheet frame
        g.drawImage (spritesheetToUse,
                    (int)fillX, (int)fillY, (int)trackWidth, (int)trackHeight,
                    0, srcY, spritesheetFrameWidth, spritesheetFrameHeight,
                    false);
    }
    
    // Draw debug border if enabled
    if (showDebugBorder)
    {
        g.setColour (juce::Colours::red);
        g.drawRect (getLocalBounds(), 1);
    }
}

void SliderModule::resized()
{
    auto bounds = getLocalBounds();
    
    // Remove component padding from all sides
    bounds.removeFromTop ((int)componentPaddingTop);
    bounds.removeFromBottom ((int)componentPaddingBottom);
    bounds.removeFromLeft ((int)componentPaddingLeft);
    bounds.removeFromRight ((int)componentPaddingRight);
    
    // Layout: [Slider] [Spacing] [Name Label]
    // Name label below slider with configurable spacing
    nameLabel.setBounds (bounds.removeFromBottom ((int)labelHeight));
    bounds.removeFromBottom ((int)labelSpacing);  // Add spacing gap
    
    // Slider at full track height - padding is applied in CustomLookAndFeel
    auto sliderBounds = bounds.withSizeKeepingCentre ((int)trackWidth, (int)trackHeight);
    slider.setBounds (sliderBounds);
}
