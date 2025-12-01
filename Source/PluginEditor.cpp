#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ColorPalette.h"

//==============================================================================
TapMatrixAudioProcessorEditor::TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set custom LookAndFeel
    setLookAndFeel (&customLookAndFeel);
    
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
    
    // Set plugin window size (1100 × 700)
    setSize (1100, 700);
}

TapMatrixAudioProcessorEditor::~TapMatrixAudioProcessorEditor()
{
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
    
    // Center the sliders
    int centerX = bounds.getCentreX();
    int centerY = bounds.getCentreY();
    
    // Use SliderModule's ideal size calculations
    int sliderWidth = SliderModule::getIdealWidth();
    int sliderHeight = SliderModule::getIdealHeight();
    
    // Position sliders side by side at their ideal sizes
    // Mix slider on left, Hue slider on right
    mixSlider.setBounds (centerX - 100, centerY - sliderHeight/2, sliderWidth, sliderHeight);
    hueSlider.setBounds (centerX + 20, centerY - sliderHeight/2, sliderWidth, sliderHeight);
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
