#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ColorPalette.h"

//==============================================================================
TapMatrixAudioProcessorEditor::TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set custom LookAndFeel
    setLookAndFeel (&customLookAndFeel);
    
    // Setup 3D Surround Stage View
    addAndMakeVisible (surroundStageView);
    
    // Setup view preset selector
    setupViewPresetSelector();
    
    // Setup test sliders - one of each style with independent parameters
    // Debug borders enabled for testing
    
    // 38x170 - Standard vertical fader (default) - Mix parameter
    slider_38x170.getSlider().setLookAndFeel (&customLookAndFeel);
    slider_38x170.attachToParameter (p.getParameters(), "mix");
    slider_38x170.setLabelText ("MIX");
    slider_38x170.setShowDebugBorder (true);
    addAndMakeVisible (slider_38x170);
    
    // 22x170 - Slim vertical fader - Output Gain
    slider_22x170.getSlider().setLookAndFeel (&customLookAndFeel);
    slider_22x170.attachToParameter (p.getParameters(), "outputGain");
    slider_22x170.setLabelText ("OUT");
    slider_22x170.setValueSuffix ("dB");
    slider_22x170.setShowDebugBorder (true);
    addAndMakeVisible (slider_22x170);
    
    // 32x129 - Medium vertical fader - Tap 1 Gain
    slider_32x129.getSlider().setLookAndFeel (&customLookAndFeel);
    slider_32x129.attachToParameter (p.getParameters(), "gain1");
    slider_32x129.setLabelText ("TAP 1");
    slider_32x129.setValueSuffix ("dB");
    slider_32x129.setShowDebugBorder (true);
    addAndMakeVisible (slider_32x129);
    
    // 32x129 Front-Back - Medium fader - Tap 1 Pan Y (Front/Back)
    slider_32x129FB.getSlider().setLookAndFeel (&customLookAndFeel);
    slider_32x129FB.attachToParameter (p.getParameters(), "panY1");
    slider_32x129FB.setLabelText ("F/B");
    slider_32x129FB.setShowDebugBorder (true);
    addAndMakeVisible (slider_32x129FB);
    
    // 22x79 - Small vertical fader - Tap 1 Feedback
    slider_22x79.getSlider().setLookAndFeel (&customLookAndFeel);
    slider_22x79.attachToParameter (p.getParameters(), "feedback1");
    slider_22x79.setLabelText ("FDBK");
    slider_22x79.setShowDebugBorder (true);
    addAndMakeVisible (slider_22x79);
    
    // 28x84 Horizontal - L/R Pan fader - Tap 1 Pan X
    slider_28x84H.getSlider().setLookAndFeel (&customLookAndFeel);
    slider_28x84H.attachToParameter (p.getParameters(), "panX1");
    slider_28x84H.setLabelText ("L/R");
    slider_28x84H.setUsePanDisplay (true);  // Enable L/R pan display mode
    slider_28x84H.setShowDebugBorder (true);
    addAndMakeVisible (slider_28x84H);
    
    // Setup hue slider (for testing color system)
    hueSlider.getSlider().setLookAndFeel (&customLookAndFeel);
    hueSlider.attachToParameter (p.getParameters(), "hue");
    hueSlider.setLabelText ("HUE");
    hueSlider.setInterval (1.0);  // Discrete steps: 0, 1, 2, ..., 9 (10 colors)
    hueSlider.getSlider().onValueChange = [this]() { updateSliderColors(); };
    hueSlider.setShowDebugBorder (true);
    addAndMakeVisible (hueSlider);
    
    // Initialize slider colors based on default hue value
    updateSliderColors();
    
    // Start timer to sync view preset state (30 FPS is enough for UI sync)
    startTimerHz (30);
    
    // Set plugin window size (wider to accommodate all test sliders)
    setSize (1000, 700);
}

TapMatrixAudioProcessorEditor::~TapMatrixAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
void TapMatrixAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Clean dark background
    g.fillAll (juce::Colour (0xff1a1a1a)); /* #1a1a1a */
    
    // Draw center crosshair for layout reference
    g.setColour (juce::Colours::grey.withAlpha (0.3f));
    auto bounds = getLocalBounds();
    g.drawLine (bounds.getCentreX(), 0, bounds.getCentreX(), bounds.getHeight(), 1.0f);
    g.drawLine (0, bounds.getCentreY(), bounds.getWidth(), bounds.getCentreY(), 1.0f);
}

void TapMatrixAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int padding = 20;
    const int viewportSize = 500;
    const int selectorHeight = 28;
    const int selectorWidth = 320;
    
    // Left side: 3D viewport (500x500) with padding
    auto viewportArea = bounds.removeFromLeft (viewportSize + padding * 2);
    viewportArea.reduce (padding, padding);
    
    // Position viewport
    surroundStageView.setBounds (viewportArea.removeFromTop (viewportSize));
    
    // View preset selector below viewport (centered)
    viewportArea.removeFromTop (10); // spacing
    auto selectorArea = viewportArea.removeFromTop (selectorHeight);
    selectorArea = selectorArea.withSizeKeepingCentre (selectorWidth, selectorHeight);
    viewPresetSelector.setBounds (selectorArea);
    
    // Right side: test sliders
    auto controlsArea = bounds;
    controlsArea.reduce (padding, padding);
    
    // Layout all sliders in a row at the top
    int sliderSpacing = 10;
    int xPos = controlsArea.getX();
    int yPos = controlsArea.getY();
    
    // Row 1: Vertical sliders of varying heights
    // 38x170 (standard)
    slider_38x170.setBounds (xPos, yPos, slider_38x170.getPreferredWidth(), slider_38x170.getPreferredHeight());
    xPos += slider_38x170.getPreferredWidth() + sliderSpacing;
    
    // 22x170 (slim)
    slider_22x170.setBounds (xPos, yPos, slider_22x170.getPreferredWidth(), slider_22x170.getPreferredHeight());
    xPos += slider_22x170.getPreferredWidth() + sliderSpacing;
    
    // 32x129 (medium)
    slider_32x129.setBounds (xPos, yPos, slider_32x129.getPreferredWidth(), slider_32x129.getPreferredHeight());
    xPos += slider_32x129.getPreferredWidth() + sliderSpacing;
    
    // 32x129 Front-Back
    slider_32x129FB.setBounds (xPos, yPos, slider_32x129FB.getPreferredWidth(), slider_32x129FB.getPreferredHeight());
    xPos += slider_32x129FB.getPreferredWidth() + sliderSpacing;
    
    // 22x79 (small)
    slider_22x79.setBounds (xPos, yPos, slider_22x79.getPreferredWidth(), slider_22x79.getPreferredHeight());
    xPos += slider_22x79.getPreferredWidth() + sliderSpacing;
    
    // Hue slider (38x170) - last in row
    hueSlider.setBounds (xPos, yPos, hueSlider.getPreferredWidth(), hueSlider.getPreferredHeight());
    
    // Row 2: Horizontal slider below
    int row2Y = yPos + slider_38x170.getPreferredHeight() + 20;
    slider_28x84H.setBounds (controlsArea.getX(), row2Y, slider_28x84H.getPreferredWidth(), slider_28x84H.getPreferredHeight());
}

void TapMatrixAudioProcessorEditor::setupViewPresetSelector()
{
    viewPresetSelector.onPresetSelected = [this](SurroundStageView::ViewPreset preset)
    {
        surroundStageView.setViewPreset (preset);
    };
    
    addAndMakeVisible (viewPresetSelector);
}

void TapMatrixAudioProcessorEditor::timerCallback()
{
    // Sync the selector with the SurroundStageView's current preset
    auto currentPreset = surroundStageView.getCurrentPreset();
    viewPresetSelector.setCurrentPreset (currentPreset);
}

void TapMatrixAudioProcessorEditor::updateSliderColors()
{
    // Safety check - ensure slider is initialized
    auto value = hueSlider.getSlider().getValue();
    if (!std::isfinite (value))
        return;
    
    // Get the selected color index (0-9)
    int colorIndex = juce::jlimit (0, ColorPalette::paletteSize - 1, (int)value);
    const auto& colorPair = ColorPalette::palettePairs[colorIndex];
    auto colour = colorPair.background;
    auto textColour = colorPair.text;
    
    // Apply to all test sliders
    slider_38x170.setAccentColour (colour);
    slider_38x170.setValueTextColour (textColour);
    
    slider_22x170.setAccentColour (colour);
    slider_22x170.setValueTextColour (textColour);
    
    slider_32x129.setAccentColour (colour);
    slider_32x129.setValueTextColour (textColour);
    
    slider_32x129FB.setAccentColour (colour);
    slider_32x129FB.setValueTextColour (textColour);
    
    slider_22x79.setAccentColour (colour);
    slider_22x79.setValueTextColour (textColour);
    
    slider_28x84H.setAccentColour (colour);
    slider_28x84H.setValueTextColour (textColour);
    
    hueSlider.setAccentColour (colour);
    hueSlider.setValueTextColour (textColour);
    
    // Force repaint of all sliders
    slider_38x170.repaint();
    slider_22x170.repaint();
    slider_32x129.repaint();
    slider_32x129FB.repaint();
    slider_22x79.repaint();
    slider_28x84H.repaint();
    hueSlider.repaint();
    repaint();
}
