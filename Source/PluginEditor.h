#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

//==============================================================================
/**
 * Visual component for displaying and editing a single tap
 */
class TapComponent : public juce::Component
{
public:
    TapComponent (int tapIndex, TapMatrixAudioProcessor& proc)
        : tapIndex (tapIndex), processor (proc)
    {
        // Create parameter ID for this tap
        auto getID = [tapIndex](const char* name) {
            return juce::String (name) + juce::String (tapIndex + 1);
        };
        
        // Gain slider
        gainSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        gainSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (gainSlider);
        gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.getParameters(), getID ("gain"), gainSlider);
        
        // Delay slider
        delaySlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        delaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        addAndMakeVisible (delaySlider);
        delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.getParameters(), getID ("delayTime"), delaySlider);
        
        // Feedback slider
        feedbackSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        feedbackSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (feedbackSlider);
        feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.getParameters(), getID ("feedback"), feedbackSlider);
        
        // Pan X slider
        panXSlider.setSliderStyle (juce::Slider::LinearHorizontal);
        panXSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (panXSlider);
        panXAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.getParameters(), getID ("panX"), panXSlider);
        
        // Pan Y slider
        panYSlider.setSliderStyle (juce::Slider::LinearVertical);
        panYSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible (panYSlider);
        panYAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            proc.getParameters(), getID ("panY"), panYSlider);
    }
    
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Draw tap background
        g.setColour (juce::Colour (0xff3d4556).withAlpha (0.8f));
        g.fillRoundedRectangle (bounds, 8.0f);
        
        // Draw tap number
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        g.drawText ("Tap " + juce::String (tapIndex + 1), 
                   bounds.reduced (5.0f).removeFromTop (20.0f),
                   juce::Justification::centred);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced (5);
        bounds.removeFromTop (25); // Space for label
        
        // Top row: gain and delay
        auto topRow = bounds.removeFromTop (80);
        gainSlider.setBounds (topRow.removeFromLeft (70));
        delaySlider.setBounds (topRow.removeFromLeft (70));
        
        bounds.removeFromTop (5);
        
        // Middle row: feedback and pan controls
        auto midRow = bounds.removeFromTop (60);
        feedbackSlider.setBounds (midRow.removeFromLeft (70));
        
        auto panArea = midRow.reduced (5);
        panXSlider.setBounds (panArea.removeFromBottom (20));
        panYSlider.setBounds (panArea.removeFromLeft (20));
    }
    
private:
    int tapIndex;
    TapMatrixAudioProcessor& processor;
    
    juce::Slider gainSlider, delaySlider, feedbackSlider, panXSlider, panYSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panXAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panYAttachment;
};

//==============================================================================
/**
 * TapMatrix Audio Processor Editor
 * 
 * Full GUI with tap field, pan visualization, and global controls
 */
class TapMatrixAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor&);
    ~TapMatrixAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TapMatrixAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    
    // Tap components (8 taps)
    std::array<std::unique_ptr<TapComponent>, 8> tapComponents;
    
    // Global controls
    juce::Slider mixSlider, outputGainSlider, hpfSlider, lpfSlider, duckingSlider;
    juce::Label mixLabel, outputGainLabel, hpfLabel, lpfLabel, duckingLabel;
    juce::ComboBox reverbTypeCombo;
    juce::Label reverbTypeLabel;
    juce::ToggleButton tapeModeToggle;
    
    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> duckingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> reverbTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> tapeModeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessorEditor)
};
