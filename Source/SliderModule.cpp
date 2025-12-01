#include "SliderModule.h"

// Static member initialization
juce::Image SliderModule::fillBarImage;
bool SliderModule::fillBarImageLoaded = false;

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
    
    // Setup value label (below slider) - temporarily disabled to avoid string assertions
    valueLabel.setText ("", juce::dontSendNotification);
    valueLabel.setFont (juce::FontOptions (11.0f));
    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    valueLabel.setVisible (false); // Hidden for now
    addAndMakeVisible (valueLabel);
    
    // Listen to slider changes to repaint fill bar
    slider.onValueChange = [this]() 
    { 
        repaint();  // Repaint to update fill bar position
    };
    
    // Load fill bar spritesheet once (shared by all SliderModule instances)
    if (!fillBarImageLoaded)
    {
        auto assetsPath = juce::File ("/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets");
        auto fillBarFile = assetsPath.getChildFile ("SliderFill_spritesheet.png");
        
        if (fillBarFile.existsAsFile())
        {
            fillBarImage = juce::ImageCache::getFromFile (fillBarFile);
            if (!fillBarImage.isNull())
            {
                DBG ("Loaded SliderFill_spritesheet.png - Size: " + 
                     juce::String(fillBarImage.getWidth()) + "x" + 
                     juce::String(fillBarImage.getHeight()) + 
                     " (" + juce::String(spritesheetTotalFrames) + " frames)");
                fillBarImageLoaded = true;
            }
        }
        else
        {
            DBG ("SliderFill_spritesheet.png NOT FOUND at: " + fillBarFile.getFullPathName());
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

void SliderModule::updateValueLabel()
{
    auto value = slider.getValue();
    if (std::isfinite (value))
        valueLabel.setText (juce::String (value, 1), juce::dontSendNotification);
    else
        valueLabel.setText ("--", juce::dontSendNotification);
}

void SliderModule::paint (juce::Graphics& g)
{
    // Draw fill bar behind the slider using spritesheet frames
    if (!fillBarImage.isNull())
    {
        // Get slider bounds
        auto sliderBounds = slider.getBounds();
        
        // Calculate which frame to display based on slider value (0 = bottom, 1 = top)
        float sliderValue = (float)slider.getValue();
        
        // Safety check - clamp to valid range
        if (!std::isfinite(sliderValue) || sliderValue < 0.0f || sliderValue > 1.0f)
            return;  // Skip drawing if value is invalid
        
        // Calculate frame index (0 to 169)
        // Spritesheet is reversed: Frame 0 = full (value 1.0), Frame 169 = empty (value 0.0)
        // So we need to invert the index
        // Use roundToInt() instead of (int) cast for smoother frame selection
        int frameIndex = juce::jlimit (0, spritesheetTotalFrames - 1, 
                                       juce::roundToInt ((1.0f - sliderValue) * (spritesheetTotalFrames - 1)));
        
        // Calculate source Y position in the spritesheet
        // Frames are stacked vertically from top (frame 0) to bottom (frame 169)
        int srcY = frameIndex * spritesheetFrameHeight;
        
        // Destination bounds (scale down from 4x to 1x)
        float fillX = sliderBounds.getX();
        float fillY = sliderBounds.getY();
        
        // Enable high-quality resampling for smooth 4x downscaling
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        
        // Draw the current frame scaled down to match track dimensions
        g.drawImage (fillBarImage,
                    (int)fillX, (int)fillY, (int)trackWidth, (int)trackHeight,
                    0, srcY, spritesheetFrameWidth, spritesheetFrameHeight);
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
