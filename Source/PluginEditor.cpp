#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapMatrixAudioProcessorEditor::TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      timelineView (p), stereoFieldView (p)
{
    // Create tap selection buttons
    for (int i = 0; i < 8; ++i)
    {
        tapButtons[i] = std::make_unique<juce::TextButton> (juce::String (i + 1));
        tapButtons[i]->setClickingTogglesState (true);
        tapButtons[i]->setRadioGroupId (1001);
        tapButtons[i]->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a2a));
        tapButtons[i]->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff51cf66));
        tapButtons[i]->setColour (juce::TextButton::textColourOffId, juce::Colour (0xff8a8a8a));
        tapButtons[i]->setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        tapButtons[i]->onClick = [this, i] { selectTap (i); };
        addAndMakeVisible (tapButtons[i].get());
    }
    
    // Select tap 1 by default
    tapButtons[0]->setToggleState (true, juce::dontSendNotification);
    
    // Create per-tap faders
    gainFader = std::make_unique<VerticalFader> ("GAIN", juce::Colour (0xff51cf66));
    delayFader = std::make_unique<VerticalFader> ("DELAY", juce::Colour (0xff51cf66));
    feedbackFader = std::make_unique<VerticalFader> ("FBACK", juce::Colour (0xff51cf66));
    crosstalkFader = std::make_unique<VerticalFader> ("CTALK", juce::Colour (0xff51cf66));
    dampingFader = std::make_unique<VerticalFader> ("DAMP", juce::Colour (0xff51cf66));
    reverbFader = std::make_unique<VerticalFader> ("REVERB", juce::Colour (0xff51cf66));
    panXFader = std::make_unique<VerticalFader> ("PAN-X", juce::Colour (0xff51cf66));
    panYFader = std::make_unique<VerticalFader> ("PAN-Y", juce::Colour (0xff51cf66));
    panZFader = std::make_unique<VerticalFader> ("PAN-Z", juce::Colour (0xff51cf66));
    
    addAndMakeVisible (gainFader.get());
    addAndMakeVisible (delayFader.get());
    addAndMakeVisible (feedbackFader.get());
    addAndMakeVisible (crosstalkFader.get());
    addAndMakeVisible (dampingFader.get());
    addAndMakeVisible (reverbFader.get());
    addAndMakeVisible (panXFader.get());
    addAndMakeVisible (panYFader.get());
    addAndMakeVisible (panZFader.get());
    
    // Create global faders
    hpfFader = std::make_unique<VerticalFader> ("HPF", juce::Colour (0xff4a9eff));
    lpfFader = std::make_unique<VerticalFader> ("LPF", juce::Colour (0xff4a9eff));
    duckingFader = std::make_unique<VerticalFader> ("DUCK", juce::Colour (0xffcc5de8));
    mixFader = std::make_unique<VerticalFader> ("MIX", juce::Colour (0xffffba08));
    outputFader = std::make_unique<VerticalFader> ("OUT", juce::Colour (0xffff6b6b));
    
    addAndMakeVisible (hpfFader.get());
    addAndMakeVisible (lpfFader.get());
    addAndMakeVisible (duckingFader.get());
    addAndMakeVisible (mixFader.get());
    addAndMakeVisible (outputFader.get());
    
    // Global attachments
    hpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "hpfFreq", hpfFader->getSlider());
    lpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "lpfFreq", lpfFader->getSlider());
    duckingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "ducking", duckingFader->getSlider());
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "mix", mixFader->getSlider());
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getParameters(), "outputGain", outputFader->getSlider());
    
    // Reverb type combo
    reverbTypeCombo.addItem ("DRK", 1);
    reverbTypeCombo.addItem ("SHT", 2);
    reverbTypeCombo.addItem ("MED", 3);
    reverbTypeCombo.addItem ("LNG", 4);
    reverbTypeCombo.addItem ("XXXL", 5);
    reverbTypeCombo.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff2a2a2a));
    reverbTypeCombo.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    reverbTypeCombo.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff4a4a4a));
    addAndMakeVisible (reverbTypeCombo);
    
    reverbTypeLabel.setText ("VERB", juce::dontSendNotification);
    reverbTypeLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    reverbTypeLabel.setJustificationType (juce::Justification::centred);
    reverbTypeLabel.setColour (juce::Label::textColourId, juce::Colour (0xff8a8a8a));
    addAndMakeVisible (reverbTypeLabel);
    
    reverbTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.getParameters(), "reverbType", reverbTypeCombo);
    
    // Toggle buttons
    tapeModeToggle.setButtonText ("TAPE");
    tapeModeToggle.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    tapeModeToggle.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xff51cf66));
    addAndMakeVisible (tapeModeToggle);
    
    tapeModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        p.getParameters(), "tapeMode", tapeModeToggle);
    
    // Add visual components
    addAndMakeVisible (timelineView);
    addAndMakeVisible (stereoFieldView);
    
    // Initialize attachments for tap 1
    updateTapAttachments();
    
    // Set window size
    setSize (1100, 700);
    
    // Start timer for visual updates
    startTimerHz (30);
}

