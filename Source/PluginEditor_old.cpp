#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapMatrixAudioProcessorEditor::TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up the title label
    titleLabel.setText ("TapMatrix - 8 Tap Spatial Delay", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);
    
    // Create all 8 tap components
    for (int i = 0; i < 8; ++i)
    {
        tapComponents[i] = std::make_unique<TapComponent> (i, p);
        addAndMakeVisible (tapComponents[i].get());
    }
    
    // Global Controls Setup
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, 
                                     const juce::String& labelText, juce::Colour colour)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        slider.setColour (juce::Slider::thumbColourId, colour);
        slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
        addAndMakeVisible (slider);
        
        label.setText (labelText, juce::dontSendNotification);
        label.setFont (juce::FontOptions (12.0f));
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible (label);
    };
    
    setupRotarySlider (mixSlider, mixLabel, "Mix", juce::Colour (0xff51cf66));
    setupRotarySlider (outputGainSlider, outputGainLabel, "Output", juce::Colour (0xffff6b6b));
    setupRotarySlider (hpfSlider, hpfLabel, "HPF", juce::Colour (0xff4a9eff));
    setupRotarySlider (lpfSlider, lpfLabel, "LPF", juce::Colour (0xffffba08));
    setupRotarySlider (duckingSlider, duckingLabel, "Ducking", juce::Colour (0xffcc5de8));
    
    // Reverb Type combo box
    reverbTypeCombo.addItem ("Dark", 1);
    reverbTypeCombo.addItem ("Short", 2);
    reverbTypeCombo.addItem ("Medium", 3);
    reverbTypeCombo.addItem ("Long", 4);
    reverbTypeCombo.addItem ("XXXL", 5);
    addAndMakeVisible (reverbTypeCombo);
    
    reverbTypeLabel.setText ("Reverb", juce::dontSendNotification);
    reverbTypeLabel.setFont (juce::FontOptions (12.0f));
    reverbTypeLabel.setJustificationType (juce::Justification::centred);
    reverbTypeLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (reverbTypeLabel);
    
    // Tape mode toggle
    tapeModeToggle.setButtonText ("Tape Mode");
    tapeModeToggle.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (tapeModeToggle);
    
    // Attach parameters
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "mix", mixSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "outputGain", outputGainSlider);
    hpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "hpfFreq", hpfSlider);
    lpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "lpfFreq", lpfSlider);
    duckingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "ducking", duckingSlider);
    reverbTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.getParameters(), "reverbType", reverbTypeCombo);
    tapeModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        p.getParameters(), "tapeMode", tapeModeToggle);

    // Set the editor size (larger for 8 taps)
    setSize (1000, 700);
    
    // Start timer for visual updates (30 fps)
    startTimerHz (30);
}

TapMatrixAudioProcessorEditor::~TapMatrixAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TapMatrixAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background gradient
    auto gradient = juce::ColourGradient (
        juce::Colour (0xff1a1d2e), 0.0f, 0.0f,
        juce::Colour (0xff0f111a), 0.0f, (float) getHeight(),
        false
    );
    g.setGradientFill (gradient);
    g.fillAll();
    
    // Draw section dividers
    g.setColour (juce::Colour (0xff4a5568).withAlpha (0.3f));
    auto bounds = getLocalBounds();
    bounds.removeFromTop (50); // Title area
    
    // Line under title
    g.drawLine (10.0f, 50.0f, (float) getWidth() - 10.0f, 50.0f, 1.0f);
    
    // Line above global controls
    float globalY = (float) getHeight() - 120.0f;
    g.drawLine (10.0f, globalY, (float) getWidth() - 10.0f, globalY, 1.0f);
}

void TapMatrixAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Title at the top
    titleLabel.setBounds (bounds.removeFromTop (50).reduced (10));
    
    bounds.removeFromTop (10); // Spacing
    
    // Global controls at bottom
    auto globalArea = bounds.removeFromBottom (120).reduced (10);
    const int numGlobalControls = 7; // mix, output, hpf, lpf, ducking, reverb, tape mode
    const int controlWidth = globalArea.getWidth() / numGlobalControls;
    
    auto setupControl = [&globalArea](juce::Component& control, juce::Component& label, int width)
    {
        auto area = globalArea.removeFromLeft (width).reduced (5);
        label.setBounds (area.removeFromTop (15));
        control.setBounds (area.reduced (5, 0));
    };
    
    setupControl (mixSlider, mixLabel, controlWidth);
    setupControl (outputGainSlider, outputGainLabel, controlWidth);
    setupControl (hpfSlider, hpfLabel, controlWidth);
    setupControl (lpfSlider, lpfLabel, controlWidth);
    setupControl (duckingSlider, duckingLabel, controlWidth);
    setupControl (reverbTypeCombo, reverbTypeLabel, controlWidth);
    
    auto tapeModeArea = globalArea.reduced (5);
    tapeModeArea.removeFromTop (15); // Align with other controls
    tapeModeToggle.setBounds (tapeModeArea.removeFromTop (30).reduced (10, 5));
    
    // Tap components in 2 rows of 4
    bounds.removeFromTop (10); // Spacing
    auto tapArea = bounds.reduced (10);
    const int tapHeight = tapArea.getHeight() / 2;
    const int tapWidth = tapArea.getWidth() / 4;
    
    for (int row = 0; row < 2; ++row)
    {
        auto rowArea = tapArea.removeFromTop (tapHeight);
        for (int col = 0; col < 4; ++col)
        {
            int tapIndex = row * 4 + col;
            auto tapBounds = rowArea.removeFromLeft (tapWidth).reduced (5);
            tapComponents[tapIndex]->setBounds (tapBounds);
        }
    }
}

void TapMatrixAudioProcessorEditor::timerCallback()
{
    // Trigger repaints for visual feedback (meters, animation, etc.)
    // Currently no real-time visuals, but this is where they'd be updated
    repaint();
}
