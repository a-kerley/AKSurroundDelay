#include "SliderModule.h"

SliderModule::SliderModule (const juce::String& labelText)
{
    // Setup slider
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::trackColourId, juce::Colours::transparentBlack);
    slider.setRange (0.0, 1.0, 0.01); // Set explicit range
    slider.setValue (0.5, juce::dontSendNotification); // Set default value
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
    
    // Listen to slider changes to update value label
    // Temporarily disabled
    // slider.onValueChange = [this]() { updateValueLabel(); };
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
    // Optional: draw background or border
}

void SliderModule::resized()
{
    auto bounds = getLocalBounds();
    
    // Layout: [Slider] [Name Label]
    // Name label below slider
    nameLabel.setBounds (bounds.removeFromBottom (16));
    
    // Keep slider at full track height so track doesn't get clipped
    // The thumb travel will be constrained in CustomLookAndFeel
    auto sliderBounds = bounds.withSizeKeepingCentre ((int)trackWidth, (int)trackHeight);
    slider.setBounds (sliderBounds);
}
