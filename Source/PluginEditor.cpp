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
    
    // Setup mix slider
    mixSlider.getSlider().setLookAndFeel (&customLookAndFeel);
    mixSlider.attachToParameter (p.getParameters(), "mix");
    mixSlider.setShowDebugBorder (true);  // Enable debug border
    addAndMakeVisible (mixSlider);
    
    // Setup hue slider (for testing color system)
    hueSlider.getSlider().setLookAndFeel (&customLookAndFeel);
    hueSlider.attachToParameter (p.getParameters(), "hue");
    hueSlider.setInterval (1.0);  // Discrete steps: 0, 1, 2, ..., 9 (10 colors)
    hueSlider.getSlider().onValueChange = [this]() { updateSliderColors(); };
    hueSlider.setShowDebugBorder (true);  // Enable debug border
    addAndMakeVisible (hueSlider);
    
    // Initialize slider colors based on default hue value
    updateSliderColors();
    
    // Start timer to sync view preset state (30 FPS is enough for UI sync)
    startTimerHz (30);
    
    // Set plugin window size (600 viewport + 200 controls + padding)
    setSize (820, 600);
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
    
    // Right side: sliders
    auto controlsArea = bounds;
    controlsArea.reduce (padding, padding);
    
    int sliderWidth = SliderModule::getIdealWidth();
    int sliderHeight = SliderModule::getIdealHeight();
    int centerY = controlsArea.getCentreY();
    int sliderX = controlsArea.getCentreX() - sliderWidth / 2;
    
    // Stack sliders vertically on right side
    mixSlider.setBounds (sliderX - 40, centerY - sliderHeight - 10, sliderWidth, sliderHeight);
    hueSlider.setBounds (sliderX + 40, centerY - sliderHeight - 10, sliderWidth, sliderHeight);
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
    
    // Apply to both sliders
    mixSlider.setAccentColour (colour);
    mixSlider.setValueTextColour (textColour);
    hueSlider.setAccentColour (colour);
    hueSlider.setValueTextColour (textColour);
    
    // Force repaint of sliders
    mixSlider.repaint();
    hueSlider.repaint();
    repaint();
}
