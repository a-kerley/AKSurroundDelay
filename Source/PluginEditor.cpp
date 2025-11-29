#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SurroundDelayAudioProcessorEditor::SurroundDelayAudioProcessorEditor (SurroundDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up the title label
    titleLabel.setText ("Surround Delay", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);
    
    // Delay Time Slider
    delayTimeSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    delayTimeSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff4a9eff));
    delayTimeSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff4a9eff));
    addAndMakeVisible (delayTimeSlider);
    
    delayTimeLabel.setText ("Delay Time", juce::dontSendNotification);
    delayTimeLabel.setFont (juce::FontOptions (14.0f));
    delayTimeLabel.setJustificationType (juce::Justification::centred);
    delayTimeLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (delayTimeLabel);
    
    // Feedback Slider
    feedbackSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    feedbackSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffff6b6b));
    feedbackSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffff6b6b));
    addAndMakeVisible (feedbackSlider);
    
    feedbackLabel.setText ("Feedback", juce::dontSendNotification);
    feedbackLabel.setFont (juce::FontOptions (14.0f));
    feedbackLabel.setJustificationType (juce::Justification::centred);
    feedbackLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (feedbackLabel);
    
    // Mix Slider
    mixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    mixSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xff51cf66));
    mixSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff51cf66));
    addAndMakeVisible (mixSlider);
    
    mixLabel.setText ("Mix", juce::dontSendNotification);
    mixLabel.setFont (juce::FontOptions (14.0f));
    mixLabel.setJustificationType (juce::Justification::centred);
    mixLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (mixLabel);
    
    // Attach sliders to parameters
    delayTimeAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        p.getParameters(), "delayTime", delayTimeSlider));
    
    feedbackAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        p.getParameters(), "feedback", feedbackSlider));
    
    mixAttachment.reset (new juce::AudioProcessorValueTreeState::SliderAttachment (
        p.getParameters(), "mix", mixSlider));

    // Set the editor size
    setSize (500, 350);
}

SurroundDelayAudioProcessorEditor::~SurroundDelayAudioProcessorEditor()
{
}

//==============================================================================
void SurroundDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background with a dark gradient
    g.fillAll (juce::Colours::darkgrey);
    
    auto gradient = juce::ColourGradient (
        juce::Colour (0xff2d3142), 0.0f, 0.0f,
        juce::Colour (0xff1a1d2e), 0.0f, (float) getHeight(),
        false
    );
    g.setGradientFill (gradient);
    g.fillAll();
}

void SurroundDelayAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title at the top
    titleLabel.setBounds (bounds.removeFromTop (50).reduced (10));
    
    bounds.removeFromTop (20); // Spacing
    
    // Three sliders in a row
    auto sliderArea = bounds.removeFromTop (180);
    const int sliderWidth = sliderArea.getWidth() / 3;
    
    // Delay Time
    auto delayArea = sliderArea.removeFromLeft (sliderWidth).reduced (10);
    delayTimeLabel.setBounds (delayArea.removeFromTop (20));
    delayTimeSlider.setBounds (delayArea);
    
    // Feedback
    auto feedbackArea = sliderArea.removeFromLeft (sliderWidth).reduced (10);
    feedbackLabel.setBounds (feedbackArea.removeFromTop (20));
    feedbackSlider.setBounds (feedbackArea);
    
    // Mix
    auto mixArea = sliderArea.reduced (10);
    mixLabel.setBounds (mixArea.removeFromTop (20));
    mixSlider.setBounds (mixArea);
}
