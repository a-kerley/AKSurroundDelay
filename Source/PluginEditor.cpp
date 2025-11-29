#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SurroundDelayAudioProcessorEditor::SurroundDelayAudioProcessorEditor (SurroundDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up the title label
    titleLabel.setText ("Surround Delay", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (24.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    // Set the editor size
    setSize (400, 300);
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
    // Position the title label at the top
    auto bounds = getLocalBounds();
    titleLabel.setBounds (bounds.removeFromTop (60).reduced (10));
}