TapMatrixAudioProcessorEditor::~TapMatrixAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TapMatrixAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background
    g.fillAll (juce::Colour (0xff0a0a0a));
    
    // Draw "TAPMATRIX" or "SLAPPER" branding
    g.setColour (juce::Colour (0xff2a2a2a));
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("T A P M A T R I X", getWidth() - 150, getHeight() - 25, 140, 20, 
                juce::Justification::centredRight);
}

void TapMatrixAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Top section: Timeline view (350px high)
    timelineView.setBounds (bounds.removeFromTop (350).reduced (10));
    
    bounds.removeFromTop (10);
    
    // Middle section: Stereo field view (left) + Controls (right)
    auto middleSection = bounds.removeFromTop (200);
    stereoFieldView.setBounds (middleSection.removeFromLeft (250).reduced (10));
    
    // Right side of middle: Global toggles
    auto toggleArea = middleSection.removeFromRight (120).reduced (10);
    reverbTypeLabel.setBounds (toggleArea.removeFromTop (20));
    reverbTypeCombo.setBounds (toggleArea.removeFromTop (30));
    toggleArea.removeFromTop (10);
    tapeModeToggle.setBounds (toggleArea.removeFromTop (25));
    
    bounds.removeFromTop (10);
    
    // Bottom section: Tap buttons + Faders
    auto bottomSection = bounds.reduced (10);
    
    // Tap selection buttons (8 buttons in a row)
    auto buttonRow = bottomSection.removeFromTop (35);
    const int buttonWidth = 40;
    const int buttonSpacing = 5;
    int startX = 10;
    
    for (int i = 0; i < 8; ++i)
    {
        tapButtons[i]->setBounds (startX + i * (buttonWidth + buttonSpacing), 0, 
                                  buttonWidth, 30);
    }
    
    bottomSection.removeFromTop (10);
    
    // Fader section
    auto faderArea = bottomSection;
    const int faderWidth = 70;
    const int spacing = 10;
    
    // Per-tap faders (9 faders)
    gainFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    delayFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    feedbackFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    crosstalkFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    dampingFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    reverbFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    panXFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    panYFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    panZFader->setBounds (faderArea.removeFromLeft (faderWidth));
    
    // Separator
    faderArea.removeFromLeft (spacing * 2);
    
    // Global faders (5 faders)
    hpfFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    lpfFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    duckingFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    mixFader->setBounds (faderArea.removeFromLeft (faderWidth));
    faderArea.removeFromLeft (spacing);
    outputFader->setBounds (faderArea.removeFromLeft (faderWidth));
}

void TapMatrixAudioProcessorEditor::timerCallback()
{
    // Update visual displays
    timelineView.repaint();
    stereoFieldView.repaint();
}

void TapMatrixAudioProcessorEditor::selectTap (int tapIndex)
{
    if (tapIndex >= 0 && tapIndex < 8)
    {
        selectedTap = tapIndex;
        updateTapAttachments();
    }
}

void TapMatrixAudioProcessorEditor::updateTapAttachments()
{
    // Detach old attachments
    gainAttachment.reset();
    delayAttachment.reset();
    feedbackAttachment.reset();
    crosstalkAttachment.reset();
    dampingAttachment.reset();
    reverbAttachment.reset();
    panXAttachment.reset();
    panYAttachment.reset();
    panZAttachment.reset();
    
    // Create parameter IDs for selected tap
    juce::String tapSuffix = juce::String (selectedTap + 1);
    
    // Attach to selected tap's parameters
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "gain" + tapSuffix, gainFader->getSlider());
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "delayTime" + tapSuffix, delayFader->getSlider());
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "feedback" + tapSuffix, feedbackFader->getSlider());
    crosstalkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "crosstalk" + tapSuffix, crosstalkFader->getSlider());
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "damping" + tapSuffix, dampingFader->getSlider());
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "reverb" + tapSuffix, reverbFader->getSlider());
    panXAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "panX" + tapSuffix, panXFader->getSlider());
    panYAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "panY" + tapSuffix, panYFader->getSlider());
    panZAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getParameters(), "panZ" + tapSuffix, panZFader->getSlider());
}
